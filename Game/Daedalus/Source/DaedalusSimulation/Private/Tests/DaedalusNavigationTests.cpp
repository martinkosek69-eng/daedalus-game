#include "DaedalusNavigationModel.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include <limits>

using namespace Daedalus;
namespace
{
FNavigationBody Body(const TCHAR* Id, const FVector3d& Position = FVector3d::ZeroVector)
{
    FNavigationBody Result;
    Result.Id = Id; Result.Name = Id; Result.PositionMetres = Position;
    return Result;
}

TArray<FNavigationSystem> Catalog()
{
    FNavigationSystem Sol;
    Sol.Id = TEXT("sol"); Sol.Name = TEXT("Solar fixture");
    Sol.GalaxyPositionLightYears = FVector3d(100000, -100000, 100000);
    Sol.Bodies.Add(Body(TEXT("sol.star")));
    auto Planet = Body(TEXT("sol.planet"), FVector3d(1000000, 2000000, 3000000));
    Planet.ParentId = TEXT("sol.star"); Planet.bKnownRadius = true; Planet.RadiusMetres = 6371000;
    Sol.Bodies.Add(Planet);
    FNavigationSystem Alpha;
    Alpha.Id = TEXT("alpha"); Alpha.Name = TEXT("One light-year fixture");
    Alpha.GalaxyPositionLightYears = Sol.GalaxyPositionLightYears + FVector3d(1, 0, 0);
    Alpha.Bodies.Add(Body(TEXT("alpha.star")));
    return {Sol, Alpha};
}

bool Finite(const FNavigationMetrics& Metrics)
{
    return FMath::IsFinite(Metrics.DistanceMetres) && FMath::IsFinite(Metrics.CurrentETASeconds)
        && FMath::IsFinite(Metrics.PlannedETASeconds) && FMath::IsFinite(Metrics.RequiredSpeedMetresPerSecond)
        && FMath::IsFinite(Metrics.Direction.X) && FMath::IsFinite(Metrics.Direction.Y) && FMath::IsFinite(Metrics.Direction.Z);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNavigationCatalogTest, "Daedalus.Navigation.CatalogRollback", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FNavigationCatalogTest::RunTest(const FString&)
{
    FNavigationPlan Plan; FString Error;
    const auto Good = Catalog();
    TestTrue(TEXT("valid navigation catalog"), Plan.Configure(Good, Error));
    TestTrue(TEXT("choose nondefault target"), Plan.SelectTarget(TEXT("alpha"), TEXT("alpha.star"), Error));
    TestTrue(TEXT("choose nonzero location"), Plan.SetLocation(TEXT("sol"), FVector3d(100, 200, 300), Error));
    const auto Prior = Plan.Query(100, 200, 10);
    auto Reject = [&](const TArray<FNavigationSystem>& Bad, const TCHAR* Label)
    {
        TestFalse(Label, Plan.Configure(Bad, Error));
        TestFalse(TEXT("failed catalog gives a reason"), Error.IsEmpty());
        const auto After = Plan.Query(100, 200, 10);
        TestTrue(TEXT("invalid catalog retains all query results and selection"),
            After.bValid && After.DistanceMetres == Prior.DistanceMetres && After.Direction == Prior.Direction
            && After.CurrentETASeconds == Prior.CurrentETASeconds
            && Plan.GetTargetSystemId() == TEXT("alpha") && Plan.GetTargetBodyId() == TEXT("alpha.star")
            && Plan.GetLocationSystemId() == TEXT("sol"));
    };
    Reject({}, TEXT("empty catalog rejected"));
    auto Bad = Good; Bad[1].Id = Bad[0].Id; Reject(Bad, TEXT("duplicate system ID rejected"));
    Bad = Good;
    const auto DuplicateBody = Bad[0].Bodies[0];
    Bad[0].Bodies.Add(DuplicateBody); Reject(Bad, TEXT("duplicate body ID in same system rejected"));
    Bad = Good; Bad[0].Bodies[1].ParentId = TEXT("missing"); Reject(Bad, TEXT("unknown parent rejected"));
    Bad = Good; Bad[0].Bodies[0].ParentId = Bad[0].Bodies[1].Id; Reject(Bad, TEXT("two-body parent cycle rejected"));
    Bad = Good; Bad[0].Bodies[0].ParentId = Bad[0].Bodies[0].Id; Reject(Bad, TEXT("self-parent rejected"));
    Bad = Good; Bad[0].Bodies[0].Id = TEXT("bad id"); Reject(Bad, TEXT("unstable whitespace ID rejected"));
    Bad = Good; Bad[0].Name = TEXT("   "); Reject(Bad, TEXT("blank display name rejected"));
    Bad = Good; Bad[0].Bodies[0].PositionMetres.X = std::numeric_limits<double>::quiet_NaN(); Reject(Bad, TEXT("NaN local position rejected"));
    Bad = Good; Bad[0].GalaxyPositionLightYears.Z = std::numeric_limits<double>::infinity(); Reject(Bad, TEXT("infinite galaxy coordinate rejected"));
    Bad = Good; Bad[0].Bodies[0].PositionMetres.Z = FNavigationPlan::MaxLocalCoordinateMetres + 1; Reject(Bad, TEXT("local coordinate bound enforced"));
    Bad = Good; Bad[0].GalaxyPositionLightYears.X = FNavigationPlan::MaxGalaxyCoordinateLightYears + 1; Reject(Bad, TEXT("galaxy coordinate bound enforced"));
    Bad = Good; Bad[0].Bodies[0].RadiusMetres = 100; Reject(Bad, TEXT("unknown radius cannot pretend to be measured"));
    Bad = Good; Bad[0].Bodies[1].RadiusMetres = 0; Reject(Bad, TEXT("known radius must be positive"));
    Bad = Good; Bad[0].Bodies[0].RadiusMetres = std::numeric_limits<double>::infinity(); Reject(Bad, TEXT("nonfinite unknown radius rejected"));
    Bad = Good; Bad[0].Bodies.Reset(); Reject(Bad, TEXT("system needs at least one target body"));
    Bad = Good; Bad[0].Bodies.SetNum(FNavigationPlan::MaxBodiesPerSystem + 1); Reject(Bad, TEXT("per-system body limit enforced"));
    Bad = Good; Bad.SetNum(FNavigationPlan::MaxSystems + 1); Reject(Bad, TEXT("system count limit enforced"));

    auto ReusedBodyId = Good;
    ReusedBodyId[1].Bodies[0].Id = ReusedBodyId[0].Bodies[0].Id;
    TestTrue(TEXT("body IDs are unique per system, not globally"), Plan.Configure(ReusedBodyId, Error));
    // A long valid hierarchy must not recurse or treat a repeated ancestor as a cycle.
    FNavigationSystem Chain; Chain.Id = TEXT("chain"); Chain.Name = TEXT("Bounded hierarchy");
    for (int32 Index = 0; Index < FNavigationPlan::MaxBodiesPerSystem; ++Index)
    {
        auto Next = Body(TEXT("placeholder"));
        Next.Id = FString::Printf(TEXT("body.%d"), Index);
        if (Index > 0) Next.ParentId = Chain.Bodies[Index - 1].Id;
        Chain.Bodies.Add(MoveTemp(Next));
    }
    TestTrue(TEXT("full 10000-body hierarchy validates without recursion"), Plan.Configure({Chain}, Error));
    TArray<FNavigationSystem> Total;
    for (int32 Index = 0; Index < 10; ++Index)
    {
        auto System = Chain; System.Id = FString::Printf(TEXT("system.%d"), Index); Total.Add(MoveTemp(System));
    }
    TestTrue(TEXT("exact practical 100000 total-body limit accepted"), Plan.Configure(Total, Error));
    auto Extra = Good[0]; Extra.Id = TEXT("extra"); Extra.Bodies.SetNum(1); Total.Add(Extra);
    TestFalse(TEXT("practical total-body guard rejects 100001"), Plan.Configure(Total, Error));
    TestEqual(TEXT("total-body failure retains valid default selection"), Plan.GetLocationSystemId(), FString(TEXT("system.0")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNavigationDistanceTest, "Daedalus.Navigation.LocalGalacticDistances", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FNavigationDistanceTest::RunTest(const FString&)
{
    FNavigationPlan Plan; FString Error; auto Data = Catalog();
    TestTrue(TEXT("configure distant sector"), Plan.Configure(Data, Error));
    TestTrue(TEXT("select local planet"), Plan.SelectTarget(TEXT("sol"), TEXT("sol.planet"), Error));
    const FVector3d LocalPoint = Data[0].Bodies[1].PositionMetres - FVector3d(.01, .02, .02);
    Plan.SetLocation(TEXT("sol"), LocalPoint, Error);
    auto Metrics = Plan.Query(1, 1, 1);
    TestTrue(TEXT("centimetre local separation survives 100000LY absolute galaxy position"), FMath::Abs(Metrics.DistanceMetres - .03) < 1e-9);
    TestTrue(TEXT("local vector direction is normalized and correct"), Metrics.Direction.Equals(FVector3d(1.0/3, 2.0/3, 2.0/3), 1e-8));
    TestTrue(TEXT("distance is centre-to-centre even inside a known-radius sphere"), Metrics.DistanceMetres > 0 && Metrics.DistanceMetres < Data[0].Bodies[1].RadiusMetres);
    Plan.SetLocation(TEXT("sol"), FVector3d::ZeroVector, Error); Plan.SelectTarget(TEXT("alpha"), TEXT("alpha.star"), Error);
    Metrics = Plan.Query(1, 1, 1);
    TestEqual(TEXT("one light-year converts to exact canonical metres"), Metrics.DistanceMetres, 9460730472580800.0);
    TestTrue(TEXT("galactic direction towards positive X"), Metrics.Direction.Equals(FVector3d(1,0,0), 1e-14));
    Data[1].GalaxyPositionLightYears = Data[0].GalaxyPositionLightYears + FVector3d(3, 4, 0);
    Plan.Configure(Data, Error); Plan.SelectTarget(TEXT("alpha"), TEXT("alpha.star"), Error);
    Metrics = Plan.Query(1, 1, 1);
    TestTrue(TEXT("3-4-5 galactic vector has five-light-year distance"), FMath::Abs(Metrics.DistanceMetres / FNavigationPlan::LightYearMetres - 5) < 1e-14);
    TestTrue(TEXT("3D galactic basis direction retained"), Metrics.Direction.Equals(FVector3d(.6,.8,0), 1e-14));
    Data[1].GalaxyPositionLightYears = Data[0].GalaxyPositionLightYears + FVector3d(1.0/1024, 0, 0);
    Data[1].Bodies[0].PositionMetres = FVector3d(125, 0, 0);
    Plan.Configure(Data, Error); Plan.SetLocation(TEXT("sol"), FVector3d(25, 0, 0), Error); Plan.SelectTarget(TEXT("alpha"), TEXT("alpha.star"), Error);
    Metrics = Plan.Query(1, 1, 1);
    TestTrue(TEXT("small inter-system separation retains local offsets"), FMath::Abs(Metrics.DistanceMetres - (FNavigationPlan::LightYearMetres / 1024 + 100)) < .01);
    Plan.SetLocation(TEXT("alpha"), Data[1].Bodies[0].PositionMetres, Error);
    Metrics = Plan.Query(1, 1, 1);
    TestTrue(TEXT("coincident target returns zero distance and zero direction"), Metrics.bValid && Metrics.DistanceMetres == 0 && Metrics.Direction.IsZero());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNavigationETATest, "Daedalus.Navigation.ETABoundaries", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FNavigationETATest::RunTest(const FString&)
{
    FNavigationPlan Plan; FString Error;
    TestFalse(TEXT("query before configure is unavailable"), Plan.Query(1,1,1).bValid);
    auto Data = Catalog(); Data[0].Bodies[0].PositionMetres = FVector3d(300, 400, 0);
    Plan.Configure(Data, Error);
    const auto Metrics = Plan.Query(100, 250, 10);
    TestTrue(TEXT("independent current/planned/required estimates"), Metrics.bValid && Metrics.bCurrentETA && Metrics.bPlannedETA && Metrics.bRequiredSpeed);
    TestEqual(TEXT("current ETA uses current absolute speed"), Metrics.CurrentETASeconds, 5.0);
    TestEqual(TEXT("planned ETA uses planned speed"), Metrics.PlannedETASeconds, 2.0);
    TestEqual(TEXT("desired arrival duration yields effective required speed"), Metrics.RequiredSpeedMetresPerSecond, 50.0);
    for (double Invalid : {0.0, -1.0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity()})
    {
        const auto Bad = Plan.Query(Invalid, Invalid, Invalid);
        TestTrue(TEXT("invalid estimate inputs keep finite geometric result and unavailable flags"),
            Bad.bValid && !Bad.bCurrentETA && !Bad.bPlannedETA && !Bad.bRequiredSpeed && Finite(Bad)
            && Bad.CurrentETASeconds == 0 && Bad.PlannedETASeconds == 0 && Bad.RequiredSpeedMetresPerSecond == 0);
    }
    auto Partial = Plan.Query(-1, 250, -1);
    TestTrue(TEXT("invalid current speed does not erase valid planned ETA"), !Partial.bCurrentETA && Partial.bPlannedETA && Partial.PlannedETASeconds == 2);
    Partial = Plan.Query(100, -1, 10);
    TestTrue(TEXT("invalid planned speed does not erase other estimates"), Partial.bCurrentETA && !Partial.bPlannedETA && Partial.bRequiredSpeed);
    const auto Overflow = Plan.Query(1e-310, 1e-310, 1e-310);
    TestTrue(TEXT("overflowing ETA/required-speed division unavailable without Inf"), Overflow.bValid && !Overflow.bCurrentETA && !Overflow.bPlannedETA && !Overflow.bRequiredSpeed && Finite(Overflow));
    Plan.SetLocation(TEXT("sol"), Data[0].Bodies[0].PositionMetres, Error);
    const auto Coincident = Plan.Query(1, 1, 1);
    TestTrue(TEXT("zero-distance positive-speed ETA and required speed are zero"), Coincident.bCurrentETA && Coincident.bPlannedETA && Coincident.bRequiredSpeed && Coincident.CurrentETASeconds == 0 && Coincident.PlannedETASeconds == 0 && Coincident.RequiredSpeedMetresPerSecond == 0);
    const auto Stopped = Plan.Query(0,0,0);
    TestTrue(TEXT("zero distance still requires positive estimate denominators"), Stopped.bValid && !Stopped.bCurrentETA && !Stopped.bPlannedETA && !Stopped.bRequiredSpeed);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNavigationSelectionTest, "Daedalus.Navigation.SelectionLocationCommands", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FNavigationSelectionTest::RunTest(const FString&)
{
    FNavigationPlan Plan; FString Error; auto Data = Catalog();
    TestFalse(TEXT("selection before configure rejected"), Plan.SelectTarget(TEXT("sol"), TEXT("sol.star"), Error));
    TestFalse(TEXT("location before configure rejected"), Plan.SetLocation(TEXT("sol"), FVector3d::ZeroVector, Error));
    Plan.Configure(Data, Error);
    TestTrue(TEXT("successful configure chooses first system/body at zero"), Plan.GetTargetSystemId() == TEXT("sol") && Plan.GetTargetBodyId() == TEXT("sol.star") && Plan.GetLocationSystemId() == TEXT("sol") && Plan.Query(1,1,1).DistanceMetres == 0);
    // Later caller edits cannot alter the plan's copied immutable catalog.
    Data[0].Bodies[0].PositionMetres = FVector3d(1000,0,0);
    TestEqual(TEXT("external catalog edit cannot mutate configured plan"), Plan.Query(1,1,1).DistanceMetres, 0.0);
    Plan.SetLocation(TEXT("sol"), FVector3d(3,4,0), Error);
    Plan.SelectTarget(TEXT("alpha"), TEXT("alpha.star"), Error);
    TestEqual(TEXT("target selection never changes location system"), Plan.GetLocationSystemId(), FString(TEXT("sol")));
    Plan.SelectTarget(TEXT("sol"), TEXT("sol.star"), Error);
    TestEqual(TEXT("target selection never changes local position"), Plan.Query(1,1,1).DistanceMetres, 5.0);
    TestFalse(TEXT("unknown system target rejected"), Plan.SelectTarget(TEXT("missing"), TEXT("sol.star"), Error));
    TestFalse(TEXT("body ID from another system rejected"), Plan.SelectTarget(TEXT("sol"), TEXT("alpha.star"), Error));
    TestEqual(TEXT("invalid target preserves selected body"), Plan.GetTargetBodyId(), FString(TEXT("sol.star")));
    TestFalse(TEXT("unknown active system rejected"), Plan.SetLocation(TEXT("missing"), FVector3d::ZeroVector, Error));
    TestFalse(TEXT("nonfinite location rejected"), Plan.SetLocation(TEXT("sol"), FVector3d(std::numeric_limits<double>::quiet_NaN(),0,0), Error));
    TestFalse(TEXT("out of range location rejected"), Plan.SetLocation(TEXT("sol"), FVector3d(1e15 + 1,0,0), Error));
    TestEqual(TEXT("invalid location retains previous local position"), Plan.Query(1,1,1).DistanceMetres, 5.0);
    TestTrue(TEXT("explicit active-system change accepted"), Plan.SetLocation(TEXT("alpha"), FVector3d::ZeroVector, Error));
    TestEqual(TEXT("explicit location change preserves target"), Plan.GetTargetSystemId(), FString(TEXT("sol")));
    TestTrue(TEXT("location change updates cross-system metrics"), Plan.Query(1,1,1).DistanceMetres == FNavigationPlan::LightYearMetres);
    TestTrue(TEXT("successful catalog refresh accepted"), Plan.Configure(Data, Error));
    TestTrue(TEXT("catalog refresh explicitly resets location and target"), Plan.GetLocationSystemId() == TEXT("sol") && Plan.GetTargetSystemId() == TEXT("sol") && Plan.GetTargetBodyId() == TEXT("sol.star"));
    TestEqual(TEXT("catalog refresh resets local position to zero"), Plan.Query(1,1,1).DistanceMetres, 1000.0);
    const auto BeforeQuery = Plan.Query(1,2,3); Plan.Query(1000,4000,50); const auto AfterQuery = Plan.Query(1,2,3);
    TestTrue(TEXT("queries do not mutate selection or distance"), BeforeQuery.DistanceMetres == AfterQuery.DistanceMetres && BeforeQuery.CurrentETASeconds == AfterQuery.CurrentETASeconds);
    return true;
}
#endif

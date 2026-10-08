#include "DaedalusShipFlightCatalog.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Dom/JsonObject.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

using namespace Daedalus;
namespace
{
bool ReadCatalog(FShipFlightCatalog& Catalog, FString& Json, FString& Error)
{
    return FFileHelper::LoadFileToString(Json, *(FPaths::ProjectContentDir() / TEXT("Data/Solar/ships.json")))
        && Catalog.LoadJson(Json, Error);
}

bool SameConfig(const FFlightConfig& A, const FFlightConfig& B)
{
    return A.MaxSpeed == B.MaxSpeed && A.Acceleration == B.Acceleration && A.Braking == B.Braking
        && A.CoastDeceleration == B.CoastDeceleration && A.LateralAcceleration == B.LateralAcceleration
        && A.TurnRateDegrees == B.TurnRateDegrees && A.AngularAccelerationDegrees == B.AngularAccelerationDegrees
        && A.PitchLimitDegrees == B.PitchLimitDegrees && A.BankDegrees == B.BankDegrees
        && A.LowSpeedTurnMultiplier == B.LowSpeedTurnMultiplier && A.HighSpeedBankFraction == B.HighSpeedBankFraction
        && A.ShipRadiusMetres == B.ShipRadiusMetres;
}

bool SameCatalog(const FShipFlightCatalog& A, const FShipFlightCatalog& B)
{
    if (A.Ships.Num() != B.Ships.Num()) return false;
    for (int32 Ship = 0; Ship < A.Ships.Num(); ++Ship)
    {
        const auto& Left = A.Ships[Ship]; const auto& Right = B.Ships[Ship];
        if (Left.Id != Right.Id || Left.Name != Right.Name || Left.LengthMetres != Right.LengthMetres
            || Left.ProfileNames != Right.ProfileNames || Left.Profiles.Num() != Right.Profiles.Num()) return false;
        for (int32 Profile = 0; Profile < Left.Profiles.Num(); ++Profile)
            if (!SameConfig(Left.Profiles[Profile], Right.Profiles[Profile])) return false;
    }
    return true;
}

bool SameState(const FFlightState& A, const FFlightState& B)
{
    return A.PositionMetres == B.PositionMetres && A.VelocityMetresPerSecond == B.VelocityMetresPerSecond
        && A.YawDegrees == B.YawDegrees && A.PitchDegrees == B.PitchDegrees && A.BankDegrees == B.BankDegrees
        && A.YawRateDegrees == B.YawRateDegrees && A.PitchRateDegrees == B.PitchRateDegrees
        && A.Throttle == B.Throttle && A.SimulationSeconds == B.SimulationSeconds && A.ContactBodyId == B.ContactBodyId;
}

bool NoDrift(const FFlightState& State)
{
    const double Speed = State.VelocityMetresPerSecond.Size();
    return Speed < 1e-9 || FVector3d::CrossProduct(State.Forward(), State.VelocityMetresPerSecond / Speed).Size() < 1e-10;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShipFlightCatalogTest, "Daedalus.Flight.ShipCatalogValidationRollback", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FShipFlightCatalogTest::RunTest(const FString&)
{
    FShipFlightCatalog Catalog; FString Json, Error;
    if (!TestTrue(TEXT("canonical ship catalog loaded"), ReadCatalog(Catalog, Json, Error))) return false;
    if (!TestEqual(TEXT("two ship definitions"), Catalog.Ships.Num(), 2)) return false;
    TestEqual(TEXT("Daedalus remains default"), Catalog.Ships[0].Id, FString(TEXT("daedalus")));
    TestEqual(TEXT("Aurora second choice"), Catalog.Ships[1].Id, FString(TEXT("aurora")));
    const auto Before = Catalog;
    auto Reject = [&](const FString& Bad, const TCHAR* Label)
    {
        TestFalse(Label, Catalog.LoadJson(Bad, Error));
        TestFalse(TEXT("invalid catalog reports reason"), Error.IsEmpty());
        TestTrue(TEXT("failure retains every prior ship/profile"), SameCatalog(Catalog, Before));
    };
    auto Mutate = [&](auto Change, const TCHAR* Label)
    {
        TSharedPtr<FJsonObject> Root;
        if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root)) return;
        Change(Root);
        FString Bad; FJsonSerializer::Serialize(Root.ToSharedRef(), TJsonWriterFactory<>::Create(&Bad));
        Reject(Bad, Label);
    };
    auto Aurora = [](const TSharedPtr<FJsonObject>& Root) { return Root->GetArrayField(TEXT("ships"))[1]->AsObject(); };
    Reject(TEXT("{"), TEXT("malformed JSON rejected"));
    Reject(TEXT("{\"version\":1,\"ships\":[]}"), TEXT("empty catalog rejected"));
    Mutate([](auto Root) { Root->SetNumberField(TEXT("version"), 2); }, TEXT("unsupported version rejected"));
    Mutate([](auto Root) { Root->SetStringField(TEXT("version"), TEXT("1")); }, TEXT("string version rejected"));
    Mutate([&](auto Root) { Aurora(Root)->SetStringField(TEXT("id"), TEXT("daedalus")); }, TEXT("duplicate ID rejected after first parsed ship"));
    Mutate([&](auto Root) { Aurora(Root)->SetStringField(TEXT("id"), TEXT("bad id")); }, TEXT("unstable whitespace ID rejected"));
    Mutate([&](auto Root) { Aurora(Root)->SetStringField(TEXT("name"), TEXT("  ")); }, TEXT("blank ship name rejected"));
    Mutate([&](auto Root) { Aurora(Root)->SetNumberField(TEXT("lengthMetres"), 0); }, TEXT("zero length rejected"));
    Mutate([&](auto Root) { Aurora(Root)->SetNumberField(TEXT("radiusMetres"), 1749); }, TEXT("radius cannot understate half hull length"));
    Mutate([&](auto Root) { Aurora(Root)->SetStringField(TEXT("radiusMetres"), TEXT("1950")); }, TEXT("numeric strings rejected"));
    Mutate([&](auto Root) { Aurora(Root)->SetNumberField(TEXT("turnRateDegrees"), -1); }, TEXT("invalid shared turning limits rejected"));
    Mutate([&](auto Root) { auto Ship = Aurora(Root); auto Profiles = Ship->GetArrayField(TEXT("profiles")); Profiles.Pop(); Ship->SetArrayField(TEXT("profiles"), Profiles); }, TEXT("exactly three profiles required"));
    Mutate([&](auto Root) { Aurora(Root)->GetArrayField(TEXT("profiles"))[1]->AsObject()->SetStringField(TEXT("name"), TEXT("")); }, TEXT("blank profile name rejected"));
    Mutate([&](auto Root) { Aurora(Root)->GetArrayField(TEXT("profiles"))[2]->AsObject()->SetNumberField(TEXT("speed"), 299792458.0); }, TEXT("light speed rejected transactionally"));
    Mutate([&](auto Root) { Aurora(Root)->GetArrayField(TEXT("profiles"))[2]->AsObject()->SetNumberField(TEXT("speed"), 300000000.0); }, TEXT("superluminal profile rejected"));
    Mutate([&](auto Root) { Aurora(Root)->GetArrayField(TEXT("profiles"))[2]->AsObject()->SetNumberField(TEXT("acceleration"), 0); }, TEXT("zero drive acceleration rejected"));
    Reject(Json.Replace(TEXT("299492665.542"), TEXT("1e309")), TEXT("nonfinite numeric overflow rejected"));
    Reject(Json.Replace(TEXT("\"lengthMetres\": 3500"), TEXT("\"lengthMetres\": 1e309")), TEXT("nonfinite hull length rejected"));

    TSharedPtr<FJsonObject> Root;
    FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root);
    Aurora(Root)->SetNumberField(TEXT("radiusMetres"), 1750);
    Aurora(Root)->SetStringField(TEXT("presentationAsset"), TEXT("ignored by simulation"));
    FString Boundary; FJsonSerializer::Serialize(Root.ToSharedRef(), TJsonWriterFactory<>::Create(&Boundary));
    TestTrue(TEXT("exact half-length radius and unknown presentation fields accepted"), Catalog.LoadJson(Boundary, Error));
    TestTrue(TEXT("successful load clears prior error"), Error.IsEmpty());
    TestEqual(TEXT("validated radius reaches all profiles"), Catalog.Ships[1].Profiles[2].ShipRadiusMetres, 1750.0);

    FString LegacyJson; TSharedPtr<FJsonObject> Legacy;
    if (!TestTrue(TEXT("legacy Daedalus tuning loaded"), FFileHelper::LoadFileToString(LegacyJson, *(FPaths::ProjectContentDir() / TEXT("Data/Solar/flight.json")))
        && FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(LegacyJson), Legacy))) return false;
    const auto& Daedalus = Catalog.Ships[0];
    TestEqual(TEXT("legacy hull length preserved"), Daedalus.LengthMetres, Legacy->GetNumberField(TEXT("shipLengthMetres")));
    for (int32 Index = 0; Index < 3; ++Index)
    {
        const auto& Config = Daedalus.Profiles[Index]; const auto Profile = Legacy->GetArrayField(TEXT("profiles"))[Index]->AsObject();
        TestEqual(TEXT("legacy profile name preserved"), Daedalus.ProfileNames[Index], Profile->GetStringField(TEXT("name")));
        TestEqual(TEXT("legacy speed preserved"), Config.MaxSpeed, Profile->GetNumberField(TEXT("speed")));
        TestEqual(TEXT("legacy acceleration preserved"), Config.Acceleration, Profile->GetNumberField(TEXT("acceleration")));
        TestEqual(TEXT("legacy coast deceleration preserved"), Config.CoastDeceleration, Profile->GetNumberField(TEXT("coastDeceleration")));
        TestEqual(TEXT("legacy braking preserved"), Config.Braking, Profile->GetNumberField(TEXT("braking")));
        TestEqual(TEXT("legacy lateral tuning preserved"), Config.LateralAcceleration, Profile->GetNumberField(TEXT("lateralAcceleration")));
        TestEqual(TEXT("legacy radius preserved"), Config.ShipRadiusMetres, Legacy->GetNumberField(TEXT("shipRadiusMetres")));
        TestEqual(TEXT("legacy turn rate preserved"), Config.TurnRateDegrees, Legacy->GetNumberField(TEXT("turnRateDegrees")));
        TestEqual(TEXT("legacy angular acceleration preserved"), Config.AngularAccelerationDegrees, Legacy->GetNumberField(TEXT("angularAccelerationDegrees")));
        TestEqual(TEXT("legacy pitch limit preserved"), Config.PitchLimitDegrees, Legacy->GetNumberField(TEXT("pitchLimitDegrees")));
        TestEqual(TEXT("legacy bank preserved"), Config.BankDegrees, Legacy->GetNumberField(TEXT("bankDegrees")));
        TestEqual(TEXT("legacy low speed turn multiplier preserved"), Config.LowSpeedTurnMultiplier, Legacy->GetNumberField(TEXT("lowSpeedTurnMultiplier")));
        TestEqual(TEXT("legacy high speed bank fraction preserved"), Config.HighSpeedBankFraction, Legacy->GetNumberField(TEXT("highSpeedBankFraction")));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShipFlightBehaviourTest, "Daedalus.Flight.AuroraDriveTurnAndNoDrift", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FShipFlightBehaviourTest::RunTest(const FString&)
{
    FShipFlightCatalog Catalog; FString Json, Error;
    if (!TestTrue(TEXT("ship tuning loaded"), ReadCatalog(Catalog, Json, Error)) || Catalog.Ships.Num() != 2) return false;
    const auto& Daedalus = Catalog.Ships[0]; const auto& Aurora = Catalog.Ships[1];
    TestTrue(TEXT("Aurora game hull and safety sphere are larger"), Aurora.LengthMetres > Daedalus.LengthMetres
        && Aurora.Profiles[0].ShipRadiusMetres > Daedalus.Profiles[0].ShipRadiusMetres);
    for (int32 Profile = 0; Profile < 3; ++Profile)
    {
        FFlightModel Flights[2];
        for (int32 Ship = 0; Ship < 2; ++Ship)
        {
            const auto& Config = Catalog.Ships[Ship].Profiles[Profile];
            if (!TestTrue(TEXT("profile initializes"), Flights[Ship].Initialize(Config, {}, {}, Error))) return false;
            Flights[Ship].SetThrottle(1, Error); Flights[Ship].Advance(1);
        }
        if (Profile > 0) TestTrue(TEXT("Aurora drive gains more speed in first second"),
            Flights[1].GetState().VelocityMetresPerSecond.Size() > Flights[0].GetState().VelocityMetresPerSecond.Size());
        Flights[1].Advance(1);
        TestTrue(TEXT("Aurora reaches full profile speed within two seconds"),
            FMath::Abs(Flights[1].GetState().VelocityMetresPerSecond.Size() - Aurora.Profiles[Profile].MaxSpeed) < .001);
        Flights[0].Advance(2); Flights[1].Advance(1);
        for (int32 Ship = 0; Ship < 2; ++Ship)
        {
            auto& Flight = Flights[Ship]; const auto& Config = Catalog.Ships[Ship].Profiles[Profile];
            TestTrue(TEXT("each ship reaches cap within three seconds"), FMath::Abs(Flight.GetState().VelocityMetresPerSecond.Size() - Config.MaxSpeed) < .001);
            Flight.SetInput({1, .7, false}, Error);
            bool bNoDrift = true, bSublight = true, bSpeedPreserved = true;
            for (int32 Step = 0; Step < 360; ++Step)
            {
                Flight.Advance(FFlightModel::FixedStepSeconds);
                const double Speed = Flight.GetState().VelocityMetresPerSecond.Size();
                bNoDrift &= NoDrift(Flight.GetState()); bSublight &= Speed < 299792458.0;
                bSpeedPreserved &= FMath::Abs(Speed - Config.MaxSpeed) < .001;
            }
            TestTrue(TEXT("full-speed compound steering preserves speed, no drift and sublight ceiling"), bNoDrift && bSublight && bSpeedPreserved);
            Flight.SetInput({-.7, -.4, true}, Error);
            bool bBrakingAligned = true;
            for (int32 Step = 0; Step < 1200; ++Step)
            {
                Flight.Advance(FFlightModel::FixedStepSeconds); bBrakingAligned &= NoDrift(Flight.GetState());
            }
            TestTrue(TEXT("steering brake stops without drift"), bBrakingAligned && Flight.GetState().VelocityMetresPerSecond.IsZero());
        }
    }
    TestTrue(TEXT("Aurora full impulse is 99.9 percent of light speed"), FMath::Abs(Aurora.Profiles[2].MaxSpeed / 299792458.0 - .999) < 1e-12);
    for (bool bAtMaxSpeed : {false, true})
    {
        FFlightModel Turns[2];
        for (int32 Ship = 0; Ship < 2; ++Ship)
        {
            const auto& Config = Catalog.Ships[Ship].Profiles[1]; FFlightState State;
            if (bAtMaxSpeed) { State.VelocityMetresPerSecond = FVector3d(Config.MaxSpeed, 0, 0); State.Throttle = 1; }
            if (!TestTrue(TEXT("comparison turn initializes"), Turns[Ship].Initialize(Config, State, {}, Error))) return false;
            Turns[Ship].SetInput({1, 0, false}, Error); Turns[Ship].Advance(3);
        }
        TestTrue(TEXT("Aurora manoeuvres more slowly at rest and full speed"), Turns[1].GetState().YawDegrees < Turns[0].GetState().YawDegrees
            && Turns[1].GetState().YawRateDegrees < Turns[0].GetState().YawRateDegrees
            && Turns[1].GetState().BankDegrees < Turns[0].GetState().BankDegrees);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShipFlightSwapTest, "Daedalus.Flight.ShipSwapSurfaceRollback", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FShipFlightSwapTest::RunTest(const FString&)
{
    FShipFlightCatalog Catalog; FString Json, Error;
    if (!TestTrue(TEXT("ship definitions for swap loaded"), ReadCatalog(Catalog, Json, Error)) || Catalog.Ships.Num() != 2) return false;
    const auto& Small = Catalog.Ships[0].Profiles[1]; const auto& Large = Catalog.Ships[1].Profiles[1];
    FFlightState Initial; Initial.PositionMetres = FVector3d(100000, 0, 0); Initial.Throttle = .2;
    const FFlightBody Body{TEXT("test.body"), Initial.PositionMetres + FVector3d(1500, 0, 0), 1000};
    FFlightModel Flight;
    if (!TestTrue(TEXT("Daedalus clears nearby body"), Flight.Initialize(Small, Initial, {Body}, Error))) return false;
    Flight.SetInput({.2, .1, false}, Error); Flight.Advance(.003); Flight.SetPaused(true);
    const auto Before = Flight; const auto State = Flight.GetState(); const double Pending = Flight.GetPendingSeconds();
    TestFalse(TEXT("larger Aurora cannot be selected inside body safety radius"), Flight.SetConfig(Large, Error));
    TestTrue(TEXT("unsafe swap preserves full state, tuning and pending time"), SameState(State, Flight.GetState())
        && SameConfig(Small, Flight.GetConfig()) && Flight.GetPendingSeconds() == Pending);
    Flight.Advance(1);
    TestTrue(TEXT("unsafe swap preserves pause"), SameState(State, Flight.GetState()) && Flight.GetPendingSeconds() == Pending);
    auto Reference = Before; Flight.SetPaused(false); Reference.SetPaused(false);
    Flight.Advance(.5); Reference.Advance(.5);
    TestTrue(TEXT("rejected swap preserves steering input and future motion"), SameState(Flight.GetState(), Reference.GetState()));

    FFlightModel Clear;
    if (!TestTrue(TEXT("open-space flight initialized"), Clear.Initialize(Small, Initial, {}, Error))) return false;
    Clear.SetThrottle(.5, Error); Clear.SetInput({.1, .05, false}, Error); Clear.Advance(.1); Clear.Advance(.003);
    const auto ClearState = Clear.GetState(); const double ClearPending = Clear.GetPendingSeconds();
    TestTrue(TEXT("valid Aurora configuration switch accepted"), Clear.SetConfig(Large, Error));
    TestTrue(TEXT("valid ship swap preserves pose, speed, throttle, clock and backlog"), SameState(ClearState, Clear.GetState())
        && Clear.GetPendingSeconds() == ClearPending && SameConfig(Large, Clear.GetConfig()));
    return true;
}
#endif

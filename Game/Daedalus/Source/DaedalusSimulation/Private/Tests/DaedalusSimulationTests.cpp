#include "DaedalusSimulation.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include <limits>

namespace
{
using namespace Daedalus;
const FString Fixture = TEXT(R"JSON({
"version":1,
"galaxies":[{"id":"milky-way","name":"Milky Way"}],
"systems":[{"id":"sol","name":"Sol","galaxyId":"milky-way","positionLy":[0,0,0],"bodies":[{"id":"earth","name":"Earth","radiusMetres":6371000,"positionMetres":[1000000,0,0]}]},
{"id":"alpha","name":"Alpha","galaxyId":"milky-way","positionLy":[4.3,0,0],"bodies":[]}],
"weapons":[{"id":"beam","damage":70,"rangeMetres":10000,"energyCost":20,"cooldownSeconds":1}],
"ships":[{"id":"scout","name":"Scout","lengthMetres":100,"maxSpeedMetresPerSecond":1000,"accelerationMetresPerSecondSquared":100,"hullCapacity":100,"shieldCapacity":50,"energyCapacity":100,"weaponId":"beam"}],
"locations":[{"id":"surface","name":"Outpost","systemId":"sol","positionMetres":[1000,0,0],"kind":"surface"},{"id":"distant","name":"Distant","systemId":"alpha","positionMetres":[0,0,0],"kind":"interior"}],
"initialShips":[{"id":"player","definitionId":"scout","systemId":"sol","positionMetres":[0,0,0],"player":true},{"id":"enemy","definitionId":"scout","systemId":"sol","positionMetres":[100,0,0],"player":false},{"id":"inactive","definitionId":"scout","systemId":"alpha","positionMetres":[200,0,0],"player":false}]
})JSON");
FCatalog MakeCatalog()
{
    FCatalog C; FString Error; C.LoadJson(Fixture, Error); return C;
}
FString ModifySave(const FString& Save, TFunctionRef<void(TSharedPtr<FJsonObject>)> Edit)
{
    TSharedPtr<FJsonObject> O;
    FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Save), O);
    Edit(O);
    FString Result; FJsonSerializer::Serialize(O.ToSharedRef(), TJsonWriterFactory<>::Create(&Result)); return Result;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCatalogValidationTest, "Daedalus.Foundation.CatalogValidation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCatalogValidationTest::RunTest(const FString& Parameters)
{
    FCatalog C; FString Error;
    TestTrue(TEXT("Valid v1 catalog"), C.LoadJson(Fixture, Error));
    TestEqual(TEXT("Two systems parsed"), C.Systems.Num(), 2);
    TestFalse(TEXT("Future version rejected"), C.LoadJson(Fixture.Replace(TEXT("\"version\":1"), TEXT("\"version\":2")), Error));
    TestEqual(TEXT("Rejected catalog retains prior data"), C.Systems.Num(), 2);
    TestFalse(TEXT("Duplicate stable IDs rejected"), C.LoadJson(Fixture.Replace(TEXT("\"id\":\"alpha\""), TEXT("\"id\":\"sol\"")), Error));
    TestFalse(TEXT("Missing definition rejected"), C.LoadJson(Fixture.Replace(TEXT("\"definitionId\":\"scout\""), TEXT("\"definitionId\":\"missing\"")), Error));
    TestFalse(TEXT("Negative capacity rejected"), C.LoadJson(Fixture.Replace(TEXT("\"hullCapacity\":100"), TEXT("\"hullCapacity\":-1")), Error));
    TestFalse(TEXT("Unsupported location rejected"), C.LoadJson(Fixture.Replace(TEXT("\"kind\":\"surface\""), TEXT("\"kind\":\"bad\"")), Error));
    TestFalse(TEXT("Non-finite parsed input rejected"), C.LoadJson(Fixture.Replace(TEXT("\"radiusMetres\":6371000"), TEXT("\"radiusMetres\":1e999")), Error));
    FCatalog Direct = C; Direct.ShipDefinitions.FindChecked(TEXT("scout")).HullCapacity = std::numeric_limits<double>::infinity();
    TestFalse(TEXT("Direct catalog validation blocks non-finite values"), Direct.Validate(Error));
    Direct = C; Direct.InitialShips.Add(Direct.InitialShips[0]);
    TestFalse(TEXT("Duplicate instance rejected"), Direct.Validate(Error));
    TestEqual(TEXT("Failed parses never replace valid catalog"), C.InitialShips.Num(), 3);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFixedStepPauseTest, "Daedalus.Foundation.FixedStepPause", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFixedStepPauseTest::RunTest(const FString& Parameters)
{
    FSimulation A, B; FString Error; const FCatalog C = MakeCatalog();
    TestTrue(TEXT("Initialize A"), A.Initialize(C, Error)); TestTrue(TEXT("Initialize B"), B.Initialize(C, Error));
    A.SetFlightInput(FVector3d(1,0,0)); B.SetFlightInput(FVector3d(1,0,0));
    TestEqual(TEXT("One second processed as sixty steps"), A.Advance(1), 60);
    for (int32 I=0; I<100; ++I) B.Advance(0.01);
    TestTrue(TEXT("Frame partitions preserve flight state"), A.GetSnapshot().Ships.FindChecked(TEXT("player")).PositionMetres.Equals(B.GetSnapshot().Ships.FindChecked(TEXT("player")).PositionMetres, 1e-8));
    const FSnapshot Before = A.GetSnapshot(); A.SetPaused(true);
    TestEqual(TEXT("Pause performs no steps"), A.Advance(20), 0);
    TestEqual(TEXT("Pause preserves clock"), A.GetSnapshot().SimulationSeconds, Before.SimulationSeconds);
    TestEqual(TEXT("Pause rejects travel"), A.Travel(TEXT("alpha"), Error), false);
    A.SetPaused(false);
    TestEqual(TEXT("Negative delta rejected"), A.Advance(-1), 0);
    TestEqual(TEXT("Non-finite delta rejected"), A.Advance(std::numeric_limits<double>::infinity()), 0);
    TestEqual(TEXT("Overload is bounded per update"), A.Advance(20), FSimulation::MaxStepsPerAdvance);
    TestTrue(TEXT("Overload retains backlog"), A.GetSnapshot().PendingSeconds > 9);
    TestEqual(TEXT("Zero delta drains retained backlog"), A.Advance(0), FSimulation::MaxStepsPerAdvance);
    TestTrue(TEXT("Total elapsed time preserved"), FMath::IsNearlyEqual(A.GetSnapshot().SimulationSeconds, 21.0, 1e-7));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatTest, "Daedalus.Foundation.Combat", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatTest::RunTest(const FString& Parameters)
{
    FString Error; FCatalog C = MakeCatalog(); FSimulation S; S.Initialize(C, Error);
    TestFalse(TEXT("Self target rejected"), S.FireAt(TEXT("player"), Error));
    TestFalse(TEXT("Other system target rejected"), S.FireAt(TEXT("inactive"), Error));
    TestTrue(TEXT("First hit"), S.FireAt(TEXT("enemy"), Error));
    TestTrue(TEXT("Shield absorbs first 50 damage"), FMath::IsNearlyEqual(S.GetSnapshot().Ships.FindChecked(TEXT("enemy")).Hull, 80.0));
    TestTrue(TEXT("Shot consumes energy"), FMath::IsNearlyEqual(S.GetSnapshot().Ships.FindChecked(TEXT("player")).Energy, 80.0));
    TestFalse(TEXT("Cooldown blocks immediate second hit"), S.FireAt(TEXT("enemy"), Error));
    S.Advance(1); TestTrue(TEXT("Cooldown expires"), S.FireAt(TEXT("enemy"), Error));
    S.Advance(1); TestTrue(TEXT("Final hit destroys enemy"), S.FireAt(TEXT("enemy"), Error));
    TestFalse(TEXT("Destroyed target cannot be hit"), S.FireAt(TEXT("enemy"), Error));
    const FVector3d Wreck = S.GetSnapshot().Ships.FindChecked(TEXT("enemy")).PositionMetres; S.Advance(1);
    TestTrue(TEXT("Destroyed object does not move"), Wreck.Equals(S.GetSnapshot().Ships.FindChecked(TEXT("enemy")).PositionMetres));
    FString Save; S.Serialize(Save, Error);
    const FString DeadPlayer = ModifySave(Save, [](TSharedPtr<FJsonObject> O) { for (auto& V : O->GetArrayField(TEXT("ships"))) { const auto Ship=V->AsObject(); if (Ship->GetStringField(TEXT("id"))==TEXT("player")) { Ship->SetNumberField(TEXT("hull"),0); Ship->SetNumberField(TEXT("shield"),0); } } });
    TestTrue(TEXT("Destroyed player saved state accepted"), S.Restore(DeadPlayer, Error));
    TestFalse(TEXT("Destroyed player cannot fire"), S.FireAt(TEXT("inactive"), Error));
    TestFalse(TEXT("Destroyed player cannot travel"), S.Travel(TEXT("alpha"), Error));
    const FVector3d DeadPosition = S.GetSnapshot().Ships.FindChecked(TEXT("player")).PositionMetres; S.SetFlightInput(FVector3d(1,0,0)); S.Advance(1);
    TestTrue(TEXT("Destroyed player cannot accelerate"), DeadPosition.Equals(S.GetSnapshot().Ships.FindChecked(TEXT("player")).PositionMetres));
    C.ShipDefinitions.FindChecked(TEXT("scout")).EnergyCapacity = 1; S.Initialize(C, Error);
    TestFalse(TEXT("Insufficient energy rejects shot"), S.FireAt(TEXT("enemy"), Error));
    C = MakeCatalog(); C.InitialShips[1].PositionMetres = FVector3d(20000,0,0); S.Initialize(C, Error);
    TestFalse(TEXT("Range rejects distant target"), S.FireAt(TEXT("enemy"), Error));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTravelTransportTest, "Daedalus.Foundation.TravelTransportPersistence", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTravelTransportTest::RunTest(const FString& Parameters)
{
    FString Error; FCatalog C = MakeCatalog(); FSimulation S; S.Initialize(C, Error);
    TestTrue(TEXT("Damage prior to departure"), S.FireAt(TEXT("enemy"), Error));
    const FShipState PriorEnemy = S.GetSnapshot().Ships.FindChecked(TEXT("enemy"));
    TestTrue(TEXT("Travel to other system"), S.Travel(TEXT("alpha"), Error));
    TestEqual(TEXT("Only other system instances active"), S.GetActiveShipIds().Num(), 2);
    S.Advance(1);
    TestEqual(TEXT("Inactive damage retained"), S.GetSnapshot().Ships.FindChecked(TEXT("enemy")).Hull, PriorEnemy.Hull);
    TestEqual(TEXT("Inactive shields not secretly regenerated"), S.GetSnapshot().Ships.FindChecked(TEXT("enemy")).Shield, PriorEnemy.Shield);
    TestTrue(TEXT("Revisit origin"), S.Travel(TEXT("sol"), Error));
    TestEqual(TEXT("Revisit retained enemy state"), S.GetSnapshot().Ships.FindChecked(TEXT("enemy")).Hull, PriorEnemy.Hull);
    TestFalse(TEXT("Cross-system transporter rejected"), S.Transport(TEXT("distant"), Error));
    const FShipState ShipBefore = S.GetSnapshot().Ships.FindChecked(TEXT("player"));
    TestTrue(TEXT("Transport to local surface"), S.Transport(TEXT("surface"), Error));
    TestTrue(TEXT("Ship remains in orbit"), S.GetSnapshot().Ships.FindChecked(TEXT("player")).PositionMetres.Equals(ShipBefore.PositionMetres));
    TestEqual(TEXT("Transport saves player location"), S.GetSnapshot().PlayerLocationId, FString(TEXT("surface")));
    TestFalse(TEXT("Cannot fly ship while away"), S.Travel(TEXT("alpha"), Error));
    FString Save; TestTrue(TEXT("Serialize transported state"), S.Serialize(Save, Error));
    FSimulation Restored; Restored.Initialize(C, Error); TestTrue(TEXT("Restore transported state"), Restored.Restore(Save, Error));
    TestEqual(TEXT("Restored player location"), Restored.GetSnapshot().PlayerLocationId, FString(TEXT("surface")));
    TestTrue(TEXT("Return aboard"), Restored.ReturnToShip(Error));
    TestTrue(TEXT("Player aboard after return"), Restored.GetSnapshot().PlayerLocationId.IsEmpty());
    TestEqual(TEXT("Damaged enemy survives save roundtrip"), Restored.GetSnapshot().Ships.FindChecked(TEXT("enemy")).Hull, PriorEnemy.Hull);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveValidationTest, "Daedalus.Foundation.SaveValidation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FSaveValidationTest::RunTest(const FString& Parameters)
{
    FString Error, Save; FSimulation S; S.Initialize(MakeCatalog(), Error); S.Advance(0.0123); S.SetPaused(true); S.Serialize(Save, Error);
    auto Reject = [&](const FString& Bad, const TCHAR* Name) { TestFalse(Name, S.Restore(Bad, Error)); FString After; S.Serialize(After, Error); TestEqual(TEXT("Failure retains entire previous save"), After, Save); };
    Reject(ModifySave(Save, [](TSharedPtr<FJsonObject> O) { O->SetNumberField(TEXT("version"), 2); }), TEXT("Future save version rejected"));
    Reject(ModifySave(Save, [](TSharedPtr<FJsonObject> O) { O->SetNumberField(TEXT("clock"), -1); }), TEXT("Negative clock rejected"));
    Reject(ModifySave(Save, [](TSharedPtr<FJsonObject> O) { O->SetStringField(TEXT("playerId"), TEXT("enemy")); }), TEXT("Player identity substitution rejected"));
    Reject(ModifySave(Save, [](TSharedPtr<FJsonObject> O) { O->SetStringField(TEXT("locationId"), TEXT("distant")); }), TEXT("Wrong-system location rejected"));
    Reject(ModifySave(Save, [](TSharedPtr<FJsonObject> O) { auto A=O->GetArrayField(TEXT("ships")); A.Add(A[0]); O->SetArrayField(TEXT("ships"),A); }), TEXT("Duplicate saved instance rejected"));
    Reject(ModifySave(Save, [](TSharedPtr<FJsonObject> O) { auto A=O->GetArrayField(TEXT("ships")); A.RemoveAt(0); O->SetArrayField(TEXT("ships"),A); }), TEXT("Missing saved instance rejected"));
    Reject(ModifySave(Save, [](TSharedPtr<FJsonObject> O) { O->GetArrayField(TEXT("ships"))[0]->AsObject()->SetNumberField(TEXT("energy"),1000); }), TEXT("Capacity overflow rejected"));
    Reject(ModifySave(Save, [](TSharedPtr<FJsonObject> O) { O->GetArrayField(TEXT("ships"))[0]->AsObject()->SetStringField(TEXT("systemId"),TEXT("unknown")); }), TEXT("Unknown saved system rejected"));
    TestTrue(TEXT("Valid snapshot accepted"), S.Restore(Save, Error));
    TestTrue(TEXT("Pause persisted"), S.GetSnapshot().bPaused);
    TestTrue(TEXT("Fractional time persisted"), FMath::IsNearlyEqual(S.GetSnapshot().PendingSeconds,0.0123,1e-12));
    S.SetPaused(false); TestEqual(TEXT("Fraction continues after restore"), S.Advance(0.0044),1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLargeCatalogTest, "Daedalus.Foundation.LargeCatalogCoordinates", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FLargeCatalogTest::RunTest(const FString& Parameters)
{
    FCatalog C = MakeCatalog();
    for (int32 I=0; I<2000; ++I)
    {
        FSystemDefinition D; D.Id=FString::Printf(TEXT("system-%d"),I); D.Name=D.Id; D.GalaxyId=TEXT("milky-way"); D.PositionLy=FVector3d(I*1000,0,0);
        C.Systems.Add(D.Id,D);
    }
    FString Error; FSimulation S; TestTrue(TEXT("Large catalog initializes without scene assets"),S.Initialize(C,Error));
    TestEqual(TEXT("All system definitions available"),S.GetCatalog().Systems.Num(),2002);
    TestEqual(TEXT("Only local ship instances active"),S.GetActiveShipIds().Num(),2);
    const FVector3d Origin(1e12,-1e12,1e12), Position=Origin+FVector3d(0.125,10,-100);
    const FVector3d Relative=MetresToCentimetresRelative(Position,Origin);
    TestTrue(TEXT("Local centimetre conversion"),Relative.Equals(FVector3d(12.5,1000,-10000),1e-8));
    TestTrue(TEXT("Coordinate roundtrip at astronomical offset"),CentimetresRelativeToMetres(Relative,Origin).Equals(Position,1e-8));
    TestTrue(TEXT("Travel to arbitrary catalog system"),S.Travel(TEXT("system-1999"),Error));
    TestEqual(TEXT("Unpopulated systems need only player instance"),S.GetActiveShipIds().Num(),1);
    return true;
}
#endif

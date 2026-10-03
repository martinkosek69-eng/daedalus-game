#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Persistence/DaedalusSaveStore.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDaedalusPersistenceTest, "Daedalus.Foundation.Persistence.Recovery", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDaedalusPersistenceTest::RunTest(const FString&)
{
    FString Json, Error;
    Daedalus::FCatalog Catalog;
    if (!FFileHelper::LoadFileToString(Json, *FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Data/foundation.json"))) ||
        !Catalog.LoadJson(Json, Error)) { AddError(Error); return false; }
    Daedalus::FSimulation World;
    if (!World.Initialize(Catalog, Error)) { AddError(Error); return false; }
    const FString Directory = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Automation"), FGuid::NewGuid().ToString());
    TestTrue(TEXT("First save"), FDaedalusSaveStore::Save(Directory, World, Error));
    World.Advance(1);
    TestTrue(TEXT("Second save"), FDaedalusSaveStore::Save(Directory, World, Error));
    World.Advance(1);
    TestTrue(TEXT("Newest generation loaded"), FDaedalusSaveStore::Load(Directory, World, Error));
    TestTrue(TEXT("Newest time"), FMath::IsNearlyEqual(World.GetSnapshot().SimulationSeconds, 1., .001));
    const FString LatestPath = FPaths::Combine(Directory, TEXT("foundation-1.sav"));
    FString Original, Protected;
    FFileHelper::LoadFileToString(Original, *LatestPath);
    const FString Future = Original.Replace(TEXT("\"version\": 1"), TEXT("\"version\": 2"));
    TestNotEqual(TEXT("Future envelope fixture changed"), Future, Original);
    FFileHelper::SaveStringToFile(Future, *LatestPath);
    TestFalse(TEXT("Future envelope protected from overwrite"), FDaedalusSaveStore::Save(Directory, World, Error));
    FFileHelper::LoadFileToString(Protected, *LatestPath);
    TestEqual(TEXT("Future bytes retained"), Protected, Future);
    TestTrue(TEXT("Older compatible generation still loads"), FDaedalusSaveStore::Load(Directory, World, Error));
    FFileHelper::SaveStringToFile(Original, *LatestPath);
    FFileHelper::SaveStringToFile(Original.Replace(TEXT("\"generation\": \"2\""), TEXT("\"generation\": \"3\"")), *LatestPath);
    TestTrue(TEXT("Generation tampering recovers prior generation"), FDaedalusSaveStore::Load(Directory, World, Error));
    TestEqual(TEXT("Tampered generation not trusted"), World.GetSnapshot().SimulationSeconds, 0.);
    FFileHelper::SaveStringToFile(Original, *LatestPath);
    FFileHelper::SaveStringToFile(TEXT("corrupt"), *FPaths::Combine(Directory, TEXT("foundation-1.sav")));
    TestTrue(TEXT("Corrupt newest recovers previous"), FDaedalusSaveStore::Load(Directory, World, Error));
    TestEqual(TEXT("Previous time"), World.GetSnapshot().SimulationSeconds, 0.);
    FFileHelper::SaveStringToFile(TEXT("corrupt"), *FPaths::Combine(Directory, TEXT("foundation-0.sav")));
    World.Advance(.5);
    FString Before, After; World.Serialize(Before, Error);
    TestFalse(TEXT("Both corrupt rejected"), FDaedalusSaveStore::Load(Directory, World, Error));
    World.Serialize(After, Error); TestEqual(TEXT("Rejected load leaves live state"), After, Before);
    // A file in place of the requested directory deterministically exercises write failure.
    const FString Blocked = FPaths::Combine(Directory, TEXT("not-a-directory"));
    FFileHelper::SaveStringToFile(TEXT("block"), *Blocked);
    TestFalse(TEXT("Write failure reported"), FDaedalusSaveStore::Save(Blocked, World, Error));
    IFileManager::Get().DeleteDirectory(*Directory, false, true); // unique own automation output only
    return true;
}
#endif

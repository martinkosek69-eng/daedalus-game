#include "World/DaedalusWorldSubsystem.h"
#include "Persistence/DaedalusSaveStore.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

void UDaedalusWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    FString Json;
    Daedalus::FCatalog Catalog;
    bReady = FFileHelper::LoadFileToString(Json, *FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Data/foundation.json"))) &&
        Catalog.LoadJson(Json, LastMessage) && Simulation.Initialize(Catalog, LastMessage);
    if (!bReady && LastMessage.IsEmpty()) LastMessage = TEXT("Runtime catalog unavailable");
    UE_LOG(LogTemp, Display, TEXT("DAEDALUS_CATALOG %s"), bReady ? TEXT("READY") : *LastMessage);
}

FString UDaedalusWorldSubsystem::SaveDirectory() const
{
    FString Override = FPlatformMisc::GetEnvironmentVariable(TEXT("DAEDALUS_SAVE_DIR"));
    FParse::Value(FCommandLine::Get(), TEXT("DaedalusSaveDir="), Override);
    // Editor experiments never overwrite a player's packaged-game saves.
    if (GetWorld() && GetWorld()->WorldType == EWorldType::PIE)
        return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SaveGames/EditorFoundation")));
    return FPaths::ConvertRelativePathToFull(Override.IsEmpty()
        ? FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SaveGames/Foundation")) : Override);
}
bool UDaedalusWorldSubsystem::Save()
{
    const bool Ok = bReady && FDaedalusSaveStore::Save(SaveDirectory(), Simulation, LastMessage);
    if (Ok) LastMessage = TEXT("Saved: verified generation");
    return Ok;
}
bool UDaedalusWorldSubsystem::Load()
{
    const bool Ok = bReady && FDaedalusSaveStore::Load(SaveDirectory(), Simulation, LastMessage);
    if (Ok) { ++SceneRevision; LastMessage = TEXT("Loaded: world restored"); }
    return Ok;
}
bool UDaedalusWorldSubsystem::Travel()
{
    if (!bReady) return false;
    const auto& State = Simulation.GetSnapshot();
    const FString Current = State.Ships.FindChecked(State.PlayerShipId).SystemId;
    TArray<FString> Ids; Simulation.GetCatalog().Systems.GetKeys(Ids); Ids.Sort();
    const int32 Next = (Ids.IndexOfByKey(Current) + 1) % Ids.Num();
    if (!Simulation.Travel(Ids[Next], LastMessage)) return false;
    ++SceneRevision; LastMessage = TEXT("Travel: system changed, world state preserved"); return true;
}
bool UDaedalusWorldSubsystem::Transport()
{
    if (!bReady) return false;
    bool Ok = false;
    if (!Simulation.GetSnapshot().PlayerLocationId.IsEmpty()) Ok = Simulation.ReturnToShip(LastMessage);
    else
    {
        const auto& State = Simulation.GetSnapshot();
        const FString System = State.Ships.FindChecked(State.PlayerShipId).SystemId;
        TArray<FString> Ids; Simulation.GetCatalog().Locations.GetKeys(Ids); Ids.Sort();
        for (const FString& Id : Ids) if (Simulation.GetCatalog().Locations[Id].SystemId == System)
        { Ok = Simulation.Transport(Id, LastMessage); break; }
    }
    if (Ok) { ++SceneRevision; LastMessage = TEXT("Transport: ship remains in orbit"); }
    return Ok;
}
bool UDaedalusWorldSubsystem::Fire()
{
    if (!bReady) return false;
    TArray<FString> Ids = Simulation.GetActiveShipIds(); Ids.Sort();
    for (const FString& Id : Ids)
    {
        if (Id != Simulation.GetSnapshot().PlayerShipId && Simulation.GetSnapshot().Ships[Id].IsAlive())
        {
            const bool Ok = Simulation.FireAt(Id, LastMessage);
            if (Ok) LastMessage = TEXT("Hit: shield/hull and energy updated");
            return Ok;
        }
    }
    LastMessage = TEXT("No live target"); return false;
}

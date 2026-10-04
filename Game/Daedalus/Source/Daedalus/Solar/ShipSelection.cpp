#include "Solar/SolarFlightGameMode.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

bool ASolarFlightGameMode::LoadShipCatalog()
{
    FString Json;
    if (!FFileHelper::LoadFileToString(Json, *(FPaths::ProjectContentDir() / TEXT("Data/Solar/ships.json")))
        || !ShipCatalog.LoadJson(Json, Message) || ShipCatalog.Ships.Num() != 2
        || ShipCatalog.Ships[0].Id != TEXT("daedalus") || ShipCatalog.Ships[1].Id != TEXT("aurora")) return false;
    UseShipDefinition(0);
    return true;
}

void ASolarFlightGameMode::UseShipDefinition(int32 Index)
{
    const auto& Definition = ShipCatalog.Ships[Index];
    ActiveShip = Index; ShipId = Definition.Id; ShipName = Definition.Name;
    ShipLengthMetres = Definition.LengthMetres;
    Profiles.Reset();
    for (int32 I = 0; I < Definition.Profiles.Num(); ++I)
        Profiles.Add({Definition.ProfileNames[I], Definition.Profiles[I]});
    // Proportions of the original approved camera framing, expressed in ship lengths.
    CameraDefaultMetres = ShipLengthMetres * 2.5;
    CameraMinMetres = ShipLengthMetres * (650.0 / 600.0);
    CameraMaxMetres = ShipLengthMetres * 10;
}

bool ASolarFlightGameMode::CreateShipView()
{
    auto* Asset = ActiveShip == 0 ? ShipAsset.Get() : AuroraAsset.Get();
    if (!Asset || FMath::Abs(Asset->GetBounds().BoxExtent.X * 2 - ShipLengthMetres * 100) > 50) return false;
    Ship = MakeMesh(Asset, nullptr); Ship->SetCastShadow(true); Ship->SetTextureForceResidentFlag(true);
    if (ActiveShip == 1) return true; // Saved Aurora has no separated engine/door animations.
    if (!LightsAsset || GlowAssets.Num() != 6 || GlowAssets.Contains(nullptr)) return false;
    HullLights = MakeMesh(LightsAsset, nullptr); HullLights->SetTextureForceResidentFlag(true);
    if (!CreateShipPresentation()) return false;
    for (const auto& Detail : ShipDetails) Detail->SetTextureForceResidentFlag(true);
    for (const auto& GlowAsset : GlowAssets)
    {
        auto* Glow = MakeMesh(GlowAsset, nullptr); Glow->SetTextureForceResidentFlag(true);
        for (int32 Slot = 0; Slot < Glow->GetNumMaterials(); ++Slot)
        {
            auto* Dynamic = UMaterialInstanceDynamic::Create(Glow->GetMaterial(Slot), this);
            Glow->SetMaterial(Slot, Dynamic); EngineDynamics.Add(Dynamic);
        }
        EngineGlows.Add(Glow);
    }
    return true;
}

void ASolarFlightGameMode::ClearShipView()
{
    auto Destroy = [](UStaticMeshComponent* Component) {
        if (Component) { Component->SetTextureForceResidentFlag(false); Component->GetOwner()->Destroy(); }
    };
    Destroy(Ship); Destroy(HullLights); Ship = nullptr; HullLights = nullptr;
    for (const auto& Component : EngineGlows) Destroy(Component);
    for (const auto& Component : ShipDetails) Destroy(Component);
    for (const auto& Component : HangarDoors) Destroy(Component);
    for (const auto& Component : ShipPointLights) Component->DestroyComponent();
    EngineGlows.Reset(); EngineDynamics.Reset(); ShipDetails.Reset(); HangarDoors.Reset();
    BeaconDynamics.Reset(); ShipPointLights.Reset(); DoorOpenOffsets.Reset(); ShipLightPositions.Reset();
    ShipLightCandela.Reset(); ShipLightIsEngine.Reset();
    DoorTravel = 0; bHangarsOpen = false; BeaconLevel = 0;
}

bool ASolarFlightGameMode::SelectShip(int32 Index)
{
    if (!bReady || Galaxy.bOpen || !ShipCatalog.Ships.IsValidIndex(Index) || Index == ActiveShip) return false;
    // Prepare a domain transaction first. Larger clearance is checked against
    // canonical bodies before any visual change; reject swaps inside a planet.
    auto Candidate = Flight;
    FString Error;
    if (!Candidate.SetConfig(ShipCatalog.Ships[Index].Profiles[ProfileIndex], Error))
    { Message = TEXT("Změna lodi není bezpečná: dokonči otočku nebo odleť dál od povrchu."); return false; }
    const int32 Previous = ActiveShip;
    ClearShipView(); UseShipDefinition(Index);
    if (!CreateShipView())
    {
        ClearShipView(); UseShipDefinition(Previous); bReady = CreateShipView();
        Message = TEXT("Model vybrané lodi nelze načíst."); return false;
    }
    Flight = MoveTemp(Candidate); // Same position, heading, speed, throttle, clock and navigation.
    if (auto* PC = GetWorld()->GetFirstPlayerController())
        if (auto* Pawn = Cast<ASolarFlightPawn>(PC->GetPawn()))
        {
            Pawn->CameraDistanceMetres = CameraDefaultMetres; Pawn->ResetCamera();
            const auto& State = Flight.GetState();
            const FRotator Rotation(State.PitchDegrees-18.3,State.YawDegrees,0);
            Pawn->Camera->SetWorldLocationAndRotation(FVector(0,0,-ShipLengthMetres*10)-Rotation.Vector()*CameraDefaultMetres*100,Rotation);
        }
    Message.Empty(); UpdateScene(0);
    return true;
}

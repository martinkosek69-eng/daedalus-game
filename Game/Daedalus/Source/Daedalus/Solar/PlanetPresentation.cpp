#include "Solar/SolarFlightGameMode.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UnrealClient.h"

// Planet presentation layers. Everything here is derived from the canonical body definitions
// each frame and never feeds back into flight, navigation or catalog state.
namespace
{
// Cloud shell radius / body radius; matches CLOUD_SHELL_SCALE in Tools/Prepare-PlanetMaterials.py.
constexpr double CloudShellScale = 1.0015;
// Projected limb radius (pixels) at which the next finer sphere is used, with hysteresis. With
// N segments the polygonal limb error is about R * pi^2 / (2 N^2): below 0.25 px at these limits.
constexpr double FinerSphereAt[] = {800, 3200};
constexpr double CoarserSphereAt[] = {700, 2800};
constexpr int32 CloudSortPriority = -2, AirSortPriority = -1;

FString LayerMaterialPath(const FSolarBodyDefinition& Body, const TCHAR* Prefix)
{
    return Body.MaterialPath.Replace(TEXT("M_Body_"), Prefix, ESearchCase::CaseSensitive);
}
bool UsesSphere(const FSolarBodyDefinition& Body)
{
    return Body.MeshPath.IsEmpty() && Body.Kind != TEXT("asteroid") && Body.Kind != TEXT("minor-body") && Body.Kind != TEXT("comet");
}
}

void ASolarFlightGameMode::PreparePlanetLayers()
{
    if (PlanetSpheres.IsEmpty())
    {
        // Level 0 is the original shared sphere; finer optional levels fall back to it if absent.
        PlanetSpheres.Add(SphereAsset);
        for (const TCHAR* Path : {TEXT("/Game/Solar/Models/SM_PlanetSphere_256.SM_PlanetSphere_256"),
                                  TEXT("/Game/Solar/Models/SM_PlanetSphere_512.SM_PlanetSphere_512")})
        {
            UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Path);
            PlanetSpheres.Add(Mesh ? Mesh : PlanetSpheres.Last().Get());
        }
    }
    CloudMeshes.SetNumZeroed(BodyDefinitions.Num()); CloudDynamics.SetNumZeroed(BodyDefinitions.Num());
    PlanetSphereLevels.Init(0, BodyDefinitions.Num()); PlanetPixelRadii.Init(0, BodyDefinitions.Num());
}

void ASolarFlightGameMode::CreatePlanetLayers(int32 Index)
{
    const auto& Body = BodyDefinitions[Index];
    if (AirMeshes[Index]) AirMeshes[Index]->SetTranslucentSortPriority(AirSortPriority);
    if (!UsesSphere(Body)) return;
    // Optional separate cloud layer: present only when the recipe built M_Cloud_<key>.
    const FString CloudPath = LayerMaterialPath(Body, TEXT("M_Cloud_"));
    if (CloudPath == Body.MaterialPath) return;
    if (UMaterialInterface* CloudMaterial = LoadObject<UMaterialInterface>(nullptr, *CloudPath, nullptr, LOAD_Quiet | LOAD_NoWarn))
    {
        CloudDynamics[Index] = UMaterialInstanceDynamic::Create(CloudMaterial, this);
        CloudMeshes[Index] = MakeMesh(SphereAsset, CloudDynamics[Index]);
        CloudMeshes[Index]->SetTranslucentSortPriority(CloudSortPriority);
    }
}

void ASolarFlightGameMode::UpdatePlanetLayers(int32 Index, const FVector& Location, const FQuat& Rotation, const FVector& Scale,
    const FLinearColor& LightDirection, const FVector& CameraPosition)
{
    const auto& Body = BodyDefinitions[Index];
    // Star light tint for lit planet masters, half way to white so authored maps keep their character.
    const FLinearColor Stellar = Systems.IsValidIndex(ActiveSystem) ? Systems[ActiveSystem].StellarColor : FLinearColor::White;
    const FLinearColor SunColor = FMath::Lerp(FLinearColor::White, Stellar / FMath::Max(Stellar.GetMax(), .001f), .5f);
    BodyDynamics[Index]->SetVectorParameterValue(TEXT("SunColor"), SunColor);

    // True projected pixel radius (the projection preserves angular size).
    double PixelRadius = 0;
    const double Distance = (Location - CameraPosition).Size();
    const double RadiusCentimetres = Scale.GetMax() * 100.0;
    if (const APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (PC->PlayerCameraManager && GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
        {
            const double HalfFov = FMath::DegreesToRadians(FMath::Clamp(double(PC->PlayerCameraManager->GetFOVAngle()), 1., 170.) * .5);
            const double Width = GEngine->GameViewport->Viewport->GetSizeXY().X;
            const double Angle = Distance > RadiusCentimetres ? FMath::Asin(RadiusCentimetres / Distance) : HALF_PI * .999;
            PixelRadius = FMath::Tan(Angle) / FMath::Tan(HalfFov) * Width * .5;
        }
    }
    PlanetPixelRadii[Index] = PixelRadius;
    int32 Level = PlanetSphereLevels[Index];
    while (Level < 2 && PixelRadius > FinerSphereAt[Level]) ++Level;
    while (Level > 0 && PixelRadius < CoarserSphereAt[Level - 1]) --Level;
    if (Level != PlanetSphereLevels[Index])
    {
        PlanetSphereLevels[Index] = Level;
        UStaticMesh* Sphere = PlanetSpheres[Level];
        if (UsesSphere(Body)) BodyMeshes[Index]->SetStaticMesh(Sphere);
        if (AirMeshes[Index]) AirMeshes[Index]->SetStaticMesh(Sphere);
        if (CloudMeshes[Index]) CloudMeshes[Index]->SetStaticMesh(Sphere);
    }
    if (CloudMeshes[Index])
    {
        CloudMeshes[Index]->SetWorldLocationAndRotation(Location, Rotation);
        CloudMeshes[Index]->SetWorldScale3D(Scale * CloudShellScale);
        CloudDynamics[Index]->SetVectorParameterValue(TEXT("SunDirection"), LightDirection);
        CloudDynamics[Index]->SetVectorParameterValue(TEXT("SunColor"), SunColor);
    }
}

void ASolarFlightGameMode::ClearPlanetLayers()
{
    for (const auto& Mesh : CloudMeshes)
    {
        if (!Mesh) continue;
        AActor* MeshOwner = Mesh->GetOwner();
        if (MeshOwner && MeshOwner != this) MeshOwner->Destroy(); else Mesh->DestroyComponent();
    }
    CloudMeshes.Reset(); CloudDynamics.Reset(); PlanetSphereLevels.Reset(); PlanetPixelRadii.Reset();
}

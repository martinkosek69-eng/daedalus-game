#include "Solar/SolarFlightGameMode.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
bool ReadVector(const TSharedPtr<FJsonObject>& O, const TCHAR* Key, FVector& Out)
{
    const TArray<TSharedPtr<FJsonValue>>* A = nullptr;
    if (!O->TryGetArrayField(Key, A) || A->Num() != 3) return false;
    for (int32 I = 0; I < 3; ++I)
        if (!(*A)[I]->TryGetNumber(Out[I]) || !FMath::IsFinite(Out[I])) return false;
    return true;
}
bool Positive(const TSharedPtr<FJsonObject>& O, const TCHAR* Key, double& Out)
{
    return O->TryGetNumberField(Key, Out) && FMath::IsFinite(Out) && Out > 0;
}
UStaticMesh* Mesh(const FString& Path)
{
    return Path.StartsWith(TEXT("/Game/Ships/Daedalus/")) ? LoadObject<UStaticMesh>(nullptr, *Path) : nullptr;
}
}

bool ASolarFlightGameMode::CreateShipPresentation()
{
    FString Text; TSharedPtr<FJsonObject> Doc;
    if (!FFileHelper::LoadFileToString(Text, *(FPaths::ProjectContentDir() / TEXT("Data/Solar/ship-details.json")))
        || !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Doc) || !Doc.IsValid()) return false;
    double Version = 0; const TArray<TSharedPtr<FJsonValue>> *Details = nullptr, *Doors = nullptr, *Lights = nullptr;
    if (!Doc->TryGetNumberField(TEXT("version"), Version) || Version != 2
        || !Doc->TryGetArrayField(TEXT("meshes"), Details) || Details->Num() != 2
        || !Doc->TryGetArrayField(TEXT("doors"), Doors) || Doors->Num() != 4
        || !Doc->TryGetArrayField(TEXT("pointLights"), Lights) || Lights->Num() != 10) return false;
    FString WindowPath;
    if (!Doc->TryGetStringField(TEXT("windowMesh"), WindowPath) || Mesh(WindowPath) != LightsAsset) return false;
    for (const auto& Row : *Details)
    {
        FString Path; if (!Row->TryGetString(Path)) return false;
        auto* Asset = Mesh(Path); if (!Asset) return false;
        auto* Detail = MakeMesh(Asset, nullptr); ShipDetails.Add(Detail);
        // Physical add-ons cast shadows; emissive window meshes do not.
        Detail->SetCastShadow(Path.Contains(TEXT("Daedalus_AddOns")));
    }
    for (const auto& Row : *Doors)
    {
        const auto O = Row->AsObject(); FString Path; FVector Offset;
        if (!O || !O->TryGetStringField(TEXT("mesh"), Path) || !ReadVector(O, TEXT("openOffsetCentimetres"), Offset)) return false;
        auto* Asset = Mesh(Path); if (!Asset || FMath::Abs(Offset.Size() - 560) > .01) return false;
        auto* Door = MakeMesh(Asset, nullptr); Door->SetCastShadow(true); Door->SetTextureForceResidentFlag(true);
        HangarDoors.Add(Door); DoorOpenOffsets.Add(Offset);
    }
    const TSharedPtr<FJsonObject>* Beacon = nullptr;
    if (!Doc->TryGetObjectField(TEXT("beacon"), Beacon)) return false;
    FString BeaconPath;
    if (!(*Beacon)->TryGetStringField(TEXT("mesh"), BeaconPath)
        || !Positive(*Beacon, TEXT("periodSeconds"), BeaconPeriod)
        || !Positive(*Beacon, TEXT("onSeconds"), BeaconOnSeconds) || BeaconOnSeconds >= BeaconPeriod
        || !Positive(*Beacon, TEXT("emissionOn"), BeaconEmissionOn)
        || !(*Beacon)->TryGetNumberField(TEXT("emissionOff"), BeaconEmissionOff) || BeaconEmissionOff != 0) return false;
    auto* BeaconAsset = Mesh(BeaconPath); if (!BeaconAsset) return false;
    auto* BeaconMesh = MakeMesh(BeaconAsset, nullptr); ShipDetails.Add(BeaconMesh);
    for (int32 Slot = 0; Slot < BeaconMesh->GetNumMaterials(); ++Slot)
    {
        auto* Dynamic = UMaterialInstanceDynamic::Create(BeaconMesh->GetMaterial(Slot), this);
        BeaconMesh->SetMaterial(Slot, Dynamic); BeaconDynamics.Add(Dynamic);
    }
    for (const auto& Row : *Lights)
    {
        const auto O = Row->AsObject(); FString Kind; FVector Position, Colour; double Candela = 0, Radius = 0;
        if (!O || !ReadVector(O, TEXT("positionCentimetres"), Position) || !ReadVector(O, TEXT("colourLinear"), Colour)
            || !Positive(O, TEXT("candela"), Candela) || !Positive(O, TEXT("radiusCentimetres"), Radius)
            || !O->TryGetStringField(TEXT("kind"), Kind) || (Kind != TEXT("hangar") && Kind != TEXT("engine"))) return false;
        auto* Light = NewObject<UPointLightComponent>(this);
        AddInstanceComponent(Light); Light->SetMobility(EComponentMobility::Movable);
        Light->SetIntensityUnits(ELightUnits::Candelas); Light->SetIntensity(Candela);
        Light->SetAttenuationRadius(Radius); Light->SetLightColor(FLinearColor(Colour.X, Colour.Y, Colour.Z));
        Light->SetCastShadows(true); Light->RegisterComponent();
        ShipPointLights.Add(Light); ShipLightPositions.Add(Position);
        // Blender's bay lights were authored against a much stronger scene sun.
        // Calibrate their visual intensity to this lab's fixed exposure/2.8-lux
        // directional sun; retain the original candela in the source manifest.
        ShipLightCandela.Add(Candela * (Kind == TEXT("hangar") ? .01 : 1));
        ShipLightIsEngine.Add(Kind == TEXT("engine"));
    }
    return true;
}

void ASolarFlightGameMode::ToggleHangars()
{
    // Model inspection only, until a gameplay hangar system owns authoritative state.
    if (bReady && !bPaused && !Galaxy.bOpen && !HangarDoors.IsEmpty()) bHangarsOpen = !bHangarsOpen;
}

void ASolarFlightGameMode::UpdateShipPresentation(float DeltaSeconds)
{
    const FQuat Rotation(Flight.GetState().Attitude());
    if (!bPaused && !Galaxy.bOpen)
        DoorTravel = FMath::Clamp(DoorTravel + (bHangarsOpen ? 1 : -1) * FMath::Min<double>(DeltaSeconds, .1) / 2.0, 0.0, 1.0);
    const double Ease = DoorTravel * DoorTravel * (3 - 2 * DoorTravel);
    for (int32 I = 0; I < HangarDoors.Num(); ++I)
        HangarDoors[I]->SetWorldLocationAndRotation(Rotation.RotateVector(DoorOpenOffsets[I] * Ease), Rotation);
    BeaconLevel = FMath::Fmod(Flight.GetState().SimulationSeconds, BeaconPeriod) < BeaconOnSeconds ? BeaconEmissionOn : BeaconEmissionOff;
    for (const auto& Dynamic : BeaconDynamics) Dynamic->SetScalarParameterValue(TEXT("BeaconLevel"), BeaconLevel);
    for (int32 I = 0; I < ShipPointLights.Num(); ++I)
    {
        ShipPointLights[I]->SetWorldLocation(Rotation.RotateVector(ShipLightPositions[I]));
        ShipPointLights[I]->SetIntensity(ShipLightCandela[I] * (ShipLightIsEngine[I] ? EngineGlowLevel : 1));
    }
}

#include "Solar/SolarFlightGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
constexpr double LocalLimit = 1e15;
constexpr double DecorativeLimit = 1e18;
bool ReadJson(const FString& RelativePath, TSharedPtr<FJsonObject>& Json)
{
    if (!RelativePath.StartsWith(TEXT("Data/")) || RelativePath.Contains(TEXT("..")) || RelativePath.Contains(TEXT("\\"))) return false;
    FString Text;
    return FFileHelper::LoadFileToString(Text, *(FPaths::ProjectContentDir() / RelativePath))
        && FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Json) && Json.IsValid();
}
bool Number(const TSharedPtr<FJsonObject>& Json, const TCHAR* Key, double& Value, double Min, double Max)
{
    return Json->TryGetNumberField(Key, Value) && FMath::IsFinite(Value) && Value >= Min && Value <= Max;
}
bool OptionalNumber(const TSharedPtr<FJsonObject>& Json, const TCHAR* Key, double& Value, double Min, double Max)
{
    return !Json->HasField(Key) || Number(Json, Key, Value, Min, Max);
}
bool Vector(const TSharedPtr<FJsonObject>& Json, const TCHAR* Key, FVector3d& Value, double Limit)
{
    const TArray<TSharedPtr<FJsonValue>>* Rows = nullptr;
    if (!Json->TryGetArrayField(Key, Rows) || Rows->Num() != 3) return false;
    for (int32 Index = 0; Index < 3; ++Index)
        if (!(*Rows)[Index]->TryGetNumber(Value[Index]) || !FMath::IsFinite(Value[Index]) || FMath::Abs(Value[Index]) > Limit) return false;
    return true;
}
bool ValidId(const FString& Id)
{
    if (Id.IsEmpty() || Id.Len() > 128) return false;
    for (TCHAR C : Id)
        if (!((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') || (C >= '0' && C <= '9') || C == '.' || C == '-' || C == '_')) return false;
    return true;
}
bool Object(const TSharedPtr<FJsonValue>& Value, TSharedPtr<FJsonObject>& Json)
{
    if (!Value.IsValid() || Value->Type != EJson::Object) return false;
    Json = Value->AsObject(); return Json.IsValid();
}
bool NullField(const TSharedPtr<FJsonObject>& Json, const TCHAR* Key)
{
    const auto Field = Json->TryGetField(Key);
    return !Field.IsValid() || Field->IsNull();
}
bool ReadBody(const TSharedPtr<FJsonObject>& Json, FSolarBodyDefinition& Body)
{
    if (!Json->TryGetStringField(TEXT("id"), Body.Id) || !ValidId(Body.Id)
        || !Json->TryGetStringField(TEXT("name"), Body.Name) || Body.Name.TrimStartAndEnd().IsEmpty() || Body.Name.Len() > 1024
        || !Json->TryGetStringField(TEXT("parentId"), Body.ParentId) || (!Body.ParentId.IsEmpty() && !ValidId(Body.ParentId))
        || !Json->TryGetStringField(TEXT("kind"), Body.Kind)) return false;
    Body.bKnownRadius = !NullField(Json, TEXT("radiusMetres"));
    Body.bKnownPosition = !NullField(Json, TEXT("positionMetres"));
    if (Json->HasField(TEXT("radiusKnown")) && !Json->TryGetBoolField(TEXT("radiusKnown"), Body.bKnownRadius)) return false;
    if (Json->HasField(TEXT("positionKnown")) && !Json->TryGetBoolField(TEXT("positionKnown"), Body.bKnownPosition)) return false;
    if (Body.bKnownRadius) { if (!Number(Json, TEXT("radiusMetres"), Body.Radius, .01, 1e12)) return false; }
    else if (!NullField(Json, TEXT("radiusMetres")) && !Number(Json, TEXT("radiusMetres"), Body.Radius, 0, 0)) return false;
    if (Body.bKnownPosition) { if (!Vector(Json, TEXT("positionMetres"), Body.Position, LocalLimit)) return false; }
    else if (!NullField(Json, TEXT("positionMetres"))) return false;
    if (!OptionalNumber(Json, TEXT("rotationHours"), Body.RotationHours, -1e9, 1e9)
        || !OptionalNumber(Json, TEXT("tiltDegrees"), Body.Tilt, -360, 360)
        || (Json->HasField(TEXT("shapeScale")) && !Vector(Json, TEXT("shapeScale"), Body.Shape, 100))
        || Body.Shape.GetMin() <= 0 || Body.Radius * Body.Shape.GetMax() > 1e12
        || !OptionalNumber(Json, TEXT("ringInnerMetres"), Body.RingInner, 0, 1e15)
        || !OptionalNumber(Json, TEXT("ringOuterMetres"), Body.RingOuter, 0, 1e15)
        || (Body.RingOuter > 0 && Body.RingOuter <= Body.RingInner)) return false;
    Body.bAtmosphere = !NullField(Json, TEXT("atmosphereColor"));
    Json->TryGetStringField(TEXT("material"), Body.MaterialPath);
    Json->TryGetStringField(TEXT("mapMaterial"), Body.MapMaterialPath);
    Json->TryGetStringField(TEXT("meshAsset"), Body.MeshPath);
    Json->TryGetStringField(TEXT("surfaceQuality"), Body.SourceQuality);
    if (Body.SourceQuality.IsEmpty()) Body.SourceQuality = TEXT("source-quality-unspecified");
    if (Body.MaterialPath.IsEmpty()) Body.MaterialPath = TEXT("/Game/Solar/Materials/M_FallbackRock");
    if (Body.MapMaterialPath.IsEmpty()) Body.MapMaterialPath = TEXT("/Game/Solar/Materials/M_MapRock");
    return Body.MaterialPath.StartsWith(TEXT("/Game/")) && Body.MapMaterialPath.StartsWith(TEXT("/Game/"))
        && (Body.MeshPath.IsEmpty() || Body.MeshPath.StartsWith(TEXT("/Game/")));
}
FString RelatedMaterial(const FSolarBodyDefinition& Body, const TCHAR* Prefix)
{
    return Body.MaterialPath.Replace(TEXT("M_Body_"), Prefix, ESearchCase::CaseSensitive);
}
UMaterialInterface* Material(const FString& Path) { return LoadObject<UMaterialInterface>(nullptr, *Path); }
bool SceneMaterialsAvailable(const FSolarSystemDefinition& System)
{
    if (!LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Solar/SM_SolarRock.SM_SolarRock"))
        || !Material(TEXT("/Game/Solar/Materials/M_Sky")) || !Material(TEXT("/Game/Solar/Materials/M_Rock"))
        || !Material(TEXT("/Game/Solar/Materials/M_UnknownMarker"))) return false;
    for (const auto& Body : System.Bodies)
    {
        if (!Body.bKnownPosition || !Body.bKnownRadius) continue;
        if (!Material(Body.MaterialPath)
            || (!Body.MeshPath.IsEmpty() && !LoadObject<UStaticMesh>(nullptr, *Body.MeshPath))
            || (Body.bAtmosphere && !Material(RelatedMaterial(Body, TEXT("M_Air_"))))
            || (Body.RingOuter > 0 && !Material(RelatedMaterial(Body, TEXT("M_Ring_"))))) return false;
    }
    return true;
}
TArray<Daedalus::FFlightBody> CollisionBodies(const FSolarSystemDefinition& System)
{
    TArray<Daedalus::FFlightBody> Result;
    for (const auto& Body : System.Bodies)
        if (Body.bKnownRadius && Body.bKnownPosition) Result.Add({Body.Id, Body.Position, Body.Radius * Body.Shape.GetMax()});
    return Result;
}
Daedalus::FFlightState InspectionState(const FSolarSystemDefinition& System, const FSolarBodyDefinition& Body,
    const Daedalus::FFlightConfig& Config, double Clock)
{
    FVector3d StarPosition = FVector3d::ZeroVector;
    for (const auto& Entry : System.Bodies) if (Entry.Id == System.PrimaryStarId) { StarPosition = Entry.Position; break; }
    FVector3d Direction = (StarPosition - Body.Position).GetSafeNormal();
    if (Body.Id == System.PrimaryStarId || Direction.IsNearlyZero()) Direction = FVector3d(-1, 1, .3).GetSafeNormal();
    Direction = (Direction + FVector3d(0, 0, .6)).GetSafeNormal();
    const double StandOff = Body.bKnownRadius ? Body.Radius * Body.Shape.GetMax() * 2.6 + 5000 : 10000;
    Daedalus::FFlightState Result; Result.PositionMetres = Body.Position + Direction * StandOff;
    const auto Look = (-Direction).Rotation();
    Result.YawDegrees = Look.Yaw; Result.PitchDegrees = FMath::Clamp(double(Look.Pitch), -Config.PitchLimitDegrees, Config.PitchLimitDegrees);
    Result.SimulationSeconds = Clock; return Result;
}
}

bool ASolarFlightGameMode::LoadSystem()
{
    TSharedPtr<FJsonObject> Universe; const TArray<TSharedPtr<FJsonValue>>* Rows = nullptr; double Version = 0;
    if (!ReadJson(TEXT("Data/Solar/universe.json"), Universe) || !Number(Universe, TEXT("version"), Version, 1, 1)
        || !Universe->TryGetArrayField(TEXT("systems"), Rows) || Rows->IsEmpty() || Rows->Num() > 10000) return false;
    TArray<FSolarSystemDefinition> NewSystems; TSet<FString> SystemIds; int32 TotalBodies = 0;
    for (const auto& Row : *Rows)
    {
        TSharedPtr<FJsonObject> Json; FSolarSystemDefinition System; FString CatalogPath, Color;
        if (!Object(Row, Json) || !Json->TryGetStringField(TEXT("id"), System.Id) || !ValidId(System.Id) || SystemIds.Contains(System.Id)
            || !Json->TryGetStringField(TEXT("name"), System.Name) || System.Name.TrimStartAndEnd().IsEmpty()
            || !Json->TryGetStringField(TEXT("primaryStarId"), System.PrimaryStarId) || !ValidId(System.PrimaryStarId)
            || !Vector(Json, TEXT("galaxyPositionLightYears"), System.GalaxyLightYears, 1e9)
            || !Json->TryGetBoolField(TEXT("available"), System.bAvailable)) return false;
        if (Json->TryGetStringField(TEXT("stellarColor"), Color)) System.StellarColor = FLinearColor(FColor::FromHex(Color));
        SystemIds.Add(System.Id);
        if (!System.bAvailable)
        {
            FSolarBodyDefinition Pending;
            Pending.Id = System.PrimaryStarId; Pending.Name = TEXT("Hvězda — obsah se připravuje"); Pending.Kind = TEXT("star");
            Pending.bKnownRadius = false; Pending.SourceQuality = TEXT("pending-source-assets; system-unavailable");
            System.Bodies.Add(Pending); NewSystems.Add(MoveTemp(System)); ++TotalBodies;
            if (TotalBodies > 100000) return false;
            continue;
        }
        TSharedPtr<FJsonObject> Catalog; const TArray<TSharedPtr<FJsonValue>>* BodyRows = nullptr; FString CatalogId;
        if (!Json->TryGetStringField(TEXT("catalog"), CatalogPath) || !ReadJson(CatalogPath, Catalog)
            || !Number(Catalog, TEXT("version"), Version, 1, 1) || !Catalog->TryGetStringField(TEXT("id"), CatalogId) || CatalogId != System.Id
            || !Catalog->TryGetArrayField(TEXT("bodies"), BodyRows) || BodyRows->IsEmpty() || BodyRows->Num() > 10000) return false;
        TotalBodies += BodyRows->Num(); if (TotalBodies > 100000) return false;
        TMap<FString, int32> BodyIndices;
        for (const auto& BodyRow : *BodyRows)
        {
            TSharedPtr<FJsonObject> BodyJson; FSolarBodyDefinition Body;
            if (!Object(BodyRow, BodyJson) || !ReadBody(BodyJson, Body) || BodyIndices.Contains(Body.Id)) return false;
            BodyIndices.Add(Body.Id, System.Bodies.Num()); System.Bodies.Add(MoveTemp(Body));
        }
        const int32* Star = BodyIndices.Find(System.PrimaryStarId);
        if (!Star || System.Bodies[*Star].Kind != TEXT("star") || !System.Bodies[*Star].bKnownPosition || !System.Bodies[*Star].bKnownRadius) return false;
        // Include unlocated database rows in parent validation; omit them only from navigation.
        TArray<uint8> Visited; Visited.SetNumZeroed(System.Bodies.Num());
        for (int32 Root = 0; Root < System.Bodies.Num(); ++Root)
        {
            TArray<int32> Path; int32 Cursor = Root;
            while (Cursor != INDEX_NONE && Visited[Cursor] != 2)
            {
                if (Visited[Cursor] == 1) return false;
                Visited[Cursor] = 1; Path.Add(Cursor);
                const FString& Parent = System.Bodies[Cursor].ParentId;
                if (Parent.IsEmpty()) Cursor = INDEX_NONE;
                else { const int32* ParentIndex = BodyIndices.Find(Parent); if (!ParentIndex) return false; Cursor = *ParentIndex; }
            }
            for (int32 Index : Path) Visited[Index] = 2;
        }
        const TArray<TSharedPtr<FJsonValue>>* BeltRows = nullptr; int32 Samples = 0;
        if (Catalog->TryGetArrayField(TEXT("belts"), BeltRows))
        {
            for (const auto& BeltRow : *BeltRows)
            {
                TSharedPtr<FJsonObject> BeltJson; FSolarBeltDefinition BeltDefinition; double Count = 0, Seed = 0;
                if (!Object(BeltRow, BeltJson) || !BeltJson->TryGetStringField(TEXT("parentId"), BeltDefinition.ParentId)
                    || !BodyIndices.Contains(BeltDefinition.ParentId)
                    || !Number(BeltJson, TEXT("innerMetres"), BeltDefinition.Inner, 0, DecorativeLimit)
                    || !Number(BeltJson, TEXT("outerMetres"), BeltDefinition.Outer, 0, DecorativeLimit) || BeltDefinition.Outer <= BeltDefinition.Inner
                    || !Number(BeltJson, TEXT("count"), Count, 0, 10000) || Count != FMath::FloorToDouble(Count)
                    || !Number(BeltJson, TEXT("seed"), Seed, 0, 4294967295.) || Seed != FMath::FloorToDouble(Seed)
                    || !OptionalNumber(BeltJson, TEXT("longitudeOffsetDegrees"), BeltDefinition.LongitudeOffset, -360, 360)) return false;
                BeltJson->TryGetStringField(TEXT("geometry"), BeltDefinition.Geometry);
                if (BeltDefinition.Geometry.IsEmpty()) BeltDefinition.Geometry = TEXT("disk");
                if (BeltDefinition.Geometry != TEXT("disk") && BeltDefinition.Geometry != TEXT("sphere") && BeltDefinition.Geometry != TEXT("trojan")) return false;
                BeltDefinition.Count = int32(Count); BeltDefinition.Seed = int32(uint32(Seed));
                Samples += BeltDefinition.Count; if (Samples > 100000) return false;
                System.Belts.Add(BeltDefinition);
            }
        }
        const TArray<TSharedPtr<FJsonValue>>* RegionRows = nullptr;
        if (Catalog->TryGetArrayField(TEXT("regions"), RegionRows))
        {
            if (RegionRows->Num() > 10000) return false;
            for (const auto& RegionRow : *RegionRows)
            {
                TSharedPtr<FJsonObject> RegionJson; FGalaxyRegionView Region;
                if (!Object(RegionRow, RegionJson) || !RegionJson->TryGetStringField(TEXT("name"), Region.Name)
                    || !RegionJson->TryGetStringField(TEXT("geometry"), Region.Geometry)
                    || !Vector(RegionJson, TEXT("centreMetres"), Region.CentreMetres, DecorativeLimit)
                    || !Number(RegionJson, TEXT("innerMetres"), Region.InnerMetres, 0, DecorativeLimit)
                    || !Number(RegionJson, TEXT("outerMetres"), Region.OuterMetres, 0, DecorativeLimit) || Region.OuterMetres <= Region.InnerMetres) return false;
                System.Regions.Add(Region);
            }
        }
        NewSystems.Add(MoveTemp(System));
    }
    if (NewSystems.IsEmpty() || !NewSystems[0].bAvailable) return false;
    TArray<Daedalus::FNavigationSystem> NavigationCatalog; TArray<FGalaxySystemView> NewMapSystems;
    TArray<TObjectPtr<UMaterialInterface>> NewMapMaterials;
    for (const auto& System : NewSystems)
    {
        Daedalus::FNavigationSystem NavSystem;
        NavSystem.Id = System.Id; NavSystem.Name = System.Name; NavSystem.GalaxyPositionLightYears = System.GalaxyLightYears;
        TSet<FString> LocatedIds; for (const auto& Body : System.Bodies) if (Body.bKnownPosition) LocatedIds.Add(Body.Id);
        FGalaxySystemView MapSystem;
        MapSystem.Id = System.Id; MapSystem.Name = System.Name; MapSystem.GalaxyLightYears = System.GalaxyLightYears;
        MapSystem.bAvailable = System.bAvailable; MapSystem.Regions = System.Regions;
        for (const auto& Body : System.Bodies)
        {
            if (Body.bKnownPosition)
            {
                Daedalus::FNavigationBody NavBody;
                NavBody.Id = Body.Id; NavBody.Name = Body.Name; NavBody.PositionMetres = Body.Position;
                NavBody.ParentId = LocatedIds.Contains(Body.ParentId) ? Body.ParentId : FString();
                NavBody.RadiusMetres = Body.Radius; NavBody.bKnownRadius = Body.bKnownRadius; NavSystem.Bodies.Add(MoveTemp(NavBody));
            }
            FGalaxyBodyView MapBody;
            MapBody.Id = Body.Id; MapBody.Name = Body.Name; MapBody.ParentId = Body.ParentId; MapBody.Kind = Body.Kind;
            MapBody.SourceQuality = Body.SourceQuality; MapBody.PositionMetres = Body.Position; MapBody.RadiusMetres = Body.Radius;
            MapBody.bKnownRadius = Body.bKnownRadius; MapBody.bKnownPosition = Body.bKnownPosition;
            if (System.bAvailable)
            {
                MapBody.MapMaterial = Material(Body.MapMaterialPath); if (!MapBody.MapMaterial) return false;
                NewMapMaterials.AddUnique(MapBody.MapMaterial);
            }
            MapSystem.Bodies.Add(MoveTemp(MapBody));
        }
        NavigationCatalog.Add(MoveTemp(NavSystem)); NewMapSystems.Add(MoveTemp(MapSystem));
    }
    Daedalus::FNavigationPlan NewNavigation, NewPreview;
    if (!NewNavigation.Configure(NavigationCatalog, Message) || !NewPreview.Configure(NavigationCatalog, Message)) return false;
    Systems = MoveTemp(NewSystems); MapSystems = MoveTemp(NewMapSystems); MapMaterials = MoveTemp(NewMapMaterials);
    Navigation = MoveTemp(NewNavigation); PreviewNavigation = MoveTemp(NewPreview);
    ConfigureActiveBodies(0); return true;
}
void ASolarFlightGameMode::ConfigureActiveBodies(int32 Index)
{
    ActiveSystem = Index; BodyDefinitions = Systems[Index].Bodies; Bodies = CollisionBodies(Systems[Index]);
    SunPosition = FVector3d::ZeroVector; SunRadius = 0; EarthRadius = 0; SelectedBody = 0;
    for (int32 Body = 0; Body < BodyDefinitions.Num(); ++Body)
    {
        const auto& D = BodyDefinitions[Body];
        if (D.Id == Systems[Index].PrimaryStarId) { SunPosition = D.Position; SunRadius = D.Radius; SelectedBody = Body; }
        if (D.Id == TEXT("sol.earth")) { EarthRadius = D.Radius; SelectedBody = Body; }
    }
}
bool ASolarFlightGameMode::CreateSystem()
{
    if (!Systems.IsValidIndex(ActiveSystem) || !SceneMaterialsAvailable(Systems[ActiveSystem])) return false;
    auto* Rock = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Solar/SM_SolarRock.SM_SolarRock"));
    if (!Sky) { Sky = MakeMesh(SphereAsset, Material(TEXT("/Game/Solar/Materials/M_Sky"))); Sky->SetWorldScale3D(FVector(2.5e7)); }
    BodyMeshes.SetNumZeroed(BodyDefinitions.Num()); BodyDynamics.SetNumZeroed(BodyDefinitions.Num());
    AirMeshes.SetNumZeroed(BodyDefinitions.Num()); AirDynamics.SetNumZeroed(BodyDefinitions.Num());
    for (int32 Index = 0; Index < BodyDefinitions.Num(); ++Index)
    {
        const auto& Body = BodyDefinitions[Index]; if (!Body.bKnownPosition || !Body.bKnownRadius) continue;
        auto* Dynamic = UMaterialInstanceDynamic::Create(Material(Body.MaterialPath), this); BodyDynamics[Index] = Dynamic;
        const bool Irregular = Body.Kind == TEXT("asteroid") || Body.Kind == TEXT("minor-body") || Body.Kind == TEXT("comet");
        UStaticMesh* Mesh = !Body.MeshPath.IsEmpty() ? LoadObject<UStaticMesh>(nullptr, *Body.MeshPath) : Irregular ? Rock : SphereAsset.Get();
        BodyMeshes[Index] = MakeMesh(Mesh, Dynamic);
        if (Body.bAtmosphere)
        {
            AirDynamics[Index] = UMaterialInstanceDynamic::Create(Material(RelatedMaterial(Body, TEXT("M_Air_"))), this);
            AirMeshes[Index] = MakeMesh(SphereAsset, AirDynamics[Index]);
        }
        if (Body.RingOuter > 0) { RingBodies.Add(Index); RingMeshes.Add(MakeMesh(PlaneAsset, Material(RelatedMaterial(Body, TEXT("M_Ring_"))))); }
        if (Body.Id == TEXT("sol.earth")) { Earth = BodyMeshes[Index]; EarthDynamic = Dynamic; Atmosphere = AirMeshes[Index]; AtmosphereDynamic = AirDynamics[Index]; }
        if (Body.Id == Systems[ActiveSystem].PrimaryStarId) Sun = BodyMeshes[Index];
    }
    UnknownMarker = MakeMesh(PlaneAsset, Material(TEXT("/Game/Solar/Materials/M_UnknownMarker"))); UnknownMarker->SetVisibility(false);
    BeltPositions.Reset(); BeltRadii.Reset();
    for (const auto& Definition : Systems[ActiveSystem].Belts)
    {
        FVector3d Centre = SunPosition;
        for (const auto& Body : BodyDefinitions) if (Body.Id == Definition.ParentId) { Centre = Body.Position; break; }
        double ReferenceAngle = 0;
        if (Definition.Geometry == TEXT("trojan"))
            for (const auto& Body : BodyDefinitions) if (Body.Id == TEXT("sol.jupiter")) { const auto Relative = Body.Position - Centre; ReferenceAngle = FMath::Atan2(Relative.Y, Relative.X); break; }
        FRandomStream Random(Definition.Seed);
        for (int32 Index = 0; Index < Definition.Count; ++Index)
        {
            const double Radius = Definition.Inner + (Definition.Outer - Definition.Inner) * Random.FRand();
            const double Angle = Definition.Geometry == TEXT("trojan")
                ? ReferenceAngle + FMath::DegreesToRadians(Definition.LongitudeOffset + Random.FRandRange(-12, 12)) : Random.FRand() * 2 * PI;
            FVector3d Position;
            if (Definition.Geometry == TEXT("sphere"))
            {
                const double Z = Random.FRandRange(-1, 1), XY = FMath::Sqrt(FMath::Max(0., 1 - Z * Z));
                Position = FVector3d(FMath::Cos(Angle) * XY, FMath::Sin(Angle) * XY, Z) * Radius;
            }
            else Position = FVector3d(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, Random.FRandRange(-1, 1) * Radius * .01);
            BeltPositions.Add(Centre + Position); BeltRadii.Add(Random.FRandRange(200, 2000));
        }
    }
    Belt = NewObject<UInstancedStaticMeshComponent>(this); AddInstanceComponent(Belt); Belt->RegisterComponent();
    Belt->SetStaticMesh(Rock); Belt->SetMaterial(0, Material(TEXT("/Game/Solar/Materials/M_Rock")));
    Belt->SetCollisionEnabled(ECollisionEnabled::NoCollision); Belt->SetCastShadow(false);
    for (int32 Index = 0; Index < BeltPositions.Num(); ++Index) Belt->AddInstance(FTransform::Identity);
    return true;
}
void ASolarFlightGameMode::ClearSystem()
{
    auto DestroyMesh = [&](UStaticMeshComponent* Mesh)
    {
        if (!Mesh) return;
        AActor* Owner = Mesh->GetOwner();
        if (Owner && Owner != this) Owner->Destroy(); else Mesh->DestroyComponent();
    };
    for (const auto& Mesh : BodyMeshes) DestroyMesh(Mesh);
    for (const auto& Mesh : AirMeshes) DestroyMesh(Mesh);
    for (const auto& Mesh : RingMeshes) DestroyMesh(Mesh);
    DestroyMesh(UnknownMarker); UnknownMarker = nullptr;
    if (Belt) { Belt->DestroyComponent(); Belt = nullptr; }
    BodyMeshes.Reset(); BodyDynamics.Reset(); AirMeshes.Reset(); AirDynamics.Reset(); RingMeshes.Reset(); RingBodies.Reset();
    BeltPositions.Reset(); BeltRadii.Reset(); Earth = nullptr; Sun = nullptr; Atmosphere = nullptr; EarthDynamic = nullptr; AtmosphereDynamic = nullptr;
}
void ASolarFlightGameMode::UpdateSystem(const FVector& CameraPosition)
{
    const auto& State = Flight.GetState();
    auto Project = [&](const FVector3d& Position)
    {
        const FVector3d Relative = Position - State.PositionMetres - FVector3d(CameraPosition) / 100;
        const double Factor = FMath::Min(1., 3e6 / FMath::Max(Relative.Size(), 1.));
        return TPair<FVector, double>(CameraPosition + FVector(Relative * (100 * Factor)), Factor);
    };
    for (int32 Index = 0; Index < BodyDefinitions.Num(); ++Index)
    {
        if (!BodyMeshes[Index]) continue;
        const auto& Body = BodyDefinitions[Index]; const auto Projection = Project(Body.Position);
        const double Angle = Body.RotationHours == 0 ? 0 : FMath::Fmod(State.SimulationSeconds * 360 / (Body.RotationHours * 3600), 360.);
        const FQuat Rotation = FQuat(FVector::ForwardVector, FMath::DegreesToRadians(Body.Tilt)) * FQuat(FVector::UpVector, FMath::DegreesToRadians(Angle));
        BodyMeshes[Index]->SetWorldLocationAndRotation(Projection.Key, Rotation);
        BodyMeshes[Index]->SetWorldScale3D(FVector(Body.Shape * Body.Radius * Projection.Value));
        const FVector3d Direction = (SunPosition - Body.Position).GetSafeNormal();
        const FLinearColor LightDirection(Direction.X, Direction.Y, Direction.Z, 0);
        BodyDynamics[Index]->SetVectorParameterValue(TEXT("SunDirection"), LightDirection);
        if (AirMeshes[Index])
        {
            AirMeshes[Index]->SetWorldLocationAndRotation(Projection.Key, Rotation);
            AirMeshes[Index]->SetWorldScale3D(FVector(Body.Shape * Body.Radius * Projection.Value * 1.012));
            AirDynamics[Index]->SetVectorParameterValue(TEXT("SunDirection"), LightDirection);
        }
    }
    for (int32 Index = 0; Index < RingMeshes.Num(); ++Index)
    {
        const int32 BodyIndex = RingBodies[Index]; const auto& Body = BodyDefinitions[BodyIndex]; const auto Projection = Project(Body.Position);
        RingMeshes[Index]->SetWorldLocationAndRotation(Projection.Key, BodyMeshes[BodyIndex]->GetComponentQuat());
        RingMeshes[Index]->SetWorldScale3D(FVector(Body.RingOuter * Projection.Value * 2, Body.RingOuter * Projection.Value * 2, 1));
    }
    if (UnknownMarker)
    {
        const bool Visible = BodyDefinitions.IsValidIndex(SelectedBody) && BodyDefinitions[SelectedBody].bKnownPosition && !BodyDefinitions[SelectedBody].bKnownRadius;
        UnknownMarker->SetVisibility(Visible);
        if (Visible)
        {
            const auto Projection = Project(BodyDefinitions[SelectedBody].Position);
            UnknownMarker->SetWorldLocationAndRotation(Projection.Key, FRotationMatrix::MakeFromZ(CameraPosition - Projection.Key).ToQuat());
            // Constant angular symbol, not a physical surface or invented radius.
            const double WidthCentimetres = FMath::Max(100., (Projection.Key - CameraPosition).Size() * .008);
            UnknownMarker->SetWorldScale3D(FVector(WidthCentimetres / 100));
        }
    }
    if (Belt) for (int32 Index = 0; Index < BeltPositions.Num(); ++Index)
    {
        const auto Projection = Project(BeltPositions[Index]);
        Belt->UpdateInstanceTransform(Index, FTransform(FQuat(FVector::UpVector, Index * .73), Projection.Key, FVector(BeltRadii[Index] * Projection.Value)), false, Index == BeltPositions.Num() - 1);
    }
    if (Sky) Sky->SetWorldLocation(CameraPosition);
    if (SolarLight)
    {
        SolarLight->SetActorRotation((-FVector(SunPosition - State.PositionMetres).GetSafeNormal()).Rotation());
        SolarLight->GetLightComponent()->SetLightColor(Systems[ActiveSystem].StellarColor);
    }
}
void ASolarFlightGameMode::SelectBody(int32 Step)
{
    if (!bReady || BodyDefinitions.IsEmpty() || Galaxy.bOpen) return;
    SelectedBody = (SelectedBody + Step % BodyDefinitions.Num() + BodyDefinitions.Num()) % BodyDefinitions.Num(); Message.Reset();
    const auto& Body = BodyDefinitions[SelectedBody];
    if (Body.bKnownPosition) Navigation.SelectTarget(Systems[ActiveSystem].Id, Body.Id, Message);
    else Message = TEXT("Poloha není známá. Těleso je pouze v databázi, bez vymyšleného umístění.");
}
bool ASolarFlightGameMode::ActivateSystem(int32 Index, int32 BodyIndex)
{
    if (!bReady || !Systems.IsValidIndex(Index) || !Systems[Index].bAvailable || !Systems[Index].Bodies.IsValidIndex(BodyIndex))
    { Message = TEXT("Tato soustava ještě nemá dostupný herní obsah."); return false; }
    const auto& System = Systems[Index]; const auto& Body = System.Bodies[BodyIndex];
    if (!Body.bKnownPosition) { Message = TEXT("Poloha není známá; testovací přesun není dostupný."); return false; }
    if (!Profiles.IsValidIndex(ProfileIndex) || !SceneMaterialsAvailable(System))
    { Message = TEXT("Testovací přesun nelze připravit: chybí asset nebo letový profil."); return false; }
    const auto Start = InspectionState(System, Body, Profiles[ProfileIndex].Config, Flight.GetState().SimulationSeconds);
    Daedalus::FFlightModel Candidate; FString Error;
    if (!Candidate.Initialize(Profiles[ProfileIndex].Config, Start, CollisionBodies(System), Error)) { Message = Error; return false; }
    Candidate.SetPaused(Galaxy.bOpen);
    if (Index != ActiveSystem)
    {
        ClearSystem(); ConfigureActiveBodies(Index);
        // All dependencies and the candidate domain pose passed before scene destruction.
        if (!CreateSystem()) { bReady = false; Message = TEXT("Vytvoření ověřené soustavy selhalo."); return false; }
    }
    Flight = MoveTemp(Candidate); SelectedBody = BodyIndex; bPaused = false;
    Navigation.SelectTarget(System.Id, Body.Id, Error);
    Message = Body.bKnownRadius ? TEXT("Testovací přesun k tělesu — nejde o cestování pohonem.")
        : TEXT("Testovací přesun ke značce: velikost neznámá, fyzický model ani kolize nejsou vytvořené.");
    if (auto* PC = GetWorld()->GetFirstPlayerController()) if (auto* Pawn = Cast<ASolarFlightPawn>(PC->GetPawn())) Pawn->ResetCamera();
    return true;
}
void ASolarFlightGameMode::InspectSelectedBody() { ActivateSystem(ActiveSystem, SelectedBody); }


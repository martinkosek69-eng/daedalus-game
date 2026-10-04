#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/HUD.h"
#include "DaedalusFlightModel.h"
#include "DaedalusShipFlightCatalog.h"
#include "Solar/GalaxyMapView.h"
#include "Solar/SolarPauseMenu.h"
#include "SolarFlightGameMode.generated.h"

class UCameraComponent;
class UStaticMeshComponent;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UStaticMesh;
class ADirectionalLight;
class UPointLightComponent;

UCLASS()
class DAEDALUS_API ASolarFlightPawn : public APawn
{
    GENERATED_BODY()
public:
    ASolarFlightPawn();
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    void ResetCamera();
    UPROPERTY() TObjectPtr<UCameraComponent> Camera;
    double CameraDistanceMetres = 1500;
private:
    float YawInput = 0, PitchInput = 0;
    bool bBrake = false, bOrbit = false;
    float OrbitYaw = 0, OrbitPitch = 0;
    bool bCameraInitialized = false;
    bool bFollowShip = true;
    float FollowYaw = 0;
    float FollowPitch = 0;
    void Yaw(float V) { YawInput = V; }
    void Pitch(float V) { PitchInput = V; }
    void MouseX(float V); void MouseY(float V);
    void OrbitOn(); void OrbitOff() { bOrbit = false; }
    void BrakeOn(); void BrakeOff() { bBrake = false; }
    void MoreThrottle(); void LessThrottle(); void ToggleThrottle();
    void ZoomIn(); void ZoomOut();
    void Slow(); void Fast(); void FullImpulse(); void ResetFlight(); void PauseFlight(); void ExitGame();
    void NextBody(); void PreviousBody(); void InspectBody();
    void ToggleMap();
    void ToggleHangars();
};

UCLASS()
class DAEDALUS_API ASolarFlightHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};

struct FSolarFlightProfile
{
    FString Name;
    Daedalus::FFlightConfig Config;
};

// Immutable canonical metres; projected meshes below are presentation only.
struct FSolarBodyDefinition
{
    FString Id, Name, ParentId, Kind, MaterialPath, MapMaterialPath, SourceQuality, MeshPath;
    FVector3d Position = FVector3d::ZeroVector, Shape = FVector3d(1);
    double Radius = 0, RotationHours = 0, Tilt = 0;
    double RingInner = 0, RingOuter = 0;
    bool bAtmosphere = false;
    bool bKnownRadius = true, bKnownPosition = true;
};

struct FSolarBeltDefinition
{
    FString ParentId, Geometry;
    double Inner = 0, Outer = 0, LongitudeOffset = 0;
    int32 Count = 0, Seed = 0;
};

struct FSolarSystemDefinition
{
    FString Id, Name, PrimaryStarId;
    FVector3d GalaxyLightYears = FVector3d::ZeroVector;
    FLinearColor StellarColor = FLinearColor::White;
    bool bAvailable = false;
    TArray<FSolarBodyDefinition> Bodies;
    TArray<FSolarBeltDefinition> Belts;
    TArray<FGalaxyRegionView> Regions;
};

UCLASS()
class DAEDALUS_API ASolarFlightGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ASolarFlightGameMode();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;
    Daedalus::FFlightModel Flight;
    bool bReady = false, bPaused = false;
    FString Message;
    TArray<FSolarFlightProfile> Profiles;
    int32 ProfileIndex = 1;
    double EarthRadius = 6371000, SunRadius = 695700000;
    FVector3d SunPosition;
    double FrameMilliseconds = 0;
    void ChangeThrottle(double Step);
    void SetProfile(int32 Index);
    void ToggleFullImpulse();
    void ResetFlight();
    void TogglePause();
    void ToggleHangars();
    bool SelectShip(int32 Index);
    int32 ActiveShip = 0;
    FString ShipId = TEXT("daedalus"), ShipName = TEXT("Daedalus");
    double ShipLengthMetres = 600, CameraDefaultMetres = 1500;
    double CameraMinMetres = 650, CameraMaxMetres = 6000;
    ESolarPausePage PauseMenuPage = ESolarPausePage::Main;
    const TArray<Daedalus::FShipFlightDefinition>& AvailableShips() const { return ShipCatalog.Ships; }
    void HandlePauseMenuClick(FVector2D Pixel, FVector2D Viewport);
    // Presentation preference only; never part of flight state or save schema.
    bool UsesAncientHUD() const;
    bool HasAncientInterface() const;
    bool SetAncientHUD(bool bAncient);
    int32 StarCount() const;
    int32 EngineOutletCount() const { return EngineGlows.Num(); }
    double EngineGlowLevel = .08;
    TArray<FSolarBodyDefinition> BodyDefinitions;
    int32 SelectedBody = 3;
    void SelectBody(int32 Step);
    void InspectSelectedBody();
    int32 RingCount() const { return RingMeshes.Num(); }
    TArray<FSolarSystemDefinition> Systems;
    TArray<FGalaxySystemView> MapSystems;
    int32 ActiveSystem = 0;
    FGalaxyMapView Galaxy;
    Daedalus::FNavigationPlan Navigation, PreviewNavigation;
    Daedalus::FNavigationMetrics PreviewMetrics() const;
    void ToggleMap();
    void HandleMapAction(const FMapAction& Action);
    bool ActivateSystem(int32 Index, int32 BodyIndex);
    void RefreshNavigation();
private:
    Daedalus::FFlightState InitialState;
    TArray<Daedalus::FFlightBody> Bodies;
    UPROPERTY() TObjectPtr<UStaticMesh> ShipAsset;
    UPROPERTY() TObjectPtr<UStaticMesh> AuroraAsset;
    Daedalus::FShipFlightCatalog ShipCatalog;
    bool LoadShipCatalog();
    bool CreateShipView();
    void ClearShipView();
    void UseShipDefinition(int32 Index);
    UPROPERTY() TObjectPtr<UStaticMesh> LightsAsset;
    UPROPERTY() TArray<TObjectPtr<UStaticMesh>> GlowAssets;
    UPROPERTY() TObjectPtr<UStaticMesh> SphereAsset;
    UPROPERTY() TObjectPtr<UStaticMesh> PlaneAsset;
    UPROPERTY() TObjectPtr<UMaterialInterface> EarthMaterial;
    UPROPERTY() TObjectPtr<UMaterialInterface> SunMaterial;
    UPROPERTY() TObjectPtr<UMaterialInterface> AtmosphereMaterial;
    UPROPERTY() TObjectPtr<UMaterialInterface> StarMaterial;
    UPROPERTY() TObjectPtr<UMaterialInterface> DustMaterial;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Ship;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> HullLights;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> EngineGlows;
    UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> EngineDynamics;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Earth;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Sun;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Atmosphere;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> BodyMeshes;
    UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> BodyDynamics;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> AirMeshes;
    UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> AirDynamics;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> RingMeshes;
    TArray<int32> RingBodies;
    // Planet presentation layers (PlanetPresentation.cpp): separate cloud shells and finer spheres
    // chosen by projected pixel radius. Presentation only; canonical body state is untouched.
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> CloudMeshes;
    UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> CloudDynamics;
    UPROPERTY() TArray<TObjectPtr<UStaticMesh>> PlanetSpheres;
    TArray<int32> PlanetSphereLevels;
    TArray<double> PlanetPixelRadii;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> ShipDetails;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> HangarDoors;
    UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> BeaconDynamics;
    UPROPERTY() TArray<TObjectPtr<UPointLightComponent>> ShipPointLights;
    TArray<FVector> DoorOpenOffsets, ShipLightPositions;
    TArray<float> ShipLightCandela;
    TArray<bool> ShipLightIsEngine;
    double DoorTravel = 0, BeaconPeriod = 5, BeaconOnSeconds = 1.0 / 3;
    double BeaconEmissionOn = 8, BeaconEmissionOff = 0, BeaconLevel = 0;
    bool bHangarsOpen = false;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Sky;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> SkyDynamic;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> UnknownMarker;
    UPROPERTY() TArray<TObjectPtr<UMaterialInterface>> MapMaterials;
    UPROPERTY() TObjectPtr<ADirectionalLight> SolarLight;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Belt;
    TArray<FVector3d> BeltPositions;
    TArray<float> BeltRadii;
    TSet<int32> DetailedBodies;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Stars;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Dust;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> EarthDynamic;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> AtmosphereDynamic;
    TArray<FVector3d> DustPositions;
    bool LoadSettings();
    bool LoadHUDSettings();
    TMap<FString, FString> ShipHUDFamilies;
    bool bAncientHUDPreferred = true;
    bool LoadSystem();
    bool CreateSystem();
    void ClearSystem();
    void ConfigureActiveBodies(int32 Index);
    void UpdateSystem(const FVector& CameraPosition);
    bool CreateScene();
    void UpdateScene(float DeltaSeconds);
    bool CreateShipPresentation();
    void UpdateShipPresentation(float DeltaSeconds);
    void ApplySharpRenderingSettings();
    void UpdateTextureDetail();
    UStaticMeshComponent* MakeMesh(UStaticMesh* Asset, UMaterialInterface* Material);
    void PreparePlanetLayers();
    void CreatePlanetLayers(int32 Index);
    void UpdatePlanetLayers(int32 Index, const FVector& Location, const FQuat& Rotation, const FVector& Scale,
        const FLinearColor& LightDirection, const FVector& CameraPosition);
    void ClearPlanetLayers();
    FString ProbeDirectory;
    int32 ProbeFrame = 0;
    bool bProbePassed = true;
    double ProbeYaw = 0, ProbePitch = 0, ProbeClock = 0;
    TArray<double> ProbeFrameTimes;
    void TickProbe();
    void TickGalaxyProbe();
    void TickSharpProbe();
    void TickPlanetProbe();
    void TickIntegrationProbe();
    void TickAuroraProbe();
    void TickAncientHUDProbe();
    bool bSharpProbe = false;
};

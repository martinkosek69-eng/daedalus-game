#include "Solar/SolarFlightGameMode.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/GameUserSettings.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
ASolarFlightGameMode* Lab(const AActor* Actor)
{
    return Actor && Actor->GetWorld() ? Cast<ASolarFlightGameMode>(Actor->GetWorld()->GetAuthGameMode()) : nullptr;
}
bool ReadObject(const FString& Path, TSharedPtr<FJsonObject>& Object)
{
    FString Text;
    return FFileHelper::LoadFileToString(Text, *Path) && FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Object) && Object.IsValid();
}
bool Number(const TSharedPtr<FJsonObject>& Object, const TCHAR* Key, double& Out)
{
    return Object->TryGetNumberField(Key, Out) && FMath::IsFinite(Out);
}
bool Vector(const TSharedPtr<FJsonObject>& Object, const TCHAR* Key, FVector3d& Out)
{
    const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
    if (!Object->TryGetArrayField(Key, Values) || Values->Num() != 3) return false;
    for (int32 I = 0; I < 3; ++I)
        if (!(*Values)[I]->TryGetNumber(Out[I]) || !FMath::IsFinite(Out[I])) return false;
    return true;
}
}

ASolarFlightPawn::ASolarFlightPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FlightCamera"));
    Camera->SetupAttachment(RootComponent);
    Camera->SetFieldOfView(52);
    Camera->PostProcessSettings.bOverride_BloomIntensity = true;
    Camera->PostProcessSettings.BloomIntensity = .12f;
    Camera->PostProcessSettings.bOverride_MotionBlurAmount = true;
    Camera->PostProcessSettings.MotionBlurAmount = 0;
    Camera->PostProcessSettings.bOverride_DepthOfFieldScale = true;
    Camera->PostProcessSettings.DepthOfFieldScale = 0;
    Camera->PostProcessSettings.bOverride_SceneFringeIntensity = true;
    Camera->PostProcessSettings.SceneFringeIntensity = 0;
    Camera->PostProcessSettings.bOverride_FilmGrainIntensity = true;
    Camera->PostProcessSettings.FilmGrainIntensity = 0;
    Camera->PostProcessSettings.bOverride_AmbientOcclusionIntensity = true;
    Camera->PostProcessSettings.AmbientOcclusionIntensity = .8f;
    AutoPossessPlayer = EAutoReceiveInput::Player0;
}
void ASolarFlightPawn::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    Input->BindAxis(TEXT("SolarYaw"), this, &ASolarFlightPawn::Yaw);
    Input->BindAxis(TEXT("SolarPitch"), this, &ASolarFlightPawn::Pitch);
    Input->BindAxis(TEXT("SolarMouseX"), this, &ASolarFlightPawn::MouseX);
    Input->BindAxis(TEXT("SolarMouseY"), this, &ASolarFlightPawn::MouseY);
    Input->BindKey(EKeys::E, IE_Pressed, this, &ASolarFlightPawn::MoreThrottle);
    Input->BindKey(EKeys::Q, IE_Pressed, this, &ASolarFlightPawn::LessThrottle);
    Input->BindKey(EKeys::R, IE_Pressed, this, &ASolarFlightPawn::ToggleThrottle);
    Input->BindKey(FInputChord(EKeys::R, true, false, false, false), IE_Pressed, this, &ASolarFlightPawn::FullImpulse);
    Input->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ASolarFlightPawn::BrakeOn);
    Input->BindKey(EKeys::SpaceBar, IE_Released, this, &ASolarFlightPawn::BrakeOff);
    Input->BindKey(EKeys::X, IE_Pressed, this, &ASolarFlightPawn::BrakeOn);
    Input->BindKey(EKeys::X, IE_Released, this, &ASolarFlightPawn::BrakeOff);
    Input->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &ASolarFlightPawn::OrbitOn);
    Input->BindKey(EKeys::RightMouseButton, IE_Released, this, &ASolarFlightPawn::OrbitOff);
    Input->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &ASolarFlightPawn::ZoomIn);
    Input->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &ASolarFlightPawn::ZoomOut);
    Input->BindKey(EKeys::Home, IE_Pressed, this, &ASolarFlightPawn::ResetCamera);
    Input->BindKey(EKeys::One, IE_Pressed, this, &ASolarFlightPawn::Slow);
    Input->BindKey(EKeys::Two, IE_Pressed, this, &ASolarFlightPawn::Fast);
    Input->BindKey(EKeys::BackSpace, IE_Pressed, this, &ASolarFlightPawn::ResetFlight);
    Input->BindKey(EKeys::P, IE_Pressed, this, &ASolarFlightPawn::PauseFlight);
    Input->BindKey(EKeys::PageDown,IE_Pressed,this,&ASolarFlightPawn::NextBody);
    Input->BindKey(EKeys::PageUp,IE_Pressed,this,&ASolarFlightPawn::PreviousBody);
    Input->BindKey(EKeys::F,IE_Pressed,this,&ASolarFlightPawn::InspectBody);
    Input->BindKey(EKeys::Escape, IE_Pressed, this, &ASolarFlightPawn::ExitGame);
    Input->BindKey(EKeys::M, IE_Pressed, this, &ASolarFlightPawn::ToggleMap);
}
void ASolarFlightPawn::ToggleMap(){if(auto* M=Lab(this);M && !M->Galaxy.bSearchFocused) M->ToggleMap();}
void ASolarFlightPawn::NextBody(){if(auto* M=Lab(this);M && !M->Galaxy.bOpen)M->SelectBody(1);}
void ASolarFlightPawn::PreviousBody(){if(auto* M=Lab(this);M && !M->Galaxy.bOpen)M->SelectBody(-1);}
void ASolarFlightPawn::InspectBody(){if(auto* M=Lab(this);M && !M->Galaxy.bOpen)M->InspectSelectedBody();}
void ASolarFlightPawn::OrbitOn(){if(auto* M=Lab(this);M && !M->Galaxy.bOpen){bOrbit=true;bFollowShip=false;}}
void ASolarFlightPawn::MouseX(float V) { if (bOrbit && Lab(this) && !Lab(this)->Galaxy.bOpen) OrbitYaw = FRotator::NormalizeAxis(OrbitYaw + V * .344f); }
void ASolarFlightPawn::MouseY(float V) { if (bOrbit && Lab(this) && !Lab(this)->Galaxy.bOpen) OrbitPitch = FMath::Clamp(OrbitPitch + V * .344f, -65.0f, 85.0f); }
void ASolarFlightPawn::MoreThrottle() { if (auto* M = Lab(this)) M->ChangeThrottle(.2); }
void ASolarFlightPawn::LessThrottle() { if (auto* M = Lab(this)) M->ChangeThrottle(-.2); }
void ASolarFlightPawn::ToggleThrottle()
{
    if (auto* M = Lab(this); M && M->bReady && !M->bPaused && !M->Galaxy.bOpen)
    {
        if (M->ProfileIndex == 2) M->SetProfile(1);
        M->Flight.SetThrottle(M->Flight.GetState().Throttle > 0 ? 0 : 1, M->Message);
    }
}
void ASolarFlightPawn::FullImpulse() { if (auto* M = Lab(this)) M->ToggleFullImpulse(); }
void ASolarFlightPawn::BrakeOn()
{
    if(auto* M=Lab(this); !M || M->Galaxy.bOpen) return;
    bBrake = true;
    if (auto* M = Lab(this)) { if (M->ProfileIndex == 2) M->SetProfile(1); FString Error; M->Flight.SetThrottle(0, Error); }
}
void ASolarFlightPawn::ResetCamera() { OrbitYaw = OrbitPitch = 0; bFollowShip = true; bCameraInitialized = false; }
void ASolarFlightPawn::ZoomIn() { if(Lab(this) && !Lab(this)->Galaxy.bOpen) CameraDistanceMetres = FMath::Max(650.0, CameraDistanceMetres / 1.12); }
void ASolarFlightPawn::ZoomOut() { if(Lab(this) && !Lab(this)->Galaxy.bOpen) CameraDistanceMetres = FMath::Min(6000.0, CameraDistanceMetres * 1.12); }
void ASolarFlightPawn::Slow() { if (auto* M = Lab(this)) M->SetProfile(0); }
void ASolarFlightPawn::Fast() { if (auto* M = Lab(this)) M->SetProfile(1); }
void ASolarFlightPawn::ResetFlight() { if(auto* M=Lab(this);M && M->Galaxy.bOpen)return; bBrake = false; if (auto* M = Lab(this)) M->ResetFlight(); ResetCamera(); }
void ASolarFlightPawn::PauseFlight() { if (auto* M = Lab(this)) M->TogglePause(); }
void ASolarFlightPawn::ExitGame() { if(auto* M=Lab(this);M && M->Galaxy.bOpen){M->ToggleMap();return;} if (auto* PC = Cast<APlayerController>(Controller)) PC->ConsoleCommand(TEXT("quit")); }
void ASolarFlightPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* M = Lab(this);
    if (!M || !M->bReady) return;
    if(M->Galaxy.bOpen)
    {
        FString Error; M->Flight.SetInput({},Error);
        auto* PC=Cast<APlayerController>(Controller);
        if(!PC)return;
        float X=0,Y=0,DX=0,DY=0; PC->GetMousePosition(X,Y);PC->GetInputMouseDelta(DX,DY);
        int32 W=0,H=0;PC->GetViewportSize(W,H);
        const float Wheel=(PC->WasInputKeyJustPressed(EKeys::MouseScrollUp)?1.f:0.f)-(PC->WasInputKeyJustPressed(EKeys::MouseScrollDown)?1.f:0.f);
        M->HandleMapAction(M->Galaxy.Input(FVector2D(X,Y),FVector2D(DX,-DY),PC->WasInputKeyJustPressed(EKeys::LeftMouseButton),
            PC->IsInputKeyDown(EKeys::RightMouseButton),PC->IsInputKeyDown(EKeys::MiddleMouseButton),Wheel,PC->WasInputKeyJustPressed(EKeys::Home),M->MapSystems,FVector2D(W,H)));
        if(M->Galaxy.bSearchFocused)
        {
            FString Query=M->Galaxy.SearchQuery;
            for(TCHAR C=TEXT('A');C<=TEXT('Z');++C)if(PC->WasInputKeyJustPressed(FKey(FName(*FString::Chr(C)))))Query+=FString::Chr(C);
            const FKey Digits[]={EKeys::Zero,EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::Six,EKeys::Seven,EKeys::Eight,EKeys::Nine};
            for(int32 I=0;I<10;++I)if(PC->WasInputKeyJustPressed(Digits[I]))Query+=FString::Chr(TEXT('0')+I);
            if(PC->WasInputKeyJustPressed(EKeys::SpaceBar))Query+=TEXT(" ");
            if(PC->WasInputKeyJustPressed(EKeys::BackSpace))Query=Query.LeftChop(1);
            if(PC->WasInputKeyJustPressed(EKeys::Enter))M->Galaxy.bSearchFocused=false;
            if(Query!=M->Galaxy.SearchQuery)M->Galaxy.SetSearchQuery(Query);
        }
        return;
    }
    // Input devices command the domain; visuals never write authoritative pose.
    Daedalus::FFlightInput Input;
    Input.Yaw = YawInput; Input.Pitch = -PitchInput; Input.bBrake = bBrake;
    if (const auto* PC = Cast<APlayerController>(Controller))
    {
        // A focus loss flushes keys in Unreal; polling the brake prevents a
        // missed release from latching it. Scripted controller input uses this
        // same pressed-key state.
        Input.bBrake = PC->IsInputKeyDown(EKeys::SpaceBar) || PC->IsInputKeyDown(EKeys::X);
    }
    FString Error;
    if (!M->Flight.SetInput(Input, Error)) M->Message = Error;
    const auto& State = M->Flight.GetState();
    if (!bCameraInitialized) { FollowYaw = State.YawDegrees; FollowPitch = State.PitchDegrees; }
    if (bFollowShip)
    {
        const float Ease = 1 - FMath::Exp(-DeltaSeconds * 2.2f);
        FollowYaw += FRotator::NormalizeAxis(State.YawDegrees - FollowYaw) * Ease;
        FollowPitch += (State.PitchDegrees - FollowPitch) * Ease;
    }
    // Manual orbit is immediate, independent of ship turning until Home.
    // Only the following heading has lag, as in the web prototype.
    const FRotator Rotation(FMath::Clamp(FollowPitch - 18.3f + OrbitPitch,-85.f,85.f), FollowYaw + OrbitYaw, 0);
    bCameraInitialized = true;
    const FVector Focus(0, 0, -6000);
    Camera->SetWorldLocationAndRotation(Focus - Rotation.Vector() * CameraDistanceMetres * 100.0, Rotation);
}

ASolarFlightGameMode::ASolarFlightGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    DefaultPawnClass = ASolarFlightPawn::StaticClass(); HUDClass = ASolarFlightHUD::StaticClass();
    static ConstructorHelpers::FObjectFinder<UStaticMesh> ShipFinder(TEXT("/Game/Ships/Daedalus/SM_Daedalus.SM_Daedalus"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Game/Solar/SM_SolarSphere.SM_SolarSphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> EarthFinder(TEXT("/Game/Solar/Materials/M_Earth.M_Earth"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> SunFinder(TEXT("/Game/Solar/Materials/M_Sun.M_Sun"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> AtmosFinder(TEXT("/Game/Solar/Materials/M_Atmosphere.M_Atmosphere"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> StarFinder(TEXT("/Game/Solar/Materials/M_Star.M_Star"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DustFinder(TEXT("/Game/Solar/Materials/M_Dust.M_Dust"));
    ShipAsset = ShipFinder.Object; SphereAsset = SphereFinder.Object; PlaneAsset = PlaneFinder.Object;
    ConstructorHelpers::FObjectFinder<UStaticMesh> LightsFinder(TEXT("/Game/Ships/Daedalus/Effects/SM_DaedalusLights.SM_DaedalusLights"));
    LightsAsset = LightsFinder.Object;
    for (const TCHAR* Name : {TEXT("ENG_Main_Port_Glow"),TEXT("ENG_Main_Starboard_Glow"),TEXT("ENG_Pod_Port_Inner_Glow"),TEXT("ENG_Pod_Port_Outer_Glow"),TEXT("ENG_Pod_Starboard_Inner_Glow"),TEXT("ENG_Pod_Starboard_Outer_Glow")})
    {
        const FString Path = FString::Printf(TEXT("/Game/Ships/Daedalus/Effects/DaedalusEngineGlow/StaticMeshes/%s.%s"),Name,Name);
        ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(*Path);
        GlowAssets.Add(Finder.Object);
    }
    EarthMaterial = EarthFinder.Object; SunMaterial = SunFinder.Object; AtmosphereMaterial = AtmosFinder.Object;
    StarMaterial = StarFinder.Object; DustMaterial = DustFinder.Object;
}
bool ASolarFlightGameMode::LoadSettings()
{
    TSharedPtr<FJsonObject> Json;
    if (!ReadObject(FPaths::ProjectContentDir() / TEXT("Data/Solar/flight.json"), Json)) return false;
    double Version = 0, Length = 0, Radius = 0;
    if (!Number(Json, TEXT("version"), Version) || Version != 1 || !Number(Json, TEXT("shipLengthMetres"), Length) || Length != 600
        || !Number(Json, TEXT("shipRadiusMetres"), Radius) || !Number(Json, TEXT("earthRadiusMetres"), EarthRadius)
        || !Number(Json, TEXT("sunRadiusMetres"), SunRadius) || EarthRadius <= 0 || SunRadius <= 0
        || !Vector(Json, TEXT("sunPositionMetres"), SunPosition) || !Vector(Json, TEXT("startPositionMetres"), InitialState.PositionMetres)
        || !Number(Json, TEXT("startYawDegrees"), InitialState.YawDegrees) || !Number(Json, TEXT("startPitchDegrees"), InitialState.PitchDegrees)) return false;
    Daedalus::FFlightConfig Base;
    Base.ShipRadiusMetres = Radius;
    if (!Number(Json, TEXT("turnRateDegrees"), Base.TurnRateDegrees) || !Number(Json, TEXT("angularAccelerationDegrees"), Base.AngularAccelerationDegrees)
        || !Number(Json, TEXT("pitchLimitDegrees"), Base.PitchLimitDegrees) || !Number(Json, TEXT("bankDegrees"), Base.BankDegrees)) return false;
    const TArray<TSharedPtr<FJsonValue>>* Rows = nullptr;
    if (!Json->TryGetArrayField(TEXT("profiles"), Rows) || Rows->Num() != 3) return false;
    for (const auto& Row : *Rows)
    {
        const auto O = Row->AsObject(); FSolarFlightProfile P; P.Config = Base;
        if (!O || !O->TryGetStringField(TEXT("name"), P.Name) || !Number(O, TEXT("speed"), P.Config.MaxSpeed)
            || !Number(O, TEXT("acceleration"), P.Config.Acceleration) || !Number(O, TEXT("braking"), P.Config.Braking)
            || !Number(O, TEXT("coastDeceleration"), P.Config.CoastDeceleration)
            || !Number(O, TEXT("lateralAcceleration"), P.Config.LateralAcceleration) || !P.Config.Validate(Message)) return false;
        Profiles.Add(P);
    }
    if (!LoadSystem()) return false;
    return Flight.Initialize(Profiles[ProfileIndex].Config, InitialState, Bodies, Message);
}
void ASolarFlightGameMode::BeginPlay()
{
    Super::BeginPlay();
    SetActorHiddenInGame(false);
    bReady = LoadSettings() && CreateScene();
    if (!bReady) { Message = TEXT("Letovou scénu nelze načíst. Zkontroluj obsah a protokol."); UE_LOG(LogTemp, Error, TEXT("SOLAR_LOAD_FAILED")); }
    if (auto* PC = GetWorld()->GetFirstPlayerController())
    {
        if(FParse::Param(FCommandLine::Get(),TEXT("SolarNative")))if(auto* Settings=GEngine->GetGameUserSettings()){
            Settings->SetScreenResolution(Settings->GetDesktopResolution());Settings->SetFullscreenMode(EWindowMode::WindowedFullscreen);Settings->SetResolutionScaleValueEx(100);Settings->ApplySettings(false);}
        ApplySharpRenderingSettings();
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
        if (PC->GetPawn()) PC->GetPawn()->AddTickPrerequisiteActor(this);
    }
    FParse::Value(FCommandLine::Get(), TEXT("SolarProbe="), ProbeDirectory);
    bSharpProbe = FParse::Value(FCommandLine::Get(), TEXT("SolarSharpProbe="), ProbeDirectory);
}
UStaticMeshComponent* ASolarFlightGameMode::MakeMesh(UStaticMesh* Asset, UMaterialInterface* Material)
{
    auto* Actor = GetWorld()->SpawnActor<AActor>();
    auto* Component = NewObject<UStaticMeshComponent>(Actor);
    Component->SetMobility(EComponentMobility::Movable);
    Actor->SetRootComponent(Component); Actor->AddInstanceComponent(Component); Component->RegisterComponent();
    Component->SetStaticMesh(Asset);
    if (Material) for (int32 Slot = 0; Slot < Component->GetNumMaterials(); ++Slot) Component->SetMaterial(Slot, Material);
    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Component->SetCastShadow(false);
    return Component;
}
bool ASolarFlightGameMode::CreateScene()
{
    if (!ShipAsset || !LightsAsset || GlowAssets.Num()!=6 || GlowAssets.Contains(nullptr) || !SphereAsset || !PlaneAsset || !EarthMaterial || !SunMaterial || !AtmosphereMaterial || !StarMaterial || !DustMaterial) return false;
    if (FMath::Abs(ShipAsset->GetBounds().BoxExtent.X * 2 - 60000) > 30) return false;
    Ship = MakeMesh(ShipAsset, nullptr); Ship->SetCastShadow(true);
    Ship->SetTextureForceResidentFlag(true);
    HullLights = MakeMesh(LightsAsset, nullptr);
    // Latest Claude hull uses separately delivered window geometry.
    HullLights->SetVisibility(true);
    if (auto* Detail=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Ships/Daedalus/Details/SM_DaedalusAddOns.SM_DaedalusAddOns"))) ShipDetails.Add(MakeMesh(Detail,nullptr));
    TSharedPtr<FJsonObject> Details;
    const TArray<TSharedPtr<FJsonValue>>* DetailRows=nullptr;
    if(!ReadObject(FPaths::ProjectContentDir()/TEXT("Data/Solar/ship-details.json"),Details) || !Details->TryGetArrayField(TEXT("meshes"),DetailRows) || DetailRows->Num()!=38) return false;
    for(const auto& Row:*DetailRows){FString Path;if(!Row->TryGetString(Path) || !Path.StartsWith(TEXT("/Game/Ships/Daedalus/Details/"))) return false;
        auto* Mesh=LoadObject<UStaticMesh>(nullptr,*Path);if(!Mesh)return false;ShipDetails.Add(MakeMesh(Mesh,nullptr));}
    for (const auto& Detail : ShipDetails) Detail->SetTextureForceResidentFlag(true);
    for (const auto& Asset : GlowAssets)
    {
        auto* Glow = MakeMesh(Asset,nullptr);
        for (int32 Slot=0;Slot<Glow->GetNumMaterials();++Slot)
        {
            auto* Dynamic = UMaterialInstanceDynamic::Create(Glow->GetMaterial(Slot),this);
            Glow->SetMaterial(Slot,Dynamic); EngineDynamics.Add(Dynamic);
        }
        EngineGlows.Add(Glow);
    }
    if (!CreateSystem()) return false;
    auto* Light = GetWorld()->SpawnActor<ADirectionalLight>(); SolarLight = Light;
    auto* LC = CastChecked<UDirectionalLightComponent>(Light->GetLightComponent());
    LC->SetMobility(EComponentMobility::Movable); LC->SetIntensity(2.8f); LC->SetLightSourceAngle(2);
    LC->SetLightColor(FLinearColor(1,.92f,.80f));
    Light->SetActorRotation((-FVector(SunPosition).GetSafeNormal()).Rotation());
    // A gentle camera-side fill keeps dark hull geometry readable; it is an
    // explicit artistic aid, not a second celestial light source.
    auto* Fill = GetWorld()->SpawnActor<ADirectionalLight>();
    Fill->GetLightComponent()->SetMobility(EComponentMobility::Movable);
    Fill->GetLightComponent()->SetIntensity(.45f); Fill->GetLightComponent()->SetCastShadows(false);
    Fill->GetLightComponent()->SetLightColor(FLinearColor(.55f,.72f,1));
    Fill->SetActorRotation(FRotator(-35, 110, 0));
    Stars = NewObject<UInstancedStaticMeshComponent>(this); Stars->RegisterComponent(); Stars->SetStaticMesh(PlaneAsset);
    Stars->SetMaterial(0, StarMaterial); Stars->SetCollisionEnabled(ECollisionEnabled::NoCollision); Stars->SetCastShadow(false);
    Stars->NumCustomDataFloats = 3;
    FString Text; TArray<TSharedPtr<FJsonValue>> Catalog;
    if (!FFileHelper::LoadFileToString(Text, *(FPaths::ProjectContentDir() / TEXT("Data/Solar/stars-hyg.json")))
        || !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Catalog) || Catalog.Num() != 8920) return false;
    constexpr double SkyRadius = 2e9;
    constexpr double Obliquity = 23.4392911 * PI / 180;
    for (const auto& Entry : Catalog)
    {
        const auto& R = Entry->AsArray(); if (R.Num() != 5) return false;
        const double RA = R[1]->AsNumber() * PI / 12, Dec = R[2]->AsNumber() * PI / 180, Mag = R[3]->AsNumber();
        const double BV = R[4]->IsNull() ? .65 : R[4]->AsNumber();
        const double X = FMath::Cos(Dec) * FMath::Cos(RA), Y = FMath::Cos(Dec) * FMath::Sin(RA), Z = FMath::Sin(Dec);
        const FVector Direction(X, Y * FMath::Cos(Obliquity) + Z * FMath::Sin(Obliquity), Z * FMath::Cos(Obliquity) - Y * FMath::Sin(Obliquity));
        const double Size = FMath::Clamp(1.08 + (6.5 - Mag) * .17, 1.08, 2.5);
        const double Width = SkyRadius * .00042 * Size;
        const int32 I = Stars->AddInstance(FTransform(FRotationMatrix::MakeFromZ(-Direction).ToQuat(), Direction * SkyRadius, FVector(Width / 100)));
        const double Brightness = FMath::Clamp(FMath::Pow(10.0, -.12 * (Mag - 1)), .3, 2.5);
        const double Warm = FMath::Clamp((BV - .3) / 1.7, 0.0, 1.0), Cool = FMath::Clamp((.3 - BV) / .7, 0.0, 1.0);
        const double Saturation = FMath::Clamp((5 - Mag) / 5, 0.0, .7);
        Stars->SetCustomDataValue(I, 0, Brightness * (1 - Cool * Saturation * .25));
        Stars->SetCustomDataValue(I, 1, Brightness * (1 - Warm * Saturation * .15));
        Stars->SetCustomDataValue(I, 2, Brightness * (1 - Warm * Saturation * .42));
    }
    Stars->MarkRenderStateDirty();
    // Sparse local navigation cues, independent of the distant fixed stars.
    Dust = NewObject<UInstancedStaticMeshComponent>(this); Dust->RegisterComponent(); Dust->SetStaticMesh(PlaneAsset);
    Dust->SetMaterial(0, DustMaterial); Dust->SetCollisionEnabled(ECollisionEnabled::NoCollision); Dust->SetCastShadow(false);
    FRandomStream Random(6304);
    for (int32 I = 0; I < 500; ++I)
    {
        DustPositions.Add(FVector3d(Random.FRandRange(-2500, 2500), Random.FRandRange(-2500, 2500), Random.FRandRange(-2500, 2500)));
        Dust->AddInstance(FTransform::Identity);
    }
    UpdateScene(0);
    return true;
}
int32 ASolarFlightGameMode::StarCount() const { return Stars ? Stars->GetInstanceCount() : 0; }
void ASolarFlightGameMode::ChangeThrottle(double Step)
{
    if (!bReady || bPaused || Galaxy.bOpen) return;
    double Value = FMath::Clamp(Flight.GetState().Throttle + Step, -.25, 1.0);
    if (FMath::Abs(Value) < .0001) Value = 0;
    if (Value <= 0 && ProfileIndex == 2) SetProfile(1);
    Flight.SetThrottle(Value, Message);
}
void ASolarFlightGameMode::SetProfile(int32 Index)
{
    if (!bReady || bPaused || Galaxy.bOpen || !Profiles.IsValidIndex(Index) || Index == ProfileIndex) return;
    if (Flight.SetConfig(Profiles[Index].Config, Message)) ProfileIndex = Index;
}
void ASolarFlightGameMode::ToggleFullImpulse()
{
    if (!bReady || bPaused || Galaxy.bOpen) return;
    const int32 Target = ProfileIndex == 2 ? 1 : 2;
    SetProfile(Target);
    if (Target == 2 && ProfileIndex == Target) Flight.SetThrottle(1, Message);
}
void ASolarFlightGameMode::ResetFlight()
{
    if(Galaxy.bOpen)return;
    if(ActiveSystem!=0){ActivateSystem(0,3);}
    if (Profiles.IsValidIndex(ProfileIndex)) bReady = Flight.Initialize(Profiles[ProfileIndex].Config, InitialState, Bodies, Message);
    bPaused = false;
}
void ASolarFlightGameMode::TogglePause() { if(Galaxy.bOpen)return; bPaused = !bPaused; Flight.SetPaused(bPaused); }
void ASolarFlightGameMode::UpdateScene(float DeltaSeconds)
{
    const auto& S = Flight.GetState();
    Ship->SetWorldLocationAndRotation(FVector::ZeroVector, FQuat(S.Attitude()));
    HullLights->SetWorldLocationAndRotation(FVector::ZeroVector, FQuat(S.Attitude()));
    // The exported glow centres are baked in the same ship frame as the hull.
    // Change brightness, not scale/pose, so outlets remain exactly registered.
    EngineGlowLevel = .08 + .92 * FMath::Max(0.0,S.Throttle);
    for (const auto& Glow : EngineGlows) Glow->SetWorldLocationAndRotation(FVector::ZeroVector,FQuat(S.Attitude()));
    for (const auto& Dynamic : EngineDynamics) Dynamic->SetScalarParameterValue(TEXT("EngineLevel"),EngineGlowLevel);
    const auto* Pawn = Cast<ASolarFlightPawn>(GetWorld()->GetFirstPlayerController()->GetPawn());
    const FVector CameraPosition = Pawn ? Pawn->Camera->GetComponentLocation() : FVector::ZeroVector;
    UpdateSystem(CameraPosition);
    Stars->SetWorldLocation(CameraPosition);
    for (const auto& Detail : ShipDetails) Detail->SetWorldLocationAndRotation(FVector::ZeroVector,FQuat(S.Attitude()));
    const double Speed = S.VelocityMetresPerSecond.Size();
    Dust->SetVisibility(Speed > .05);
    for (int32 I = 0; I < DustPositions.Num(); ++I)
    {
        auto& P = DustPositions[I]; P -= S.VelocityMetresPerSecond.GetSafeNormal() * FMath::Min(Speed, 2200.0) * ((bPaused || Galaxy.bOpen) ? 0.0 : FMath::Min<double>(DeltaSeconds, .1));
        for (int32 A = 0; A < 3; ++A) P[A] = FMath::Fmod(FMath::Fmod(P[A] + 2500, 5000) + 5000, 5000) - 2500;
        const FVector Location(P * 100);
        const double Fade = FMath::Clamp((P.Size() - 400) / 600, 0.0, 1.0);
        Dust->UpdateInstanceTransform(I, FTransform(FRotationMatrix::MakeFromZ(CameraPosition - Location).ToQuat(), Location, FVector(Fade * (2+8*FMath::Clamp(Speed/250000.,0.,1.)),Fade*1.4,1)), false, I == DustPositions.Num() - 1);
    }
}
void ASolarFlightGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    FrameMilliseconds += (DeltaSeconds * 1000 - FrameMilliseconds) * .08;
    if (!bReady) { if (!ProbeDirectory.IsEmpty()) { if (bSharpProbe) TickSharpProbe(); else TickProbe(); } return; }
    Flight.Advance(DeltaSeconds);
    if (!Flight.GetError().IsEmpty()) Message = Flight.GetError();
    UpdateScene(DeltaSeconds);
    RefreshNavigation();
    if (!ProbeDirectory.IsEmpty()) { if (bSharpProbe) TickSharpProbe(); else TickProbe(); }
}
void ASolarFlightHUD::DrawHUD()
{
    Super::DrawHUD();
    auto* M = Lab(this); if (!M || !Canvas) return;
    if(M->bReady && M->Galaxy.bOpen){M->Galaxy.Draw(Canvas,GEngine->GetLargeFont(),M->MapSystems,M->ActiveSystem,M->PreviewMetrics());return;}
    const float Scale = FMath::Max(.6f,FMath::Min(Canvas->SizeX / 1280.0f,Canvas->SizeY / 720.0f));
    const float Width = Canvas->SizeX / Scale, Height = Canvas->SizeY / Scale;
    const FLinearColor Pale(.55f,.69f,.77f), Cyan(.38f,.81f,.92f), White(.88f,.94f,.98f);
    auto Text = [&](const FString& S, float X, float Y, FLinearColor C, float Factor = 1.0f)
    {
        // Rasterize the runtime font at actual output pixels. Scaling a tiny
        // atlas afterwards blurs 4K text; offline fonts also omit Czech glyphs.
        FCanvasTextItem Item(FVector2D(X*Scale,Y*Scale),FText::FromString(S),
            FSlateFontInfo(GEngine->GetLargeFont(),FMath::Max(8,FMath::RoundToInt(14*Scale*Factor))),C);
        Item.EnableShadow(FLinearColor::Black,FVector2D(Scale,Scale));
        Canvas->DrawItem(Item);
    };
    auto Center = [&](const FString& S,float Y,FLinearColor C,float Factor=1.0f)
    {
        FCanvasTextItem Item(FVector2D(Canvas->SizeX*.5f,Y*Scale),FText::FromString(S),
            FSlateFontInfo(GEngine->GetLargeFont(),FMath::Max(8,FMath::RoundToInt(15*Scale*Factor))),C);
        Item.bCentreX=true;Item.EnableShadow(FLinearColor::Black,FVector2D(Scale,Scale));Canvas->DrawItem(Item);
    };
    DrawRect(FLinearColor(.008f,.020f,.030f,.88f),12*Scale,12*Scale,710*Scale,76*Scale);
    Text(TEXT("DAEDALUS  /  LETOVÁ ZKOUŠKA"),22,20,Cyan);
    if (!M->bReady) { Text(M->Message,22,45,FLinearColor::Red); return; }
    const auto& S=M->Flight.GetState();
    const double Speed=S.VelocityMetresPerSecond.Size();
    Text(M->ActiveSystem==0 ? FString::Printf(TEXT("Země · nad povrchem %.0f km"),(S.PositionMetres.Size()-M->EarthRadius)/1000) : M->Systems[M->ActiveSystem].Name,22,43,Pale);
    const auto& Target=M->BodyDefinitions[M->SelectedBody];
    const double Distance=FMath::Max(0.,(Target.Position-S.PositionMetres).Size()-Target.Radius*Target.Shape.GetMax());
    Text(Target.bKnownPosition ? FString::Printf(TEXT("Cíl: %s · %.0f km   PgUp/PgDn · F testovací přesun · M mapa"),*Target.Name,Distance/1000) : FString::Printf(TEXT("%s · poloha neznámá · M mapa"),*Target.Name),22,65,Cyan,.85f);
    const auto Nav=M->Navigation.Query(Speed,M->Flight.GetConfig().MaxSpeed,M->Galaxy.DesiredSeconds);
    if(Nav.bValid)
    {
        FString Name=M->Navigation.GetTargetBodyId();
        for(const auto& Sys:M->MapSystems)if(Sys.Id==M->Navigation.GetTargetSystemId())
            for(const auto& B:Sys.Bodies)if(B.Id==Name){Name=Sys.Name+TEXT(" / ")+B.Name;break;}
        const float Right=Width-395;
        DrawRect(FLinearColor(.008f,.020f,.030f,.88f),Right*Scale,12*Scale,383*Scale,76*Scale);
        Text(TEXT("NAVIGACE: ")+Name.Left(40),Right+12,20,Cyan,.85f);
        const FString Range=Nav.DistanceMetres>=Daedalus::FNavigationPlan::LightYearMetres*.01 ? FString::Printf(TEXT("%.2f světelných let"),Nav.DistanceMetres/Daedalus::FNavigationPlan::LightYearMetres) : FString::Printf(TEXT("%.0f km"),Nav.DistanceMetres/1000);
        Text(TEXT("Vzdálenost: ")+Range,Right+12,43,Pale,.85f);
        Text(TEXT("M — mapa, databáze a doba letu"),Right+12,65,Pale,.8f);
    }
    const FString SpeedText=Speed>=1000?FString::Printf(TEXT("%.0f km/s"),Speed/1000):FString::Printf(TEXT("%.0f m/s"),Speed);
    Center(SpeedText,Height-228,White,2.8f);
    Center(TEXT("S K U T E Č N Á   R Y C H L O S T"),Height-165,Pale,.72f);
    const float Left=(Width-370)*.5f,Top=Height-145;
    DrawRect(FLinearColor(.008f,.020f,.030f,.90f),Left*Scale,Top*Scale,370*Scale,86*Scale);
    DrawRect(FLinearColor(.07f,.15f,.19f,.9f),Left*Scale,Top*Scale,370*Scale,Scale);
    Text(TEXT("NASTAVENÝ TAH"),Left+13,Top+10,Pale,.85f);
    Text(FString::Printf(TEXT("%.0f %%"),S.Throttle*100),Left+300,Top+10,Cyan);
    DrawRect(FLinearColor(.15f,.20f,.23f), (Left+57)*Scale,(Top+40)*Scale,250*Scale,4*Scale);
    const float Throttle=FMath::Clamp(float((S.Throttle+.25)/1.25),0.f,1.f);
    DrawRect(Cyan,(Left+57)*Scale,(Top+40)*Scale,250*Throttle*Scale,4*Scale);
    DrawRect(White,(Left+54+250*Throttle)*Scale,(Top+36)*Scale,6*Scale,12*Scale);
    Text(TEXT("Q −"),Left+13,Top+31,Pale); Text(TEXT("+ E"),Left+322,Top+31,Pale);
    Center(M->Profiles[M->ProfileIndex].Name,Top+62,Pale,.9f);
    Center(TEXT("1 Přístavní / 2 Impuls    R let / stop    Shift+R plný impuls    Mezerník brzda"),Height-48,Pale,.85f);
    Center(TEXT("W/S sklon   A/D zatáčení   Pravé tlačítko + myš kamera   Kolečko zoom   Home za loď"),Height-28,Pale,.8f);
    if (M->bPaused) Center(TEXT("POZASTAVENO — P pokračovat"),75,Cyan,1.2f);
    if (!S.ContactBodyId.IsEmpty()) Center(TEXT("Povrchová ochrana: otoč se a odleť."),100,Cyan);
    if (!M->Message.IsEmpty())
    {
        DrawRect(FLinearColor(.008f,.020f,.030f,.88f),(Width-680)*.5f*Scale,120*Scale,680*Scale,27*Scale);
        Center(M->Message,125,Pale);
    }
}



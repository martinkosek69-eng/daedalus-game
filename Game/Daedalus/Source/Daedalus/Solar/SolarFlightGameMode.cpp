#include "Solar/SolarFlightGameMode.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
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
    Camera->SetFieldOfView(60);
    Camera->PostProcessSettings.bOverride_BloomIntensity = true;
    Camera->PostProcessSettings.BloomIntensity = .3f;
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
    Input->BindKey(EKeys::Escape, IE_Pressed, this, &ASolarFlightPawn::ExitGame);
}
void ASolarFlightPawn::MouseX(float V) { if (bOrbit) OrbitYaw = FRotator::NormalizeAxis(OrbitYaw + V * 1.8f); }
void ASolarFlightPawn::MouseY(float V) { if (bOrbit) OrbitPitch = FMath::Clamp(OrbitPitch + V * 1.8f, -60.0f, 65.0f); }
void ASolarFlightPawn::MoreThrottle() { if (auto* M = Lab(this)) M->ChangeThrottle(.2); }
void ASolarFlightPawn::LessThrottle() { if (auto* M = Lab(this)) M->ChangeThrottle(-.2); }
void ASolarFlightPawn::ToggleThrottle()
{
    if (auto* M = Lab(this); M && M->bReady && !M->bPaused)
        M->Flight.SetThrottle(M->Flight.GetState().Throttle > 0 ? 0 : 1, M->Message);
}
void ASolarFlightPawn::BrakeOn()
{
    bBrake = true;
    if (auto* M = Lab(this)) { FString Error; M->Flight.SetThrottle(0, Error); }
}
void ASolarFlightPawn::ResetCamera() { OrbitYaw = OrbitPitch = 0; }
void ASolarFlightPawn::ZoomIn() { CameraDistanceMetres = FMath::Max(650.0, CameraDistanceMetres / 1.12); }
void ASolarFlightPawn::ZoomOut() { CameraDistanceMetres = FMath::Min(6000.0, CameraDistanceMetres * 1.12); }
void ASolarFlightPawn::Slow() { if (auto* M = Lab(this)) M->SetProfile(0); }
void ASolarFlightPawn::Fast() { if (auto* M = Lab(this)) M->SetProfile(1); }
void ASolarFlightPawn::ResetFlight() { bBrake = false; if (auto* M = Lab(this)) M->ResetFlight(); ResetCamera(); }
void ASolarFlightPawn::PauseFlight() { if (auto* M = Lab(this)) M->TogglePause(); }
void ASolarFlightPawn::ExitGame() { if (auto* PC = Cast<APlayerController>(Controller)) PC->ConsoleCommand(TEXT("quit")); }
void ASolarFlightPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* M = Lab(this);
    if (!M || !M->bReady) return;
    // Input devices command the domain; visuals never write authoritative pose.
    Daedalus::FFlightInput Input;
    Input.Yaw = YawInput; Input.Pitch = PitchInput; Input.bBrake = bBrake;
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
    const FRotator Desired(-12.0 + FMath::Clamp(State.PitchDegrees * .35, -20.0, 20.0) + OrbitPitch, State.YawDegrees + OrbitYaw, 0);
    FRotator Rotation = bCameraInitialized ? FMath::RInterpTo(Camera->GetComponentRotation(), Desired, DeltaSeconds, 5) : Desired;
    Rotation.Roll = 0;
    bCameraInitialized = true;
    const FVector Focus(0, 0, 6000);
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
    if (!Json->TryGetArrayField(TEXT("profiles"), Rows) || Rows->Num() != 2) return false;
    for (const auto& Row : *Rows)
    {
        const auto O = Row->AsObject(); FSolarFlightProfile P; P.Config = Base;
        if (!O || !O->TryGetStringField(TEXT("name"), P.Name) || !Number(O, TEXT("speed"), P.Config.MaxSpeed)
            || !Number(O, TEXT("acceleration"), P.Config.Acceleration) || !Number(O, TEXT("braking"), P.Config.Braking)
            || !Number(O, TEXT("coastDeceleration"), P.Config.CoastDeceleration)
            || !Number(O, TEXT("lateralAcceleration"), P.Config.LateralAcceleration) || !P.Config.Validate(Message)) return false;
        Profiles.Add(P);
    }
    Bodies = {{TEXT("sol.earth"), FVector3d::ZeroVector, EarthRadius}, {TEXT("sol.sun"), SunPosition, SunRadius}};
    return Flight.Initialize(Profiles[0].Config, InitialState, Bodies, Message);
}
void ASolarFlightGameMode::BeginPlay()
{
    Super::BeginPlay();
    SetActorHiddenInGame(false);
    bReady = LoadSettings() && CreateScene();
    if (!bReady) { Message = TEXT("Letovou scénu nelze načíst. Zkontroluj obsah a protokol."); UE_LOG(LogTemp, Error, TEXT("SOLAR_LOAD_FAILED")); }
    if (auto* PC = GetWorld()->GetFirstPlayerController())
    {
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
        if (PC->GetPawn()) PC->GetPawn()->AddTickPrerequisiteActor(this);
    }
    FParse::Value(FCommandLine::Get(), TEXT("SolarProbe="), ProbeDirectory);
}
UStaticMeshComponent* ASolarFlightGameMode::MakeMesh(UStaticMesh* Asset, UMaterialInterface* Material)
{
    auto* Actor = GetWorld()->SpawnActor<AActor>();
    auto* Component = NewObject<UStaticMeshComponent>(Actor);
    Component->SetMobility(EComponentMobility::Movable);
    Actor->SetRootComponent(Component); Actor->AddInstanceComponent(Component); Component->RegisterComponent();
    Component->SetStaticMesh(Asset); if (Material) Component->SetMaterial(0, Material);
    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Component->SetCastShadow(false);
    return Component;
}
bool ASolarFlightGameMode::CreateScene()
{
    if (!ShipAsset || !LightsAsset || GlowAssets.Num()!=6 || GlowAssets.Contains(nullptr) || !SphereAsset || !PlaneAsset || !EarthMaterial || !SunMaterial || !AtmosphereMaterial || !StarMaterial || !DustMaterial) return false;
    if (FMath::Abs(ShipAsset->GetBounds().BoxExtent.X * 2 - 60000) > 30) return false;
    Ship = MakeMesh(ShipAsset, nullptr); Ship->SetCastShadow(true);
    HullLights = MakeMesh(LightsAsset, nullptr);
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
    EarthDynamic = UMaterialInstanceDynamic::Create(EarthMaterial, this);
    AtmosphereDynamic = UMaterialInstanceDynamic::Create(AtmosphereMaterial, this);
    Earth = MakeMesh(SphereAsset, EarthDynamic);
    Atmosphere = MakeMesh(SphereAsset, AtmosphereDynamic);
    Sun = MakeMesh(SphereAsset, SunMaterial);
    auto* Light = GetWorld()->SpawnActor<ADirectionalLight>();
    auto* LC = CastChecked<UDirectionalLightComponent>(Light->GetLightComponent());
    LC->SetMobility(EComponentMobility::Movable); LC->SetIntensity(4.5f); LC->SetLightSourceAngle(.53f);
    Light->SetActorRotation((-FVector(SunPosition).GetSafeNormal()).Rotation());
    // A gentle camera-side fill keeps dark hull geometry readable; it is an
    // explicit artistic aid, not a second celestial light source.
    auto* Fill = GetWorld()->SpawnActor<ADirectionalLight>();
    Fill->GetLightComponent()->SetMobility(EComponentMobility::Movable);
    Fill->GetLightComponent()->SetIntensity(.8f); Fill->GetLightComponent()->SetCastShadows(false);
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
        const double Width = SkyRadius * .00075 * Size;
        const int32 I = Stars->AddInstance(FTransform(FRotationMatrix::MakeFromZ(-Direction).ToQuat(), Direction * SkyRadius, FVector(Width / 100)));
        const double Brightness = FMath::Clamp(FMath::Pow(10.0, -.16 * (Mag - 1)), .09, 1.3);
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
    for (int32 I = 0; I < 100; ++I)
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
    if (!bReady || bPaused) return;
    double Value = FMath::Clamp(Flight.GetState().Throttle + Step, -.25, 1.0);
    if (FMath::Abs(Value) < .0001) Value = 0;
    Flight.SetThrottle(Value, Message);
}
void ASolarFlightGameMode::SetProfile(int32 Index)
{
    if (!bReady || !Profiles.IsValidIndex(Index) || Index == ProfileIndex) return;
    if (bPaused || Flight.GetState().VelocityMetresPerSecond.Size() > .01 || FMath::Abs(Flight.GetState().Throttle) > .001)
    { Message = TEXT("Před změnou režimu zastav loď a nastav tah na nulu."); return; }
    if (Flight.Initialize(Profiles[Index].Config, Flight.GetState(), Bodies, Message)) ProfileIndex = Index;
}
void ASolarFlightGameMode::ResetFlight()
{
    if (Profiles.IsValidIndex(ProfileIndex)) bReady = Flight.Initialize(Profiles[ProfileIndex].Config, InitialState, Bodies, Message);
    bPaused = false;
}
void ASolarFlightGameMode::TogglePause() { bPaused = !bPaused; Flight.SetPaused(bPaused); }
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
    // Render far spheres nearer while retaining their angular radius. Domain
    // positions and collision keep real metres; nearby bodies use exact scale.
    auto ProjectBody = [&](UStaticMeshComponent* Mesh, const FVector3d& Position, double Radius, double Expansion)
    {
        const FVector3d Relative = Position - S.PositionMetres - FVector3d(CameraPosition) / 100;
        const double Distance = Relative.Size();
        const double Factor = FMath::Min(1.0, 3e6 / FMath::Max(Distance, 1.0));
        Mesh->SetWorldLocation(CameraPosition + FVector(Relative * (100 * Factor)));
        Mesh->SetWorldScale3D(FVector(Radius * Factor * Expansion)); // imported sphere radius100cm
    };
    ProjectBody(Earth, FVector3d::ZeroVector, EarthRadius, 1);
    ProjectBody(Atmosphere, FVector3d::ZeroVector, EarthRadius, 1.012);
    ProjectBody(Sun, SunPosition, SunRadius, 1);
    const FVector3d SunDir = (SunPosition - S.PositionMetres).GetSafeNormal();
    const FLinearColor Direction(SunDir.X, SunDir.Y, SunDir.Z, 0);
    EarthDynamic->SetVectorParameterValue(TEXT("SunDirection"), Direction);
    AtmosphereDynamic->SetVectorParameterValue(TEXT("SunDirection"), Direction);
    Stars->SetWorldLocation(CameraPosition);
    const double Speed = S.VelocityMetresPerSecond.Size();
    Dust->SetVisibility(Speed > .05);
    for (int32 I = 0; I < DustPositions.Num(); ++I)
    {
        auto& P = DustPositions[I]; P -= S.VelocityMetresPerSecond * (bPaused ? 0.0 : FMath::Min<double>(DeltaSeconds, .1));
        for (int32 A = 0; A < 3; ++A) P[A] = FMath::Fmod(FMath::Fmod(P[A] + 2500, 5000) + 5000, 5000) - 2500;
        const FVector Location(P * 100);
        const double Fade = FMath::Clamp((P.Size() - 400) / 600, 0.0, 1.0);
        Dust->UpdateInstanceTransform(I, FTransform(FRotationMatrix::MakeFromZ(CameraPosition - Location).ToQuat(), Location, FVector(Fade * .7)), false, I == DustPositions.Num() - 1);
    }
}
void ASolarFlightGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    FrameMilliseconds += (DeltaSeconds * 1000 - FrameMilliseconds) * .08;
    if (!bReady) { if (!ProbeDirectory.IsEmpty()) TickProbe(); return; }
    Flight.Advance(DeltaSeconds);
    if (!Flight.GetError().IsEmpty()) Message = Flight.GetError();
    UpdateScene(DeltaSeconds);
    if (!ProbeDirectory.IsEmpty()) TickProbe();
}
void ASolarFlightHUD::DrawHUD()
{
    Super::DrawHUD();
    auto* M = Lab(this); if (!M || !Canvas) return;
    const float Scale = FMath::Clamp(Canvas->SizeX / 1280.0f, .8f, 1.5f);
    DrawRect(FLinearColor(.008f,.014f,.022f,.72f), 12*Scale, 12*Scale, 420*Scale, (M->bPaused || !M->Message.IsEmpty() || !M->Flight.GetState().ContactBodyId.IsEmpty() ? 240 : 146)*Scale);
    DrawRect(FLinearColor(.008f,.014f,.022f,.72f), 12*Scale, Canvas->SizeY-72*Scale, 775*Scale, 68*Scale);
    const FLinearColor Pale(.77f, .86f, .92f), Cyan(.35f, .77f, 1), White(.94f, .97f, 1);
    auto Text = [&](const FString& S, float X, float Y, FLinearColor C, float Factor = 1.0f)
    { DrawText(S, C, X * Scale, Y * Scale, GEngine->GetSmallFont(), Scale * Factor, false); };
    Text(TEXT("DAEDALUS  /  LETOVÁ ZKOUŠKA"), 25, 22, Cyan, 1.25f);
    if (!M->bReady) { Text(M->Message, 25, 65, FLinearColor::Red); return; }
    const auto& S = M->Flight.GetState(); const auto& C = M->Flight.GetConfig();
    const double Speed = S.VelocityMetresPerSecond.Size();
    Text(M->Profiles[M->ProfileIndex].Name, 25, 48, Pale);
    Text(FString::Printf(TEXT("RYCHLOST  %.1f m/s     TAH  %.0f %%"), Speed, S.Throttle * 100), 25, 77, White, 1.2f);
    Text(FString::Printf(TEXT("Otáčení %.1f °/s   Sklon %.1f °   Zastavení ≈ %.0f m"), S.YawRateDegrees, S.PitchDegrees, Speed * Speed / (2 * C.Braking)), 25, 106, Pale);
    Text(FString::Printf(TEXT("Země: nad povrchem %.1f km"), (S.PositionMetres.Size() - M->EarthRadius) / 1000), 25, 131, Pale);
    if (M->bPaused) Text(TEXT("POZASTAVENO — P pokračovat"), 25, 163, Cyan, 1.2f);
    if (!S.ContactBodyId.IsEmpty()) Text(TEXT("Povrchová ochrana: loď zastavena. Otoč se a odleť."), 25, 191, Cyan);
    if (!M->Message.IsEmpty()) Text(M->Message, 25, 219, Pale);
    const float Bottom = Canvas->SizeY / Scale;
    Text(TEXT("W/S sklon  ·  A/D zatáčení  ·  Q/E tah  ·  R stát / plný tah  ·  MEZERNÍK nebo X brzda"), 25, Bottom - 62, Pale);
    Text(TEXT("Pravé tlačítko + myš: rozhlížení  ·  Kolečko: kamera  ·  Home: za loď"), 25, Bottom - 42, Pale);
    Text(TEXT("1 manévrování / 2 místní přelet (ve stoje)  ·  Backspace: začátek  ·  P pauza  ·  Esc konec"), 25, Bottom - 22, Pale);
}

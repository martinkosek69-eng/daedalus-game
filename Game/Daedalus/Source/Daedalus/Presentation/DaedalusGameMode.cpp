#include "Presentation/DaedalusGameMode.h"
#include "World/DaedalusWorldSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Canvas.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
UDaedalusWorldSubsystem* DomainFor(const AActor* Actor)
{
    return Actor->GetGameInstance() ? Actor->GetGameInstance()->GetSubsystem<UDaedalusWorldSubsystem>() : nullptr;
}
}
ADaedalusPawn::ADaedalusPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(RootComponent);
    Camera->SetRelativeLocation(FVector(-50000, -50000, 32000));
    Camera->SetRelativeRotation((-Camera->GetRelativeLocation()).Rotation());
    Camera->FieldOfView = 75;
    AutoPossessPlayer = EAutoReceiveInput::Player0;
}
void ADaedalusPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (auto* D = DomainFor(this); D && D->bReady)
    {
        D->Simulation.SetFlightInput(FlightInput.GetClampedToMaxSize(1));
        if (!D->Simulation.GetSnapshot().PlayerLocationId.IsEmpty())
        {
            Camera->SetRelativeLocation(FVector(-2500, -2500, 1800));
            Camera->SetRelativeRotation((-Camera->GetRelativeLocation()).Rotation());
        }
        else
        {
            Camera->SetRelativeLocation(FVector(-50000, -50000, 32000));
            Camera->SetRelativeRotation((-Camera->GetRelativeLocation()).Rotation());
        }
    }
}
void ADaedalusPawn::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    Input->BindAxis(TEXT("Forward"), this, &ADaedalusPawn::Forward);
    Input->BindAxis(TEXT("Right"), this, &ADaedalusPawn::Right);
    Input->BindAxis(TEXT("Up"), this, &ADaedalusPawn::Up);
    Input->BindAction(TEXT("Fire"), IE_Pressed, this, &ADaedalusPawn::Fire);
    Input->BindAction(TEXT("Travel"), IE_Pressed, this, &ADaedalusPawn::Travel);
    Input->BindAction(TEXT("Transport"), IE_Pressed, this, &ADaedalusPawn::Transport);
    Input->BindAction(TEXT("Save"), IE_Pressed, this, &ADaedalusPawn::Save);
    Input->BindAction(TEXT("Load"), IE_Pressed, this, &ADaedalusPawn::Load);
    Input->BindAction(TEXT("Pause"), IE_Pressed, this, &ADaedalusPawn::Pause);
}
void ADaedalusPawn::Fire() { if (auto* D = DomainFor(this)) D->Fire(); }
void ADaedalusPawn::Travel() { if (auto* D = DomainFor(this)) D->Travel(); }
void ADaedalusPawn::Transport() { if (auto* D = DomainFor(this)) D->Transport(); }
void ADaedalusPawn::Save() { if (auto* D = DomainFor(this)) D->Save(); }
void ADaedalusPawn::Load() { if (auto* D = DomainFor(this)) D->Load(); }
void ADaedalusPawn::Pause()
{
    if (auto* D = DomainFor(this); D && D->bReady) D->Simulation.SetPaused(!D->Simulation.GetSnapshot().bPaused);
}

void ADaedalusHUD::DrawHUD()
{
    Super::DrawHUD();
    auto* D = DomainFor(this);
    if (!Canvas || !D) return;
    DrawRect(FLinearColor(0.015f, 0.025f, 0.045f, 0.95f), 12, 12, 760, 222);
    DrawText(TEXT("DAEDALUS | FOUNDATION | original diagnostic scene"), FLinearColor(0.4f, 0.8f, 1), 24, 22);
    if (!D->bReady) { DrawText(D->LastMessage, FLinearColor::Red, 24, 48); return; }
    const auto& State = D->Simulation.GetSnapshot();
    const auto& Player = State.Ships.FindChecked(State.PlayerShipId);
    DrawText(FString::Printf(TEXT("System: %s | Location: %s | Time: %.2f s %s"),
        *Player.SystemId, State.PlayerLocationId.IsEmpty() ? TEXT("aboard ship") : *State.PlayerLocationId,
        State.SimulationSeconds, State.bPaused ? TEXT("PAUSED") : TEXT("")), FLinearColor::White, 24, 47);
    DrawText(FString::Printf(TEXT("Hull %.1f | Shield %.1f | Energy %.1f | Speed %.1f m/s | Active ships %d"),
        Player.Hull, Player.Shield, Player.Energy, Player.VelocityMetresPerSecond.Length(), D->Simulation.GetActiveShipIds().Num()),
        FLinearColor::White, 24, 72);
    DrawText(TEXT("WASD: translation | Q/E: down/up | SPACE: fire | T: test travel | B: transport/return"),
        FLinearColor(0.8f, 0.85f, 0.9f), 24, 98);
    DrawText(TEXT("F5: save | F9: load | P: pause | close window: quit"), FLinearColor(0.8f, 0.85f, 0.9f), 24, 122);
    DrawText(D->LastMessage, FLinearColor(0.9f, 0.8f, 0.45f), 24, 149);
    DrawText(FString::Printf(TEXT("Pending time: %.3f s | %s"), State.PendingSeconds, *D->Simulation.GetLastAdvanceError()),
        State.PendingSeconds > 1 ? FLinearColor::Yellow : FLinearColor::White, 24, 174);
    for (const FString& Id : D->Simulation.GetActiveShipIds()) if (Id != State.PlayerShipId)
    {
        const auto& Target = State.Ships.FindChecked(Id);
        DrawText(FString::Printf(TEXT("Target: %s | Hull %.1f | Shield %.1f"), *Id, Target.Hull, Target.Shield), FLinearColor::White, 24, 198); break;
    }
}

ADaedalusGameMode::ADaedalusGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    DefaultPawnClass = ADaedalusPawn::StaticClass();
    HUDClass = ADaedalusHUD::StaticClass();
    // Hard references on the class default object make cook dependencies explicit.
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cruiser(TEXT("/Game/Ships/Foundation/SM_FoundationCruiser.SM_FoundationCruiser"));
    CubeMesh = Cube.Object; SphereMesh = Sphere.Object; CruiserMesh = Cruiser.Object;
}
void ADaedalusGameMode::BeginPlay()
{
    Super::BeginPlay();
    Domain = DomainFor(this);
    FString Smoke;
    if (FParse::Value(FCommandLine::Get(), TEXT("DaedalusSmoke="), Smoke)) { RunSmoke(Smoke); return; }
    FParse::Value(FCommandLine::Get(), TEXT("DaedalusVisualProbe="), ProbeDirectory);
    RebuildScene();
}
AActor* ADaedalusGameMode::Shape(const FVector& Position, const FVector& Scale, bool bSphere, bool bCruiser)
{
    AActor* Actor = GetWorld()->SpawnActor<AActor>();
    UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(Actor);
    Actor->SetRootComponent(Mesh); Actor->AddInstanceComponent(Mesh); Mesh->RegisterComponent();
    UStaticMesh* Asset = bCruiser ? CruiserMesh.Get() : bSphere ? SphereMesh.Get() : CubeMesh.Get();
    bVisualAssetsValid = bVisualAssetsValid && Asset != nullptr;
    Mesh->SetStaticMesh(Asset); Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Actor->SetActorLocation(Position); Actor->SetActorScale3D(Scale);
    return Actor;
}
void ADaedalusGameMode::RebuildScene()
{
    bVisualAssetsValid = CubeMesh && SphereMesh && CruiserMesh;
    for (auto& Pair : Ships) if (Pair.Value) Pair.Value->Destroy(); Ships.Reset();
    for (auto& Actor : Environment) if (Actor) Actor->Destroy(); Environment.Reset();
    if (!Domain || !Domain->bReady) return;
    const auto& State = Domain->Simulation.GetSnapshot();
    const auto& Player = State.Ships.FindChecked(State.PlayerShipId);
    ADirectionalLight* Light = GetWorld()->SpawnActor<ADirectionalLight>();
    Light->SetActorRotation(FRotator(-35, -20, 0)); Light->GetLightComponent()->SetIntensity(5); Environment.Add(Light);
    if (State.PlayerLocationId.IsEmpty())
    {
        for (const FString& Id : Domain->Simulation.GetActiveShipIds())
        {
            const auto& Ship = State.Ships.FindChecked(Id);
            const auto& Def = Domain->Simulation.GetCatalog().ShipDefinitions.FindChecked(Ship.DefinitionId);
            const bool OriginalMesh = Ship.DefinitionId == TEXT("fixture.cruiser");
            Ships.Add(Id, Shape(Daedalus::MetresToCentimetresRelative(Ship.PositionMetres, Player.PositionMetres),
                OriginalMesh ? FVector(Def.LengthMetres / 120.0) : FVector(Def.LengthMetres, Def.LengthMetres * .35, Def.LengthMetres * .15), false, OriginalMesh));
        }
        for (const auto& Body : Domain->Simulation.GetCatalog().Systems.FindChecked(Player.SystemId).Bodies)
            Environment.Add(Shape(Daedalus::MetresToCentimetresRelative(Body.PositionMetres, Player.PositionMetres), FVector(Body.RadiusMetres * 2), true));
    }
    else
    {
        Environment.Add(Shape(FVector(0, 0, -200), FVector(100, 100, 2), false));
        Environment.Add(Shape(FVector(1500, 0, 200), FVector(4, 4, 4), false));
        Environment.Add(Shape(FVector(-1500, 0, 200), FVector(4, 4, 4), false));
    }
    AppliedRevision = Domain->SceneRevision;
}
void ADaedalusGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!Domain || !Domain->bReady) return;
    const auto& Before = Domain->Simulation.GetSnapshot();
    const FString ActiveSystem = Before.Ships.FindChecked(Before.PlayerShipId).SystemId;
    if (!Before.bPaused)
        for (const auto& Initial : Domain->Simulation.GetCatalog().InitialShips)
        {
            const auto* Ship = Before.Ships.Find(Initial.Id);
            if (Initial.bPlayer || !Ship || !Ship->IsAlive() || Ship->SystemId != ActiveSystem) continue;
            const auto& Definition = Domain->Simulation.GetCatalog().ShipDefinitions.FindChecked(Ship->DefinitionId);
            FString PilotError;
            Domain->Simulation.SetShipFlightInput(Ship->Id, Daedalus::HoldPositionInput(*Ship, Definition, Initial.PositionMetres), PilotError);
        }
    Domain->Simulation.Advance(DeltaSeconds);
    bool Changed = AppliedRevision != Domain->SceneRevision;
    if (Domain->Simulation.GetSnapshot().PlayerLocationId.IsEmpty())
    {
        const TArray<FString> Active = Domain->Simulation.GetActiveShipIds();
        Changed |= Active.Num() != Ships.Num();
        for (const FString& Id : Active) Changed |= !Ships.Contains(Id);
    }
    if (Changed) RebuildScene();
    const auto& State = Domain->Simulation.GetSnapshot();
    const auto& Player = State.Ships.FindChecked(State.PlayerShipId);
    for (auto& Pair : Ships)
    {
        const auto& Ship = State.Ships.FindChecked(Pair.Key);
        Pair.Value->SetActorLocation(Daedalus::MetresToCentimetresRelative(Ship.PositionMetres, Player.PositionMetres));
        Pair.Value->SetActorHiddenInGame(!Ship.IsAlive());
    }
    if (State.PlayerLocationId.IsEmpty())
    {
        const auto& Bodies = Domain->Simulation.GetCatalog().Systems.FindChecked(Player.SystemId).Bodies;
        for (int32 I = 0; I < Bodies.Num(); ++I)
            if (Environment.IsValidIndex(I + 1)) Environment[I + 1]->SetActorLocation(Daedalus::MetresToCentimetresRelative(Bodies[I].PositionMetres, Player.PositionMetres));
    }
    if (!ProbeDirectory.IsEmpty()) TickVisualProbe();
}

void ADaedalusGameMode::RunSmoke(const FString& Mode)
{
    bool Ok = Domain && Domain->bReady;
    FString Error;
    if (Ok && Mode == TEXT("write"))
    {
        Ok = Domain->Simulation.FireAt(TEXT("ship.origin.target"), Error);
        Domain->Simulation.SetFlightInput(FVector(1, 0, 0)); Domain->Simulation.Advance(1.0);
        Domain->Simulation.SetFlightInput(FVector::ZeroVector);
        Ok = Ok && Domain->Simulation.Travel(TEXT("fixture.remote"), Error) &&
            Domain->Simulation.Transport(TEXT("fixture.station"), Error) && Domain->Save();
    }
    else if (Ok && Mode == TEXT("read"))
    {
        Ok = Domain->Load();
        const auto& State = Domain->Simulation.GetSnapshot();
        Ok = Ok && State.PlayerShipId == TEXT("ship.player") && State.PlayerLocationId == TEXT("fixture.station") &&
            State.Ships.FindChecked(State.PlayerShipId).SystemId == TEXT("fixture.remote") &&
            State.Ships.FindChecked(TEXT("ship.origin.target")).Shield < 50 && State.SimulationSeconds > 0.9;
        Ok = Ok && Domain->Simulation.ReturnToShip(Error) && Domain->Simulation.Travel(TEXT("fixture.origin"), Error);
    }
    else if (Ok) { Error = TEXT("Unknown smoke mode"); Ok = false; }
    RebuildScene();
    Ok = Ok && bVisualAssetsValid && Ships.Num() == (Domain->Simulation.GetSnapshot().PlayerLocationId.IsEmpty() ? Domain->Simulation.GetActiveShipIds().Num() : 0);
    FString Output;
    FParse::Value(FCommandLine::Get(), TEXT("DaedalusSmokeResult="), Output);
    if (Output.IsEmpty()) { Error = TEXT("Smoke requires explicit result path"); Ok = false; }
    if (!Output.IsEmpty())
    {
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Output), true);
        const FString Result = FString::Printf(TEXT("{\"passed\":%s,\"mode\":\"%s\",\"activeVisualShips\":%d}"), Ok ? TEXT("true") : TEXT("false"), *Mode, Ships.Num());
        Ok = FFileHelper::SaveStringToFile(Result, *Output) && Ok;
    }
    UE_LOG(LogTemp, Display, TEXT("DAEDALUS_SMOKE %s %s %s"), *Mode, Ok ? TEXT("PASS") : TEXT("FAIL"), *Error);
    FPlatformMisc::RequestExitWithStatus(false, Ok ? 0 : 1);
}

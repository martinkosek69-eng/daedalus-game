#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/Pawn.h"
#include "DaedalusGameMode.generated.h"

class UDaedalusWorldSubsystem;
class UCameraComponent;
class UStaticMeshComponent;
class UStaticMesh;

UCLASS()
class DAEDALUS_API ADaedalusPawn : public APawn
{
    GENERATED_BODY()
public:
    ADaedalusPawn();
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    UPROPERTY() TObjectPtr<UCameraComponent> Camera;
private:
    FVector FlightInput = FVector::ZeroVector;
    void Forward(float Value) { FlightInput.X = Value; }
    void Right(float Value) { FlightInput.Y = Value; }
    void Up(float Value) { FlightInput.Z = Value; }
    void Fire(); void Travel(); void Transport(); void Save(); void Load(); void Pause();
};

UCLASS()
class DAEDALUS_API ADaedalusHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
};

UCLASS()
class DAEDALUS_API ADaedalusGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    ADaedalusGameMode();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    int32 VisualShipCount() const { return Ships.Num(); }
private:
    UPROPERTY() TObjectPtr<UDaedalusWorldSubsystem> Domain;
    UPROPERTY() TMap<FString, TObjectPtr<AActor>> Ships;
    UPROPERTY() TArray<TObjectPtr<AActor>> Environment;
    UPROPERTY() TObjectPtr<UStaticMesh> CubeMesh;
    UPROPERTY() TObjectPtr<UStaticMesh> SphereMesh;
    UPROPERTY() TObjectPtr<UStaticMesh> CruiserMesh;
    bool bVisualAssetsValid = true;
    TMap<FString, FVector3d> GuardAnchors;
    uint64 AppliedRevision = MAX_uint64;
    void RebuildScene();
    AActor* Shape(const FVector& Position, const FVector& Scale, bool bSphere, bool bCruiser = false);
    void RunSmoke(const FString& Mode);
    FString ProbeDirectory;
    int32 ProbeFrame = 0;
    bool bProbePassed = true;
    double ProbePausedTime = 0;
    void TickVisualProbe();
};

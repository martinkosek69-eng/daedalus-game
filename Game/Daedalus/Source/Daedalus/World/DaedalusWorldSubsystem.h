#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DaedalusSimulation.h"
#include "DaedalusWorldSubsystem.generated.h"

UCLASS()
class DAEDALUS_API UDaedalusWorldSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    Daedalus::FSimulation Simulation;
    bool bReady = false;
    FString LastMessage;
    uint64 SceneRevision = 0;
    FString SaveDirectory() const;
    bool Save();
    bool Load();
    bool Travel();
    bool Transport();
    bool Fire();
};

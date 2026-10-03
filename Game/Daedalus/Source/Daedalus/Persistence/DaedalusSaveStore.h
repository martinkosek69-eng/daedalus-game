#pragma once
#include "CoreMinimal.h"
#include "DaedalusSimulation.h"

/** Two alternating verified generations; a failed write preserves the other slot. */
class DAEDALUS_API FDaedalusSaveStore
{
public:
    static bool Save(const FString& Directory, const Daedalus::FSimulation& Simulation, FString& Error);
    static bool Load(const FString& Directory, Daedalus::FSimulation& Simulation, FString& Error);
};

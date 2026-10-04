#pragma once

#include "CoreMinimal.h"

namespace Daedalus
{
/** Canonical body position is local metres; unknown radii are explicitly zero. */
struct DAEDALUSSIMULATION_API FNavigationBody
{
    FString Id, Name, ParentId;
    FVector3d PositionMetres = FVector3d::ZeroVector;
    double RadiusMetres = 0;
    bool bKnownRadius = false;
};

/** All systems in a configured plan use the same galaxy coordinate basis. */
struct DAEDALUSSIMULATION_API FNavigationSystem
{
    FString Id, Name;
    FVector3d GalaxyPositionLightYears = FVector3d::ZeroVector;
    TArray<FNavigationBody> Bodies;
};

struct DAEDALUSSIMULATION_API FNavigationMetrics
{
    bool bValid = false; // Distance/direction available; ETA flags are independent.
    bool bCurrentETA = false, bPlannedETA = false, bRequiredSpeed = false;
    double DistanceMetres = 0; // Centre-to-centre; never subtract a body's radius.
    FVector3d Direction = FVector3d::ZeroVector;
    double CurrentETASeconds = 0, PlannedETASeconds = 0;
    double RequiredSpeedMetresPerSecond = 0;
};

/** Singleplayer value planner, independent of Actors/visuals and flight physics.
 * Selection never moves the ship. Successful Configure copies an immutable
 * catalog (each system needs at least one body) and resets location/target to
 * its first system/body at local zero;
 * invalid configuration/commands retain the complete previously valid plan.
 * Same-system subtraction happens in local metres to preserve short distances.
 * Cross-system estimates use the common galaxy basis and local metre offsets.
 * Query has no mutation. Only finite positive speeds/desired time yield estimates;
 * unavailable/overflowing estimates have false flags and zero numeric fields. */
class DAEDALUSSIMULATION_API FNavigationPlan
{
public:
    static constexpr double LightYearMetres = 9460730472580800.0;
    static constexpr double MaxLocalCoordinateMetres = 1e15;
    static constexpr double MaxGalaxyCoordinateLightYears = 1e9;
    static constexpr int32 MaxSystems = 10000;
    static constexpr int32 MaxBodiesPerSystem = 10000;
    static constexpr int32 MaxTotalBodies = 100000;

    bool Configure(const TArray<FNavigationSystem>& InSystems, FString& Error);
    bool SetLocation(const FString& SystemId, const FVector3d& PositionMetres, FString& Error);
    bool SelectTarget(const FString& SystemId, const FString& BodyId, FString& Error);
    const FString& GetTargetSystemId() const { return TargetSystemId; }
    const FString& GetTargetBodyId() const { return TargetBodyId; }
    const FString& GetLocationSystemId() const { return LocationSystemId; }
    FNavigationMetrics Query(double CurrentSpeed, double PlannedSpeed, double DesiredSeconds) const;

private:
    TArray<FNavigationSystem> Systems;
    TMap<FString, int32> SystemIndices;
    TArray<TMap<FString, int32>> BodyIndices;
    FString LocationSystemId, TargetSystemId, TargetBodyId;
    FVector3d LocationMetres = FVector3d::ZeroVector;
};
}

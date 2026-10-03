#pragma once

#include "CoreMinimal.h"

namespace Daedalus
{
struct FGalaxyDefinition { FString Id, Name; };
struct FBodyDefinition
{
    FString Id, Name;
    double RadiusMetres = 0;
    FVector3d PositionMetres = FVector3d::ZeroVector;
};
struct FSystemDefinition
{
    FString Id, Name, GalaxyId;
    FVector3d PositionLy = FVector3d::ZeroVector;
    TArray<FBodyDefinition> Bodies;
};
struct FWeaponDefinition
{
    FString Id;
    double Damage = 0, RangeMetres = 0, EnergyCost = 0, CooldownSeconds = 0;
};
struct FShipDefinition
{
    FString Id, Name, WeaponId;
    double LengthMetres = 0, MaxSpeedMetresPerSecond = 0;
    double AccelerationMetresPerSecondSquared = 0;
    double HullCapacity = 0, ShieldCapacity = 0, EnergyCapacity = 0;
};
struct FLocationDefinition
{
    FString Id, Name, SystemId, Kind;
    FVector3d PositionMetres = FVector3d::ZeroVector;
};
struct FInitialShip
{
    FString Id, DefinitionId, SystemId;
    FVector3d PositionMetres = FVector3d::ZeroVector;
    bool bPlayer = false;
};

/** Text-only definitions: no Actor, UObject or asset ownership. Replace only after validation. */
struct DAEDALUSSIMULATION_API FCatalog
{
    TMap<FString, FGalaxyDefinition> Galaxies;
    TMap<FString, FSystemDefinition> Systems;
    TMap<FString, FWeaponDefinition> Weapons;
    TMap<FString, FShipDefinition> ShipDefinitions;
    TMap<FString, FLocationDefinition> Locations;
    TArray<FInitialShip> InitialShips;
    bool LoadJson(const FString& Json, FString& Error);
    bool Validate(FString& Error) const;
};

struct FShipState
{
    FString Id, DefinitionId, SystemId;
    FVector3d PositionMetres = FVector3d::ZeroVector;
    FVector3d VelocityMetresPerSecond = FVector3d::ZeroVector;
    double Hull = 0, Shield = 0, Energy = 0, WeaponCooldownSeconds = 0;
    bool IsAlive() const { return Hull > 0; }
};
struct FSnapshot
{
    TMap<FString, FShipState> Ships;
    FString PlayerShipId, PlayerLocationId;
    double SimulationSeconds = 0, PendingSeconds = 0;
    bool bPaused = false;
};

/** Single-player domain simulation. All methods called sequentially on its owner thread. */
class DAEDALUSSIMULATION_API FSimulation
{
public:
    static constexpr double FixedStepSeconds = 1.0 / 60.0;
    static constexpr int32 MaxStepsPerAdvance = 600;
    bool Initialize(const FCatalog& Catalog, FString& Error);
    int32 Advance(double RealSeconds);
    const FString& GetLastAdvanceError() const { return LastAdvanceError; }
    void SetFlightInput(const FVector3d& Input);
    bool SetShipFlightInput(const FString& InstanceId, const FVector3d& Input, FString& Error);
    bool SpawnShip(const FString& InstanceId, const FString& DefinitionId, const FString& SystemId, const FVector3d& PositionMetres, FString& Error);
    void SetPaused(bool bPaused) { State.bPaused = bPaused; }
    bool FireAt(const FString& TargetId, FString& Error);
    bool Travel(const FString& SystemId, FString& Error);
    bool Transport(const FString& LocationId, FString& Error);
    bool ReturnToShip(FString& Error);
    const FSnapshot& GetSnapshot() const { return State; }
    const FCatalog& GetCatalog() const { return Definitions; }
    TArray<FString> GetActiveShipIds() const;
    bool Serialize(FString& Json, FString& Error) const;
    bool Restore(const FString& Json, FString& Error);
private:
    FCatalog Definitions;
    FSnapshot State;
    FVector3d FlightInput = FVector3d::ZeroVector;
    TMap<FString, FVector3d> ShipFlightInputs;
    FString LastAdvanceError;
    bool bInitialized = false;
    void Step(double Seconds);
    bool CanCommand(FString& Error, bool bRequireAboard = true) const;
    bool ValidateSnapshot(const FSnapshot& Candidate, FString& Error) const;
};

DAEDALUSSIMULATION_API FVector3d MetresToCentimetresRelative(const FVector3d& PositionMetres, const FVector3d& OriginMetres);
DAEDALUSSIMULATION_API FVector3d CentimetresRelativeToMetres(const FVector3d& PositionCentimetres, const FVector3d& OriginMetres);
}

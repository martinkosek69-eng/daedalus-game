#include "DaedalusSimulation.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace Daedalus
{
namespace
{
constexpr int32 MaxEntries = 100000;
constexpr double MaxScalar = 1e12;
constexpr double MaxPosition = 1e15;
bool Fail(FString& Error, const FString& Message) { Error = Message; return false; }
bool NumberValid(double N, double Min = 0, double Max = MaxScalar)
{
    return FMath::IsFinite(N) && N >= Min && N <= Max;
}
bool VectorValid(const FVector3d& V, double Max = MaxPosition)
{
    return NumberValid(V.X, -Max, Max) && NumberValid(V.Y, -Max, Max) && NumberValid(V.Z, -Max, Max);
}
bool IdValid(const FString& Id)
{
    if (Id.IsEmpty() || Id.Len() > 128) return false;
    for (TCHAR C : Id)
    {
        if (!((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') || (C >= '0' && C <= '9') || C == '-' || C == '_' || C == '.')) return false;
    }
    return true;
}
bool Parse(const FString& Text, TSharedPtr<FJsonObject>& Root, FString& Error, int32 MaxBytes)
{
    if (Text.Len() > MaxBytes) return Fail(Error, TEXT("JSON exceeds supported size."));
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid()) return Fail(Error, TEXT("Malformed JSON object."));
    double Version = 0;
    const auto VersionField = Root->TryGetField(TEXT("version"));
    if (!VersionField.IsValid() || VersionField->Type != EJson::Number || !VersionField->TryGetNumber(Version) || Version != 1) return Fail(Error, TEXT("Unsupported JSON version; expected 1."));
    return true;
}
bool TextField(const TSharedPtr<FJsonObject>& O, const TCHAR* Key, FString& Value, FString& Error, bool bAllowEmpty = false)
{
    const auto Field = O->TryGetField(Key);
    if (!Field.IsValid() || Field->Type != EJson::String || !O->TryGetStringField(Key, Value) || Value.Len() > 1024 || (!bAllowEmpty && Value.IsEmpty()))
        return Fail(Error, FString::Printf(TEXT("Missing or invalid text field: %s."), Key));
    return true;
}
bool NumberField(const TSharedPtr<FJsonObject>& O, const TCHAR* Key, double& Value, FString& Error)
{
    const auto Field = O->TryGetField(Key);
    if (!Field.IsValid() || Field->Type != EJson::Number || !Field->TryGetNumber(Value) || !FMath::IsFinite(Value)) return Fail(Error, FString::Printf(TEXT("Missing or non-finite numeric field: %s."), Key));
    return true;
}
bool VectorField(const TSharedPtr<FJsonObject>& O, const TCHAR* Key, FVector3d& Value, FString& Error)
{
    const TArray<TSharedPtr<FJsonValue>>* A = nullptr;
    if (!O->TryGetArrayField(Key, A) || A->Num() != 3) return Fail(Error, FString::Printf(TEXT("Expected three coordinates: %s."), Key));
    double X, Y, Z;
    for (const auto& V : *A) if (!V.IsValid() || V->Type != EJson::Number) return Fail(Error, TEXT("Coordinates must be numbers."));
    if (!(*A)[0]->TryGetNumber(X) || !(*A)[1]->TryGetNumber(Y) || !(*A)[2]->TryGetNumber(Z)) return Fail(Error, TEXT("Coordinates must be numbers."));
    Value = FVector3d(X, Y, Z);
    if (!VectorValid(Value)) return Fail(Error, TEXT("Coordinates outside supported finite range."));
    return true;
}
bool ArrayField(const TSharedPtr<FJsonObject>& O, const TCHAR* Key, const TArray<TSharedPtr<FJsonValue>>*& A, FString& Error)
{
    if (!O->TryGetArrayField(Key, A) || A->Num() > MaxEntries) return Fail(Error, FString::Printf(TEXT("Missing or oversized array: %s."), Key));
    return true;
}
bool Object(const TSharedPtr<FJsonValue>& V, TSharedPtr<FJsonObject>& O, FString& Error)
{
    if (!V.IsValid() || V->Type != EJson::Object) return Fail(Error, TEXT("Array entry must be an object."));
    O = V->AsObject();
    return O.IsValid();
}
template<typename T> bool Insert(TMap<FString, T>& Map, T&& Item, FString& Error)
{
    if (!IdValid(Item.Id) || Map.Contains(Item.Id)) return Fail(Error, TEXT("Invalid or duplicate stable ID: ") + Item.Id);
    Map.Add(Item.Id, MoveTemp(Item));
    return true;
}
void PutVector(const TSharedRef<FJsonObject>& O, const TCHAR* Key, const FVector3d& V)
{
    TArray<TSharedPtr<FJsonValue>> A;
    A.Add(MakeShared<FJsonValueNumber>(V.X)); A.Add(MakeShared<FJsonValueNumber>(V.Y)); A.Add(MakeShared<FJsonValueNumber>(V.Z));
    O->SetArrayField(Key, A);
}
FShipState InitialState(const FInitialShip& I, const FCatalog& Catalog)
{
    const FShipDefinition& D = Catalog.ShipDefinitions.FindChecked(I.DefinitionId);
    FShipState S;
    S.Id = I.Id; S.DefinitionId = I.DefinitionId; S.SystemId = I.SystemId; S.PositionMetres = I.PositionMetres;
    S.Hull = D.HullCapacity; S.Shield = D.ShieldCapacity; S.Energy = D.EnergyCapacity;
    return S;
}
bool SegmentSphere(const FVector3d& Start, const FVector3d& End, const FVector3d& Center, double Radius, double& Fraction)
{
    const FVector3d Offset = Start - Center, Delta = End - Start;
    const double C = Offset.SizeSquared() - Radius * Radius;
    if (C <= 0) { Fraction = 0; return true; }
    const double A = Delta.SizeSquared();
    if (A <= 1e-20) return false;
    const double B = FVector3d::DotProduct(Offset, Delta);
    const double Discriminant = B * B - A * C;
    if (Discriminant < 0) return false;
    const double T = (-B - FMath::Sqrt(Discriminant)) / A;
    if (T < 0 || T > 1) return false;
    Fraction = T; return true;
}
}

bool FCatalog::LoadJson(const FString& Json, FString& Error)
{
    Error.Reset();
    TSharedPtr<FJsonObject> Root;
    if (!Parse(Json, Root, Error, 32 * 1024 * 1024)) return false;
    FCatalog Candidate;
    const TArray<TSharedPtr<FJsonValue>>* A = nullptr;
    if (!ArrayField(Root, TEXT("galaxies"), A, Error)) return false;
    for (const auto& V : *A)
    {
        TSharedPtr<FJsonObject> O; FGalaxyDefinition D;
        if (!Object(V, O, Error) || !TextField(O, TEXT("id"), D.Id, Error) || !TextField(O, TEXT("name"), D.Name, Error) || !Insert(Candidate.Galaxies, MoveTemp(D), Error)) return false;
    }
    if (!ArrayField(Root, TEXT("systems"), A, Error)) return false;
    for (const auto& V : *A)
    {
        TSharedPtr<FJsonObject> O; FSystemDefinition D;
        if (!Object(V, O, Error) || !TextField(O, TEXT("id"), D.Id, Error) || !TextField(O, TEXT("name"), D.Name, Error) || !TextField(O, TEXT("galaxyId"), D.GalaxyId, Error) || !VectorField(O, TEXT("positionLy"), D.PositionLy, Error)) return false;
        const TArray<TSharedPtr<FJsonValue>>* Bodies = nullptr;
        if (!ArrayField(O, TEXT("bodies"), Bodies, Error)) return false;
        for (const auto& B : *Bodies)
        {
            TSharedPtr<FJsonObject> BO; FBodyDefinition BD;
            if (!Object(B, BO, Error) || !TextField(BO, TEXT("id"), BD.Id, Error) || !TextField(BO, TEXT("name"), BD.Name, Error) || !NumberField(BO, TEXT("radiusMetres"), BD.RadiusMetres, Error) || !VectorField(BO, TEXT("positionMetres"), BD.PositionMetres, Error)) return false;
            D.Bodies.Add(MoveTemp(BD));
        }
        if (!Insert(Candidate.Systems, MoveTemp(D), Error)) return false;
    }
    if (!ArrayField(Root, TEXT("weapons"), A, Error)) return false;
    for (const auto& V : *A)
    {
        TSharedPtr<FJsonObject> O; FWeaponDefinition D;
        if (!Object(V, O, Error) || !TextField(O, TEXT("id"), D.Id, Error) || !NumberField(O, TEXT("damage"), D.Damage, Error) || !NumberField(O, TEXT("rangeMetres"), D.RangeMetres, Error) || !NumberField(O, TEXT("energyCost"), D.EnergyCost, Error) || !NumberField(O, TEXT("cooldownSeconds"), D.CooldownSeconds, Error) || !Insert(Candidate.Weapons, MoveTemp(D), Error)) return false;
    }
    if (!ArrayField(Root, TEXT("ships"), A, Error)) return false;
    for (const auto& V : *A)
    {
        TSharedPtr<FJsonObject> O; FShipDefinition D;
        if (!Object(V, O, Error) || !TextField(O, TEXT("id"), D.Id, Error) || !TextField(O, TEXT("name"), D.Name, Error) || !TextField(O, TEXT("weaponId"), D.WeaponId, Error)
            || !NumberField(O, TEXT("lengthMetres"), D.LengthMetres, Error) || !NumberField(O, TEXT("maxSpeedMetresPerSecond"), D.MaxSpeedMetresPerSecond, Error)
            || !NumberField(O, TEXT("accelerationMetresPerSecondSquared"), D.AccelerationMetresPerSecondSquared, Error)
            || !NumberField(O, TEXT("hullCapacity"), D.HullCapacity, Error) || !NumberField(O, TEXT("shieldCapacity"), D.ShieldCapacity, Error) || !NumberField(O, TEXT("energyCapacity"), D.EnergyCapacity, Error)
            || !Insert(Candidate.ShipDefinitions, MoveTemp(D), Error)) return false;
    }
    if (!ArrayField(Root, TEXT("locations"), A, Error)) return false;
    for (const auto& V : *A)
    {
        TSharedPtr<FJsonObject> O; FLocationDefinition D;
        if (!Object(V, O, Error) || !TextField(O, TEXT("id"), D.Id, Error) || !TextField(O, TEXT("name"), D.Name, Error) || !TextField(O, TEXT("systemId"), D.SystemId, Error) || !TextField(O, TEXT("kind"), D.Kind, Error) || !VectorField(O, TEXT("positionMetres"), D.PositionMetres, Error) || !Insert(Candidate.Locations, MoveTemp(D), Error)) return false;
    }
    if (!ArrayField(Root, TEXT("initialShips"), A, Error)) return false;
    for (const auto& V : *A)
    {
        TSharedPtr<FJsonObject> O; FInitialShip D;
        if (!Object(V, O, Error) || !TextField(O, TEXT("id"), D.Id, Error) || !TextField(O, TEXT("definitionId"), D.DefinitionId, Error) || !TextField(O, TEXT("systemId"), D.SystemId, Error) || !VectorField(O, TEXT("positionMetres"), D.PositionMetres, Error) || !O->TryGetBoolField(TEXT("player"), D.bPlayer)) return Fail(Error, TEXT("Invalid initial ship."));
        Candidate.InitialShips.Add(MoveTemp(D));
    }
    if (!Candidate.Validate(Error)) return false;
    *this = MoveTemp(Candidate);
    return true;
}

bool FCatalog::Validate(FString& Error) const
{
    Error.Reset();
    if (Galaxies.IsEmpty() || Systems.IsEmpty() || ShipDefinitions.IsEmpty() || InitialShips.IsEmpty()) return Fail(Error, TEXT("Catalog requires a galaxy, system, ship definition and player ship."));
    if (Galaxies.Num() > MaxEntries || Systems.Num() > MaxEntries || ShipDefinitions.Num() > MaxEntries || Weapons.Num() > MaxEntries || Locations.Num() > MaxEntries || InitialShips.Num() > MaxEntries) return Fail(Error, TEXT("Catalog exceeds supported entry count."));
    for (const auto& P : Galaxies) if (!IdValid(P.Key) || P.Key != P.Value.Id || P.Value.Name.IsEmpty()) return Fail(Error, TEXT("Invalid galaxy."));
    TSet<FString> BodyIds;
    for (const auto& P : Systems)
    {
        const FSystemDefinition& D = P.Value;
        if (!IdValid(P.Key) || P.Key != D.Id || D.Name.IsEmpty() || !Galaxies.Contains(D.GalaxyId) || !VectorValid(D.PositionLy, 1e9) || D.Bodies.Num() > MaxEntries) return Fail(Error, TEXT("Invalid system or galaxy reference: ") + P.Key);
        for (const FBodyDefinition& B : D.Bodies)
        {
            if (!IdValid(B.Id) || BodyIds.Contains(B.Id) || B.Name.IsEmpty() || !NumberValid(B.RadiusMetres, 0.000001) || !VectorValid(B.PositionMetres)) return Fail(Error, TEXT("Invalid or duplicate body: ") + B.Id);
            BodyIds.Add(B.Id);
        }
    }
    for (const auto& P : Weapons)
    {
        const FWeaponDefinition& D = P.Value;
        if (!IdValid(P.Key) || P.Key != D.Id || !NumberValid(D.Damage, 0.000001) || !NumberValid(D.RangeMetres, 0.000001) || !NumberValid(D.EnergyCost) || !NumberValid(D.CooldownSeconds, 0.000001)) return Fail(Error, TEXT("Invalid weapon: ") + P.Key);
    }
    for (const auto& P : ShipDefinitions)
    {
        const FShipDefinition& D = P.Value;
        if (!IdValid(P.Key) || P.Key != D.Id || D.Name.IsEmpty() || !Weapons.Contains(D.WeaponId)
            || !NumberValid(D.LengthMetres, 0.000001) || !NumberValid(D.MaxSpeedMetresPerSecond, 0.000001)
            || !NumberValid(D.AccelerationMetresPerSecondSquared, 0.000001) || !NumberValid(D.HullCapacity, 0.000001) || !NumberValid(D.ShieldCapacity) || !NumberValid(D.EnergyCapacity)) return Fail(Error, TEXT("Invalid ship definition or weapon reference: ") + P.Key);
    }
    for (const auto& P : Locations)
    {
        const FLocationDefinition& D = P.Value;
        if (!IdValid(P.Key) || P.Key != D.Id || D.Name.IsEmpty() || !Systems.Contains(D.SystemId) || !VectorValid(D.PositionMetres) || (D.Kind != TEXT("surface") && D.Kind != TEXT("interior"))) return Fail(Error, TEXT("Invalid location: ") + P.Key);
    }
    TSet<FString> ShipIds;
    int32 Players = 0;
    for (const FInitialShip& S : InitialShips)
    {
        if (!IdValid(S.Id) || ShipIds.Contains(S.Id) || !ShipDefinitions.Contains(S.DefinitionId) || !Systems.Contains(S.SystemId) || !VectorValid(S.PositionMetres)) return Fail(Error, TEXT("Invalid initial ship or reference: ") + S.Id);
        ShipIds.Add(S.Id); Players += S.bPlayer ? 1 : 0;
    }
    if (Players != 1) return Fail(Error, TEXT("Exactly one player ship is required."));
    return true;
}

FVector3d MetresToCentimetresRelative(const FVector3d& PositionMetres, const FVector3d& OriginMetres) { return (PositionMetres - OriginMetres) * 100.0; }
FVector3d CentimetresRelativeToMetres(const FVector3d& PositionCentimetres, const FVector3d& OriginMetres) { return OriginMetres + PositionCentimetres / 100.0; }

bool FSimulation::Initialize(const FCatalog& Catalog, FString& Error)
{
    if (!Catalog.Validate(Error)) return false;
    FSnapshot Initial;
    for (const FInitialShip& I : Catalog.InitialShips)
    {
        FShipState S = InitialState(I, Catalog);
        Initial.Ships.Add(S.Id, S);
        if (I.bPlayer) Initial.PlayerShipId = I.Id;
    }
    Definitions = Catalog; State = MoveTemp(Initial); FlightInput = FVector3d::ZeroVector; ShipFlightInputs.Reset(); LastAdvanceError.Reset(); bInitialized = true;
    return true;
}

bool FSimulation::CanCommand(FString& Error, bool bRequireAboard) const
{
    Error.Reset();
    const FShipState* Player = State.Ships.Find(State.PlayerShipId);
    if (!bInitialized || !Player) return Fail(Error, TEXT("Simulation is not initialized."));
    if (State.bPaused) return Fail(Error, TEXT("Simulation is paused."));
    if (!Player->IsAlive()) return Fail(Error, TEXT("Player ship is destroyed."));
    if (bRequireAboard && !State.PlayerLocationId.IsEmpty()) return Fail(Error, TEXT("Player must be aboard the ship."));
    return true;
}

void FSimulation::SetFlightInput(const FVector3d& Input)
{
    FlightInput = VectorValid(Input, MaxScalar) ? Input.GetClampedToMaxSize(1.0) : FVector3d::ZeroVector;
}

bool FSimulation::SetShipFlightInput(const FString& InstanceId, const FVector3d& Input, FString& Error)
{
    if (!CanCommand(Error, false)) return false;
    const FShipState* Ship = State.Ships.Find(InstanceId);
    if (!Ship || !Ship->IsAlive() || !VectorValid(Input, MaxScalar)) return Fail(Error, TEXT("Flight input requires a live known ship and finite vector."));
    if (InstanceId == State.PlayerShipId)
    {
        if (!CanCommand(Error)) return false;
        SetFlightInput(Input);
    }
    else ShipFlightInputs.Add(InstanceId, Input.GetClampedToMaxSize(1.0));
    return true;
}

bool FSimulation::SpawnShip(const FString& InstanceId, const FString& DefinitionId, const FString& SystemId, const FVector3d& PositionMetres, FString& Error)
{
    Error.Reset();
    if (!bInitialized || !IdValid(InstanceId) || State.Ships.Contains(InstanceId) || State.Ships.Num() >= MaxEntries
        || !Definitions.ShipDefinitions.Contains(DefinitionId) || !Definitions.Systems.Contains(SystemId) || !VectorValid(PositionMetres)) return Fail(Error, TEXT("Spawn requires a unique stable ID, known definition/system and finite position."));
    FInitialShip I; I.Id = InstanceId; I.DefinitionId = DefinitionId; I.SystemId = SystemId; I.PositionMetres = PositionMetres;
    State.Ships.Add(InstanceId, InitialState(I, Definitions));
    return true;
}

int32 FSimulation::Advance(double RealSeconds)
{
    LastAdvanceError.Reset();
    if (!bInitialized) { LastAdvanceError = TEXT("Simulation is not initialized."); return 0; }
    if (!NumberValid(RealSeconds, 0, 86400)) { LastAdvanceError = TEXT("Delta must be finite and between zero and one day."); return 0; }
    if (State.bPaused) return 0;
    if (!NumberValid(State.PendingSeconds + RealSeconds, 0, 1e9)) { LastAdvanceError = TEXT("Pending elapsed time exceeds supported range."); return 0; }
    State.PendingSeconds += RealSeconds;
    const int32 Steps = static_cast<int32>(FMath::Min<double>(FMath::FloorToDouble((State.PendingSeconds + 1e-10) / FixedStepSeconds), MaxStepsPerAdvance));
    for (int32 I = 0; I < Steps; ++I) Step(FixedStepSeconds);
    State.PendingSeconds = FMath::Max(0.0, State.PendingSeconds - Steps * FixedStepSeconds);
    return Steps;
}

void FSimulation::Step(double Seconds)
{
    const FString ActiveSystem = State.Ships.FindChecked(State.PlayerShipId).SystemId;
    for (auto& P : State.Ships)
    {
        FShipState& S = P.Value;
        if (!S.IsAlive() || S.SystemId != ActiveSystem) continue;
        const FShipDefinition& D = Definitions.ShipDefinitions.FindChecked(S.DefinitionId);
        FVector3d Input = FVector3d::ZeroVector;
        if (S.Id == State.PlayerShipId && State.PlayerLocationId.IsEmpty()) Input = FlightInput;
        else if (S.Id != State.PlayerShipId) if (const FVector3d* Control = ShipFlightInputs.Find(S.Id)) Input = *Control;
        S.VelocityMetresPerSecond = (S.VelocityMetresPerSecond + Input * D.AccelerationMetresPerSecondSquared * Seconds).GetClampedToMaxSize(D.MaxSpeedMetresPerSecond);
        const FVector3d Next = S.PositionMetres + S.VelocityMetresPerSecond * Seconds;
        double ContactFraction = 1; bool bContact = false;
        for (const FBodyDefinition& B : Definitions.Systems.FindChecked(S.SystemId).Bodies)
        {
            double T = 0;
            if (SegmentSphere(S.PositionMetres, Next, B.PositionMetres, B.RadiusMetres + D.LengthMetres * 0.5, T)) { ContactFraction = FMath::Min(ContactFraction, T); bContact = true; }
        }
        if (bContact)
        {
            const double Length = (Next - S.PositionMetres).Size();
            const double SafeFraction = FMath::Max(0.0, ContactFraction - (Length > 0 ? 0.01 / Length : 0));
            S.PositionMetres += (Next - S.PositionMetres) * SafeFraction;
            S.VelocityMetresPerSecond = FVector3d::ZeroVector;
        }
        else if (VectorValid(Next)) S.PositionMetres = Next;
        else S.VelocityMetresPerSecond = FVector3d::ZeroVector;
        S.WeaponCooldownSeconds = FMath::Max(0.0, S.WeaponCooldownSeconds - Seconds);
        S.Energy = FMath::Min(D.EnergyCapacity, S.Energy + D.EnergyCapacity * 0.01 * Seconds);
        S.Shield = FMath::Min(D.ShieldCapacity, S.Shield + D.ShieldCapacity * 0.01 * Seconds);
    }
    State.SimulationSeconds += Seconds;
}

bool FSimulation::FireAt(const FString& TargetId, FString& Error)
{
    if (!CanCommand(Error)) return false;
    FShipState& Player = State.Ships.FindChecked(State.PlayerShipId);
    FShipState* Target = State.Ships.Find(TargetId);
    if (!Target || TargetId == Player.Id || Target->SystemId != Player.SystemId || !Target->IsAlive()) return Fail(Error, TEXT("Target must be another live ship in the active system."));
    const FWeaponDefinition& W = Definitions.Weapons.FindChecked(Definitions.ShipDefinitions.FindChecked(Player.DefinitionId).WeaponId);
    if (FVector3d::Dist(Player.PositionMetres, Target->PositionMetres) > W.RangeMetres) return Fail(Error, TEXT("Target is outside weapon range."));
    if (Player.WeaponCooldownSeconds > 1e-10) return Fail(Error, TEXT("Weapon is cooling down."));
    if (Player.Energy < W.EnergyCost) return Fail(Error, TEXT("Insufficient weapon energy."));
    for (const FBodyDefinition& B : Definitions.Systems.FindChecked(Player.SystemId).Bodies)
    {
        double Fraction = 0;
        if (SegmentSphere(Player.PositionMetres, Target->PositionMetres, B.PositionMetres, B.RadiusMetres, Fraction)) return Fail(Error, TEXT("Weapon line of sight is blocked by a celestial body."));
    }
    Player.Energy -= W.EnergyCost; Player.WeaponCooldownSeconds = W.CooldownSeconds;
    const double Absorbed = FMath::Min(Target->Shield, W.Damage);
    Target->Shield -= Absorbed; Target->Hull = FMath::Max(0.0, Target->Hull - (W.Damage - Absorbed));
    if (!Target->IsAlive()) Target->VelocityMetresPerSecond = FVector3d::ZeroVector;
    return true;
}

bool FSimulation::Travel(const FString& SystemId, FString& Error)
{
    if (!CanCommand(Error)) return false;
    FShipState& Player = State.Ships.FindChecked(State.PlayerShipId);
    if (!Definitions.Systems.Contains(SystemId) || Player.SystemId == SystemId) return Fail(Error, TEXT("Destination must be another known system."));
    Player.SystemId = SystemId; Player.PositionMetres = FVector3d::ZeroVector; Player.VelocityMetresPerSecond = FVector3d::ZeroVector; FlightInput = FVector3d::ZeroVector; ShipFlightInputs.Reset();
    return true;
}

bool FSimulation::Transport(const FString& LocationId, FString& Error)
{
    if (!CanCommand(Error)) return false;
    const FShipState& Player = State.Ships.FindChecked(State.PlayerShipId);
    const FLocationDefinition* Location = Definitions.Locations.Find(LocationId);
    if (!Location || Location->SystemId != Player.SystemId || FVector3d::Dist(Player.PositionMetres, Location->PositionMetres) > 50000.0) return Fail(Error, TEXT("Transport destination must be in the same system within 50 km."));
    State.PlayerLocationId = LocationId; FlightInput = FVector3d::ZeroVector;
    return true;
}

bool FSimulation::ReturnToShip(FString& Error)
{
    if (!CanCommand(Error, false)) return false;
    if (State.PlayerLocationId.IsEmpty()) return Fail(Error, TEXT("Player is already aboard."));
    State.PlayerLocationId.Reset(); FlightInput = FVector3d::ZeroVector;
    return true;
}

TArray<FString> FSimulation::GetActiveShipIds() const
{
    TArray<FString> Result;
    const FShipState* Player = State.Ships.Find(State.PlayerShipId);
    if (Player) for (const auto& P : State.Ships) if (P.Value.SystemId == Player->SystemId) Result.Add(P.Key);
    Result.Sort(); return Result;
}

bool FSimulation::ValidateSnapshot(const FSnapshot& C, FString& Error) const
{
    if (!bInitialized) return Fail(Error, TEXT("Load a validated catalog before restoring a save."));
    if (!NumberValid(C.SimulationSeconds, 0, 1e12) || !NumberValid(C.PendingSeconds, 0, 1e9) || C.Ships.IsEmpty() || C.Ships.Num() > MaxEntries || !C.Ships.Contains(C.PlayerShipId)) return Fail(Error, TEXT("Invalid clock, ship count or player reference."));
    for (const FInitialShip& I : Definitions.InitialShips)
    {
        if (I.bPlayer && C.PlayerShipId != I.Id) return Fail(Error, TEXT("Saved player identity does not match this catalog."));
    }
    for (const auto& P : C.Ships)
    {
        const FShipState& S = P.Value;
        const FShipDefinition* D = Definitions.ShipDefinitions.Find(S.DefinitionId);
        if (!IdValid(P.Key) || P.Key != S.Id || !D || !Definitions.Systems.Contains(S.SystemId) || !VectorValid(S.PositionMetres) || !VectorValid(S.VelocityMetresPerSecond, MaxScalar)) return Fail(Error, TEXT("Invalid saved ship reference or position: ") + P.Key);
        const FWeaponDefinition& W = Definitions.Weapons.FindChecked(D->WeaponId);
        if (!NumberValid(S.Hull, 0, D->HullCapacity) || !NumberValid(S.Shield, 0, D->ShieldCapacity) || !NumberValid(S.Energy, 0, D->EnergyCapacity)
            || !NumberValid(S.WeaponCooldownSeconds, 0, W.CooldownSeconds) || S.VelocityMetresPerSecond.Size() > D->MaxSpeedMetresPerSecond + 1e-6
            || (!S.IsAlive() && !S.VelocityMetresPerSecond.IsNearlyZero())) return Fail(Error, TEXT("Saved ship values violate capacities: ") + P.Key);
    }
    if (!C.PlayerLocationId.IsEmpty())
    {
        const FLocationDefinition* L = Definitions.Locations.Find(C.PlayerLocationId);
        if (!L || L->SystemId != C.Ships.FindChecked(C.PlayerShipId).SystemId) return Fail(Error, TEXT("Invalid saved player location."));
    }
    return true;
}

bool FSimulation::Serialize(FString& Json, FString& Error) const
{
    Error.Reset();
    if (!ValidateSnapshot(State, Error)) return false;
    const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetNumberField(TEXT("version"), 1); Root->SetNumberField(TEXT("clock"), State.SimulationSeconds); Root->SetNumberField(TEXT("remainder"), State.PendingSeconds);
    Root->SetBoolField(TEXT("paused"), State.bPaused); Root->SetStringField(TEXT("playerId"), State.PlayerShipId); Root->SetStringField(TEXT("locationId"), State.PlayerLocationId);
    TArray<FString> Ids; State.Ships.GetKeys(Ids); Ids.Sort();
    TArray<TSharedPtr<FJsonValue>> Ships;
    for (const FString& Id : Ids)
    {
        const FShipState& S = State.Ships.FindChecked(Id);
        const TSharedRef<FJsonObject> O = MakeShared<FJsonObject>();
        O->SetStringField(TEXT("id"), S.Id); O->SetStringField(TEXT("definitionId"), S.DefinitionId); O->SetStringField(TEXT("systemId"), S.SystemId);
        PutVector(O, TEXT("positionMetres"), S.PositionMetres); PutVector(O, TEXT("velocityMetresPerSecond"), S.VelocityMetresPerSecond);
        O->SetNumberField(TEXT("hull"), S.Hull); O->SetNumberField(TEXT("shield"), S.Shield); O->SetNumberField(TEXT("energy"), S.Energy); O->SetNumberField(TEXT("weaponCooldownSeconds"), S.WeaponCooldownSeconds);
        Ships.Add(MakeShared<FJsonValueObject>(O));
    }
    Root->SetArrayField(TEXT("ships"), Ships);
    FString Candidate;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Candidate);
    if (!FJsonSerializer::Serialize(Root, Writer)) return Fail(Error, TEXT("Save serialization failed."));
    Json = MoveTemp(Candidate); return true;
}

bool FSimulation::Restore(const FString& Json, FString& Error)
{
    Error.Reset();
    TSharedPtr<FJsonObject> Root;
    if (!Parse(Json, Root, Error, 16 * 1024 * 1024)) return false;
    FSnapshot Candidate;
    if (!NumberField(Root, TEXT("clock"), Candidate.SimulationSeconds, Error) || !NumberField(Root, TEXT("remainder"), Candidate.PendingSeconds, Error)
        || !Root->TryGetBoolField(TEXT("paused"), Candidate.bPaused) || !TextField(Root, TEXT("playerId"), Candidate.PlayerShipId, Error) || !TextField(Root, TEXT("locationId"), Candidate.PlayerLocationId, Error, true)) return Fail(Error, TEXT("Invalid saved clock or player fields."));
    const TArray<TSharedPtr<FJsonValue>>* A = nullptr;
    if (!ArrayField(Root, TEXT("ships"), A, Error)) return false;
    for (const auto& V : *A)
    {
        TSharedPtr<FJsonObject> O; FShipState S;
        if (!Object(V, O, Error) || !TextField(O, TEXT("id"), S.Id, Error) || !TextField(O, TEXT("definitionId"), S.DefinitionId, Error) || !TextField(O, TEXT("systemId"), S.SystemId, Error)
            || !VectorField(O, TEXT("positionMetres"), S.PositionMetres, Error) || !VectorField(O, TEXT("velocityMetresPerSecond"), S.VelocityMetresPerSecond, Error)
            || !NumberField(O, TEXT("hull"), S.Hull, Error) || !NumberField(O, TEXT("shield"), S.Shield, Error) || !NumberField(O, TEXT("energy"), S.Energy, Error)
            || !NumberField(O, TEXT("weaponCooldownSeconds"), S.WeaponCooldownSeconds, Error) || !Insert(Candidate.Ships, MoveTemp(S), Error)) return false;
    }
    if (!ValidateSnapshot(Candidate, Error)) return false;
    for (const FInitialShip& I : Definitions.InitialShips)
        if (!Candidate.Ships.Contains(I.Id)) Candidate.Ships.Add(I.Id, InitialState(I, Definitions));
    if (!ValidateSnapshot(Candidate, Error)) return false;
    State = MoveTemp(Candidate); FlightInput = FVector3d::ZeroVector; ShipFlightInputs.Reset(); LastAdvanceError.Reset();
    return true;
}
}

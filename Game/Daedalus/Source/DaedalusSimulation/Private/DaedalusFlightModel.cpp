#include "DaedalusFlightModel.h"

#include <limits>

namespace Daedalus
{
namespace
{
constexpr double CoordinateLimit = 1e15;
constexpr double ClockLimit = 1e12;
constexpr double PendingLimit = 1e6;

bool InRange(double Value, double Min, double Max)
{
    return FMath::IsFinite(Value) && Value >= Min && Value <= Max;
}

bool ValidPosition(const FVector3d& Value)
{
    return InRange(Value.X, -CoordinateLimit, CoordinateLimit)
        && InRange(Value.Y, -CoordinateLimit, CoordinateLimit)
        && InRange(Value.Z, -CoordinateLimit, CoordinateLimit);
}

double Approach(double Value, double Target, double Change)
{
    return Value + FMath::Clamp(Target - Value, -Change, Change);
}

double SignedSpeed(const FFlightState& State)
{
    const double Speed = State.VelocityMetresPerSecond.Size();
    return FVector3d::DotProduct(State.VelocityMetresPerSecond, State.Forward()) < 0 ? -Speed : Speed;
}

// A small conservative gap also covers floating-point rounding when a local
// flight takes place far from the system origin (e.g. one astronomical unit).
double ContactGap(const FVector3d& A, const FVector3d& B)
{
    const double Largest = FMath::Max(A.GetAbsMax(), B.GetAbsMax());
    return FMath::Max(0.01, Largest * std::numeric_limits<double>::epsilon() * 16.0);
}

bool ValidState(const FFlightConfig& Config, const FFlightState& State, double SpeedLimit = -1)
{
    return ValidPosition(State.PositionMetres)
        && ValidPosition(State.VelocityMetresPerSecond)
        && State.VelocityMetresPerSecond.Size() <= (SpeedLimit >= 0 ? SpeedLimit : Config.MaxSpeed) + 1e-6
        && InRange(State.YawDegrees, -1e6, 1e6)
        && InRange(State.PitchDegrees, -Config.PitchLimitDegrees, Config.PitchLimitDegrees)
        && InRange(State.BankDegrees, -Config.BankDegrees, Config.BankDegrees)
        && InRange(State.YawRateDegrees, -Config.TurnRateDegrees, Config.TurnRateDegrees)
        && InRange(State.PitchRateDegrees, -Config.TurnRateDegrees, Config.TurnRateDegrees)
        && InRange(State.Throttle, -0.25, 1)
        && InRange(State.SimulationSeconds, 0, ClockLimit);
}

bool FindContact(const FVector3d& Start, const FVector3d& Delta,
    const FFlightBody& Body, double ShipRadius, double& Fraction)
{
    const double Length = Delta.Size();
    if (Length <= 1e-12) return false;
    const FVector3d Direction = Delta / Length;
    const FVector3d Offset = Start - Body.PositionMetres;
    const double Radius = Body.RadiusMetres + ShipRadius;
    const double Along = FVector3d::DotProduct(Offset, Direction);
    if (Along >= 0) return false; // Already moving away, including from contact.
    const FVector3d Perpendicular = Offset - Direction * Along;
    const double SideSquared = Perpendicular.SizeSquared();
    if (SideSquared > Radius * Radius) return false;
    const double Entry = -Along - FMath::Sqrt(FMath::Max(0.0, Radius * Radius - SideSquared));
    if (Entry > Length) return false;
    const double Gap = ContactGap(Start, Body.PositionMetres);
    Fraction = FMath::Clamp((Entry - Gap) / Length, 0.0, 1.0);
    return true;
}
}

bool FFlightConfig::Validate(FString& Error) const
{
    if (!InRange(MaxSpeed, 0.01, 299792458.0) || MaxSpeed >= 299792458.0
        || !InRange(Acceleration, 0.001, 1e10)
        || !InRange(Braking, 0.001, 1e10)
        || !InRange(CoastDeceleration, 0.001, 1e10)
        || !InRange(LateralAcceleration, 0.001, 1e10)
        || !InRange(TurnRateDegrees, 0.001, 360)
        || !InRange(AngularAccelerationDegrees, 0.001, 3600)
        || !InRange(PitchLimitDegrees, 1, 89)
        || !InRange(BankDegrees, 0, 80)
        || !InRange(ShipRadiusMetres, 0.01, 1e9))
    {
        Error = TEXT("Flight configuration contains unsupported, non-finite or non-positive values.");
        return false;
    }
    Error.Reset();
    return true;
}

FQuat4d FFlightState::Attitude() const
{
    return FRotator3d(PitchDegrees, YawDegrees, BankDegrees).Quaternion();
}

FVector3d FFlightState::Forward() const
{
    return FRotator3d(PitchDegrees, YawDegrees, 0).Vector();
}

bool FFlightModel::Fail(const FString& Message, FString& Error)
{
    LastError = Message;
    Error = Message;
    return false;
}

bool FFlightModel::Initialize(const FFlightConfig& InConfig, const FFlightState& InitialState,
    const TArray<FFlightBody>& InBodies, FString& Error)
{
    FString ValidationError;
    if (!InConfig.Validate(ValidationError)) return Fail(ValidationError, Error);
    if (!ValidState(InConfig, InitialState))
        return Fail(TEXT("Initial flight state is outside supported finite bounds."), Error);
    if (InBodies.Num() > 10000)
        return Fail(TEXT("Flight body limit exceeded."), Error);
    TSet<FString> Ids;
    for (const FFlightBody& Body : InBodies)
    {
        if (Body.Id.IsEmpty() || Body.Id.Len() > 256 || Ids.Contains(Body.Id)
            || !ValidPosition(Body.PositionMetres) || !InRange(Body.RadiusMetres, 0.01, 1e12))
            return Fail(TEXT("Flight bodies need unique IDs, finite positions and positive bounded radii."), Error);
        Ids.Add(Body.Id);
        if ((InitialState.PositionMetres - Body.PositionMetres).Size()
            < Body.RadiusMetres + InConfig.ShipRadiusMetres)
            return Fail(TEXT("Initial ship overlaps a body."), Error);
    }
    if (!InitialState.ContactBodyId.IsEmpty() && !Ids.Contains(InitialState.ContactBodyId))
        return Fail(TEXT("Initial contact refers to an unknown body."), Error);

    // Copy everything before committing, including when a caller passes getters.
    const FFlightConfig NewConfig = InConfig;
    FFlightState NewState = InitialState;
    TArray<FFlightBody> NewBodies = InBodies;
    NewState.YawDegrees = FRotator3d::NormalizeAxis(NewState.YawDegrees);
    NewState.VelocityMetresPerSecond = NewState.Forward() * SignedSpeed(NewState);
    NewState.ContactBodyId.Reset();
    Config = NewConfig;
    State = MoveTemp(NewState);
    Bodies = MoveTemp(NewBodies);
    Input = FFlightInput();
    PendingSeconds = 0;
    TransitionBraking = 0;
    TransitionCoastDeceleration = 0;
    bPaused = false;
    bInitialized = true;
    LastError.Reset();
    Error.Reset();
    return true;
}

bool FFlightModel::SetConfig(const FFlightConfig& InConfig, FString& Error)
{
    if (!bInitialized) return Fail(TEXT("Flight model is not initialized."), Error);
    FString ValidationError;
    if (!InConfig.Validate(ValidationError)) return Fail(ValidationError, Error);
    const double CurrentSpeed = State.VelocityMetresPerSecond.Size();
    if (!ValidState(InConfig, State, FMath::Max(InConfig.MaxSpeed, CurrentSpeed)))
        return Fail(TEXT("New flight limits are incompatible with the current attitude or state."), Error);
    for (const FFlightBody& Body : Bodies)
    {
        if ((State.PositionMetres - Body.PositionMetres).Size() < Body.RadiusMetres + InConfig.ShipRadiusMetres)
            return Fail(TEXT("New ship radius would overlap a body."), Error);
    }

    // Retain authority across repeated downgrades until the excess speed is gone.
    // Never clamp velocity here: commands must not teleport or suddenly stop a ship.
    if (CurrentSpeed > InConfig.MaxSpeed)
    {
        TransitionBraking = FMath::Max(TransitionBraking, Config.Braking);
        TransitionCoastDeceleration = FMath::Max(TransitionCoastDeceleration, Config.CoastDeceleration);
    }
    else
    {
        TransitionBraking = 0;
        TransitionCoastDeceleration = 0;
    }
    Config = InConfig;
    LastError.Reset();
    Error.Reset();
    return true;
}

bool FFlightModel::SetThrottle(double Value, FString& Error)
{
    if (!bInitialized) return Fail(TEXT("Flight model is not initialized."), Error);
    if (!InRange(Value, -0.25, 1)) return Fail(TEXT("Throttle must be finite and between -0.25 and 1."), Error);
    State.Throttle = Input.bBrake ? 0 : Value;
    LastError.Reset();
    Error.Reset();
    return true;
}

bool FFlightModel::SetInput(const FFlightInput& Value, FString& Error)
{
    if (!bInitialized) return Fail(TEXT("Flight model is not initialized."), Error);
    if (!InRange(Value.Yaw, -1, 1) || !InRange(Value.Pitch, -1, 1))
        return Fail(TEXT("Flight axes must be finite and between -1 and 1."), Error);
    Input = Value;
    if (Input.bBrake) State.Throttle = 0;
    LastError.Reset();
    Error.Reset();
    return true;
}

int32 FFlightModel::Advance(double RealSeconds)
{
    if (!bInitialized)
    {
        LastError = TEXT("Flight model is not initialized.");
        return 0;
    }
    if (!InRange(RealSeconds, 0, PendingLimit)
        || (!bPaused && PendingSeconds + RealSeconds > PendingLimit))
    {
        LastError = TEXT("Flight time must be finite, non-negative and within the pending-time limit.");
        return 0;
    }
    LastError.Reset();
    if (bPaused) return 0;
    PendingSeconds += RealSeconds;
    int32 Steps = 0;
    while (Steps < MaxStepsPerAdvance && PendingSeconds + 1e-10 >= FixedStepSeconds)
    {
        if (!Step()) break;
        PendingSeconds = FMath::Max(0.0, PendingSeconds - FixedStepSeconds);
        ++Steps;
    }
    return Steps;
}

bool FFlightModel::Step()
{
    FFlightState Next = State;
    const double Dt = FixedStepSeconds;
    const double TurnAuthority = State.Throttle < 0 ? 1.0 : 0.5 + 0.5 * FMath::Clamp(State.Throttle / 0.25, 0.0, 1.0);
    const double RateLimit = Config.TurnRateDegrees * TurnAuthority;
    Next.YawRateDegrees = Approach(State.YawRateDegrees, Input.Yaw * RateLimit,
        Config.AngularAccelerationDegrees * Dt);
    // Ease towards the pitch boundary before reaching it. The terminal rate
    // integrates an exponential approach, rather than hitting a hard stop.
    // Bound its slope by available angular acceleration for ordinary steering.
    const double PitchDamping = FMath::Min(2.0, Config.AngularAccelerationDegrees / RateLimit);
    const double LimitFraction = 1.0 - FMath::Exp(-PitchDamping * Dt);
    const double UpRate = (Config.PitchLimitDegrees - State.PitchDegrees) * LimitFraction / Dt;
    const double DownRate = (Config.PitchLimitDegrees + State.PitchDegrees) * LimitFraction / Dt;
    const double PitchTarget = FMath::Clamp(Input.Pitch * RateLimit, -DownRate, UpRate);
    Next.PitchRateDegrees = FMath::Clamp(Approach(State.PitchRateDegrees, PitchTarget,
        Config.AngularAccelerationDegrees * Dt), -DownRate, UpRate);
    Next.YawDegrees = FRotator3d::NormalizeAxis(State.YawDegrees + Next.YawRateDegrees * Dt);
    Next.PitchDegrees = FMath::Clamp(State.PitchDegrees + Next.PitchRateDegrees * Dt,
        -Config.PitchLimitDegrees, Config.PitchLimitDegrees);
    const double TargetBank = Config.BankDegrees * Next.YawRateDegrees / Config.TurnRateDegrees;
    const double BankDamping = 2.0 * Config.AngularAccelerationDegrees / FMath::Max(1.0, Config.BankDegrees);
    Next.BankDegrees = TargetBank + (State.BankDegrees - TargetBank) * FMath::Exp(-BankDamping * Dt);

    if (Input.bBrake) Next.Throttle = 0;
    const FVector3d Forward = Next.Forward();
    // Rotate the entire signed velocity with the nose, preserving speed through
    // a turn. Projecting onto the NEW heading would cause both slip and an
    // artificial speed loss on every steering step.
    const double ForwardSpeed = SignedSpeed(State);
    const double TargetSpeed = Next.Throttle * Config.MaxSpeed;
    const bool bSlowing = ForwardSpeed * TargetSpeed < 0 || FMath::Abs(TargetSpeed) < FMath::Abs(ForwardSpeed);
    const double Braking = FMath::Max(Config.Braking, TransitionBraking);
    const double Deceleration = Input.bBrake ? Braking : FMath::Max(Config.CoastDeceleration, TransitionCoastDeceleration);
    double NewForwardSpeed = Approach(ForwardSpeed, TargetSpeed,
        (bSlowing ? Deceleration : Config.Acceleration) * Dt);
    if (Input.bBrake)
    {
        NewForwardSpeed = Approach(ForwardSpeed, 0, Braking * Dt);
    }
    const double SpeedLimit = FMath::Max(Config.MaxSpeed, FMath::Abs(ForwardSpeed));
    Next.VelocityMetresPerSecond = Forward * FMath::Clamp(NewForwardSpeed, -SpeedLimit, SpeedLimit);

    const FVector3d Delta = Next.VelocityMetresPerSecond * Dt;
    double Earliest = 1;
    Next.ContactBodyId.Reset();
    for (const FFlightBody& Body : Bodies)
    {
        double Fraction = 1;
        if (FindContact(State.PositionMetres, Delta, Body, Config.ShipRadiusMetres, Fraction)
            && (Next.ContactBodyId.IsEmpty() || Fraction < Earliest))
        {
            Earliest = Fraction;
            Next.ContactBodyId = Body.Id;
        }
    }
    Next.PositionMetres = State.PositionMetres + Delta * Earliest;
    if (!Next.ContactBodyId.IsEmpty()) Next.VelocityMetresPerSecond = FVector3d::ZeroVector;
    Next.SimulationSeconds += Dt;
    if (!ValidState(Config, Next, SpeedLimit))
    {
        LastError = TEXT("Flight step reached a supported state boundary; prior state retained.");
        return false;
    }
    State = MoveTemp(Next);
    if (State.VelocityMetresPerSecond.Size() <= Config.MaxSpeed + 1e-6)
    {
        TransitionBraking = 0;
        TransitionCoastDeceleration = 0;
    }
    return true;
}
}

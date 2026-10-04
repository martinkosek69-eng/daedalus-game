#pragma once

#include "CoreMinimal.h"

namespace Daedalus
{
/** Assisted flight tuning, in metres, seconds and degrees. These are game settings,
 * not measurements of Star Trek Online's proprietary flight implementation. */
struct DAEDALUSSIMULATION_API FFlightConfig
{
    double MaxSpeed = 350;
    double Acceleration = 140;
    double Braking = 100;
    double CoastDeceleration = 43.75;
    double LateralAcceleration = 80; // Legacy tuning retained for catalog compatibility; directional assist eliminates slip.
    double TurnRateDegrees = 6;
    double AngularAccelerationDegrees = 18;
    double PitchLimitDegrees = 80;
    double BankDegrees = 20;
    double ShipRadiusMetres = 360;
    bool Validate(FString& Error) const;
};

struct FFlightInput
{
    double Yaw = 0;   // [-1,1], positive turns right (+Y from +X).
    double Pitch = 0; // [-1,1], positive raises the nose (+Z).
    bool bBrake = false;
};

struct FFlightBody
{
    FString Id;
    FVector3d PositionMetres = FVector3d::ZeroVector;
    double RadiusMetres = 0;
};

struct DAEDALUSSIMULATION_API FFlightState
{
    FVector3d PositionMetres = FVector3d::ZeroVector;
    FVector3d VelocityMetresPerSecond = FVector3d::ZeroVector;
    double YawDegrees = 0;
    double PitchDegrees = 0;
    double BankDegrees = 0;
    double YawRateDegrees = 0;
    double PitchRateDegrees = 0;
    double Throttle = 0;
    double SimulationSeconds = 0;
    FString ContactBodyId;
    FQuat4d Attitude() const;
    FVector3d Forward() const;
};

/** Singleplayer, sequential owner-thread domain. No Actor/input-device ownership.
 * Fixed 120 Hz; Advance processes at most 600 steps and retains the rest. Pause
 * accrues no new time. Throttle persists [-.25,1]; brake sets it to zero. At zero
 * throttle the flight assist slows the ship. Directional assist keeps signed
 * speed aligned with the nose at every step, including reverse and braking.
 * Initialization aligns any supplied velocity with the nose, preserving speed.
 * Collision is a conservative swept ship sphere against stationary body spheres,
 * stopping on contact without damage, gravity or a surface landing model.
 * Supported coordinate components: +/-1e15 m; pending time at most 1e6 s. */
class DAEDALUSSIMULATION_API FFlightModel
{
public:
    static constexpr double FixedStepSeconds = 1.0 / 120.0;
    static constexpr int32 MaxStepsPerAdvance = 600;
    bool Initialize(const FFlightConfig& InConfig, const FFlightState& InitialState,
        const TArray<FFlightBody>& InBodies, FString& Error);
    /** Change drive tuning without resetting pose, speed, input, throttle, clock,
     * backlog or pause. Excess speed after a downgrade dissipates gradually with
     * the previous drive's braking authority. Incompatible attitude/shape limits
     * are rejected without changing the configuration or simulation state. */
    bool SetConfig(const FFlightConfig& InConfig, FString& Error);
    bool SetThrottle(double Value, FString& Error);
    bool SetInput(const FFlightInput& Value, FString& Error);
    int32 Advance(double RealSeconds);
    void SetPaused(bool bValue) { bPaused = bValue; }
    const FFlightState& GetState() const { return State; }
    const FFlightConfig& GetConfig() const { return Config; }
    double GetBrakingDeceleration() const { return FMath::Max(Config.Braking, TransitionBraking); }
    double GetPendingSeconds() const { return PendingSeconds; }
    const FString& GetError() const { return LastError; }
private:
    bool Step();
    bool Fail(const FString& Message, FString& Error);
    FFlightConfig Config;
    FFlightState State;
    FFlightInput Input;
    TArray<FFlightBody> Bodies;
    double PendingSeconds = 0;
    double TransitionBraking = 0;
    double TransitionCoastDeceleration = 0;
    bool bInitialized = false;
    bool bPaused = false;
    FString LastError;
};
}

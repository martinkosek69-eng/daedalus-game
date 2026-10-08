#include "DaedalusFlightModel.h"
#include "Dom/JsonObject.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include <limits>
using namespace Daedalus;
namespace
{
void Time(FFlightModel& M, double Seconds)
{
    M.Advance(Seconds);
    while (M.GetPendingSeconds() >= FFlightModel::FixedStepSeconds) M.Advance(0);
}
FFlightModel NewFlight(const FFlightConfig& C = FFlightConfig())
{
    FFlightModel M; FString E; M.Initialize(C, FFlightState(), {}, E); return M;
}
bool ReadSolarProfiles(TArray<FFlightConfig>& Profiles)
{
    FString Text;
    TSharedPtr<FJsonObject> Root;
    if (!FFileHelper::LoadFileToString(Text, *(FPaths::ProjectContentDir() / TEXT("Data/Solar/flight.json")))
        || !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Root) || !Root.IsValid()) return false;
    const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
    if (!Root->TryGetArrayField(TEXT("profiles"), Values) || Values->Num() != 3) return false;
    for (const auto& Value : *Values)
    {
        const TSharedPtr<FJsonObject> Profile = Value->AsObject();
        FFlightConfig C; FString Error;
        if (!Profile.IsValid()
            || !Root->TryGetNumberField(TEXT("turnRateDegrees"), C.TurnRateDegrees)
            || !Root->TryGetNumberField(TEXT("angularAccelerationDegrees"), C.AngularAccelerationDegrees)
            || !Root->TryGetNumberField(TEXT("pitchLimitDegrees"), C.PitchLimitDegrees)
            || !Root->TryGetNumberField(TEXT("bankDegrees"), C.BankDegrees)
            || !Root->TryGetNumberField(TEXT("lowSpeedTurnMultiplier"), C.LowSpeedTurnMultiplier)
            || !Root->TryGetNumberField(TEXT("highSpeedBankFraction"), C.HighSpeedBankFraction)
            || !Profile->TryGetNumberField(TEXT("speed"), C.MaxSpeed)
            || !Profile->TryGetNumberField(TEXT("acceleration"), C.Acceleration)
            || !Profile->TryGetNumberField(TEXT("coastDeceleration"), C.CoastDeceleration)
            || !Profile->TryGetNumberField(TEXT("braking"), C.Braking)
            || !Profile->TryGetNumberField(TEXT("lateralAcceleration"), C.LateralAcceleration)
            || !C.Validate(Error)) return false;
        Profiles.Add(C);
    }
    return true;
}
bool NoSlip(const FFlightState& State)
{
    const double Speed = State.VelocityMetresPerSecond.Size();
    return Speed < 1e-9 || FVector3d::CrossProduct(State.Forward(), State.VelocityMetresPerSecond / Speed).Size() < 1e-10;
}
bool SameState(const FFlightState& A, const FFlightState& B)
{
    return A.PositionMetres == B.PositionMetres && A.VelocityMetresPerSecond == B.VelocityMetresPerSecond
        && A.YawDegrees == B.YawDegrees && A.PitchDegrees == B.PitchDegrees && A.BankDegrees == B.BankDegrees
        && A.YawRateDegrees == B.YawRateDegrees && A.PitchRateDegrees == B.PitchRateDegrees
        && A.Throttle == B.Throttle && A.SimulationSeconds == B.SimulationSeconds && A.ContactBodyId == B.ContactBodyId;
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFlightMotionTest, "Daedalus.Flight.ThrottleAccelerationBraking", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFlightMotionTest::RunTest(const FString&)
{
    auto M = NewFlight(); FString E;
    TestTrue(TEXT("throttle command"), M.SetThrottle(1, E));
    Time(M, 8);
    TestTrue(TEXT("accelerated capped350m/s"), FMath::Abs(M.GetState().VelocityMetresPerSecond.X - 350) < .01);
    TestEqual(TEXT("throttle persists"), M.GetState().Throttle, 1.0);
    auto Coast = M; auto Brake = M;
    Coast.SetThrottle(0, E); FFlightInput I; I.bBrake = true; Brake.SetInput(I, E);
    Time(Coast, 4); Time(Brake, 4);
    TestTrue(TEXT("coasting still moves"), Coast.GetState().VelocityMetresPerSecond.X > 150);
    TestTrue(TEXT("held brake stopped"), Brake.GetState().VelocityMetresPerSecond.IsNearlyZero(.01));
    TestEqual(TEXT("brake zeroes throttle"), Brake.GetState().Throttle, 0.0);
    Time(Coast, 5); TestTrue(TEXT("coast stops without reverse"), Coast.GetState().VelocityMetresPerSecond.IsNearlyZero(.01));
    Brake.SetInput({}, E); Brake.SetThrottle(-.25, E); Time(Brake, 8);
    TestTrue(TEXT("reverse quarter speed"), FMath::Abs(Brake.GetState().VelocityMetresPerSecond.X + 87.5) < .01);
    TArray<FFlightConfig> Profiles;
    if (!TestTrue(TEXT("canonical solar profiles loaded"), ReadSolarProfiles(Profiles))) return false;
    for (const FFlightConfig& C : Profiles)
    {
        auto Flight = NewFlight(C); Flight.SetThrottle(1, E);
        Time(Flight, 2.5);
        TestTrue(TEXT("actual solar profile reaches max within three seconds"), FMath::Abs(Flight.GetState().VelocityMetresPerSecond.Size() - C.MaxSpeed) < .001);
        Time(Flight, .5);
        TestTrue(TEXT("maximum speed plateau remains capped"), FMath::Abs(Flight.GetState().VelocityMetresPerSecond.Size() - C.MaxSpeed) < .001);
        auto PartitionA = Flight; auto PartitionB = Flight;
        PartitionA.SetInput({-.8,.6,false}, E); PartitionB.SetInput({-.8,.6,false}, E);
        for (int32 Frame = 0; Frame < 180; ++Frame) PartitionA.Advance(1.0 / 60);
        for (int32 Frame = 0; Frame < 432; ++Frame) PartitionB.Advance(1.0 / 144);
        TestTrue(TEXT("canonical profiles retain frame-partition invariance"),
            (PartitionA.GetState().PositionMetres - PartitionB.GetState().PositionMetres).Size() < 1e-6
            && (PartitionA.GetState().VelocityMetresPerSecond - PartitionB.GetState().VelocityMetresPerSecond).Size() < 1e-6
            && FMath::Abs(PartitionA.GetState().BankDegrees - PartitionB.GetState().BankDegrees) < 1e-8);
        Flight.SetInput({1,.7,false}, E);
        bool bAligned = true, bPreserved = true;
        for (int32 Step = 0; Step < 720; ++Step)
        {
            Flight.Advance(FFlightModel::FixedStepSeconds);
            bAligned &= NoSlip(Flight.GetState());
            bPreserved &= FMath::Abs(Flight.GetState().VelocityMetresPerSecond.Size() - C.MaxSpeed) < .001;
        }
        TestTrue(TEXT("fast compound turn never produces lateral slip"), bAligned);
        TestTrue(TEXT("steering alone never slows full-speed ship"), bPreserved);
        Flight.SetThrottle(-.25, E);
        bool bReverseAligned = true;
        for (int32 Step = 0; Step < 1200; ++Step)
        {
            Flight.Advance(FFlightModel::FixedStepSeconds);
            bReverseAligned &= NoSlip(Flight.GetState());
        }
        TestTrue(TEXT("reverse transition and reverse turns remain aligned"), bReverseAligned);
        TestTrue(TEXT("reverse maintains negative quarter speed"), FMath::Abs(FVector3d::DotProduct(Flight.GetState().Forward(), Flight.GetState().VelocityMetresPerSecond) + .25 * C.MaxSpeed) < .001);
        Flight.SetInput({-1,-1,true}, E);
        bool bBrakeAligned = true, bBrakeMonotone = true;
        double PreviousSpeed = Flight.GetState().VelocityMetresPerSecond.Size();
        for (int32 Step = 0; Step < 1200; ++Step)
        {
            Flight.Advance(FFlightModel::FixedStepSeconds);
            const double Speed = Flight.GetState().VelocityMetresPerSecond.Size();
            bBrakeAligned &= NoSlip(Flight.GetState());
            bBrakeMonotone &= Speed <= PreviousSpeed + 1e-8;
            PreviousSpeed = Speed;
        }
        TestTrue(TEXT("braking while steering remains aligned and slows monotonically"), bBrakeAligned && bBrakeMonotone);
        TestTrue(TEXT("braking reaches an exact stop without reversal"), Flight.GetState().VelocityMetresPerSecond.IsZero());
    }
    auto Transition = NewFlight(Profiles[1]);
    Transition.SetThrottle(1, E); Time(Transition, 2.5);
    const FFlightState BeforeUpgrade = Transition.GetState();
    TestTrue(TEXT("upgrade to full sublight while moving"), Transition.SetConfig(Profiles[2], E));
    TestTrue(TEXT("upgrade changes no state immediately"), SameState(BeforeUpgrade, Transition.GetState()));
    Time(Transition, 2.5);
    TestTrue(TEXT("full sublight reached through acceleration"),
        FMath::Abs(Transition.GetState().VelocityMetresPerSecond.Size() - 250000000.0) < .001);
    const FFlightState BeforeDowngrade = Transition.GetState();
    TestTrue(TEXT("downgrade to impulse while moving"), Transition.SetConfig(Profiles[1], E));
    TestTrue(TEXT("downgrade preserves pose speed and clock exactly"), SameState(BeforeDowngrade, Transition.GetState()));
    const auto BeforeStep = Transition.GetState();
    auto RepeatedDowngrade = Transition;
    TestTrue(TEXT("second downgrade accepted before first settles"), RepeatedDowngrade.SetConfig(Profiles[0], E));
    Time(RepeatedDowngrade, 8);
    TestTrue(TEXT("repeated downgrade retains full impulse coasting authority"),
        FMath::Abs(RepeatedDowngrade.GetState().VelocityMetresPerSecond.Size() - Profiles[0].MaxSpeed) < .001);
    Transition.Advance(FFlightModel::FixedStepSeconds);
    const double SpeedAfterStep = Transition.GetState().VelocityMetresPerSecond.Size();
    TestTrue(TEXT("first downgrade step decelerates gradually with old authority"),
        SpeedAfterStep > Profiles[1].MaxSpeed
        && FMath::Abs(SpeedAfterStep - (BeforeStep.VelocityMetresPerSecond.Size()
            - Profiles[2].CoastDeceleration * FFlightModel::FixedStepSeconds)) < .001);
    auto BrakingTransition = Transition;
    BrakingTransition.SetInput({.7,-.4,true}, E);
    Time(BrakingTransition, 14);
    TestTrue(TEXT("brake during downshift retains high authority and stops promptly"),
        BrakingTransition.GetState().VelocityMetresPerSecond.IsZero() && NoSlip(BrakingTransition.GetState()));
    Transition.SetInput({.7,.4,false}, E);
    bool bMonotone = true, bNoSlip = true;
    double PreviousSpeed = SpeedAfterStep;
    for (int32 Step = 0; Step < 960; ++Step)
    {
        Transition.Advance(FFlightModel::FixedStepSeconds);
        const double Speed = Transition.GetState().VelocityMetresPerSecond.Size();
        bMonotone &= Speed <= PreviousSpeed + 1e-6;
        bNoSlip &= NoSlip(Transition.GetState());
        PreviousSpeed = Speed;
    }
    TestTrue(TEXT("downshift smoothly reaches normal impulse without lateral drift"),
        bMonotone && bNoSlip && FMath::Abs(PreviousSpeed - Profiles[1].MaxSpeed) < .001);
    Transition.SetInput({}, E); Transition.SetThrottle(0, E);
    Transition.Advance(FFlightModel::FixedStepSeconds);
    TestTrue(TEXT("ordinary coasting restored once excess speed is gone"),
        FMath::Abs(Transition.GetState().VelocityMetresPerSecond.Size()
            - (Profiles[1].MaxSpeed - Profiles[1].CoastDeceleration * FFlightModel::FixedStepSeconds)) < .001);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFlightTurnTest, "Daedalus.Flight.TurnPitchBankAndDrift", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFlightTurnTest::RunTest(const FString&)
{
    FFlightConfig C; C.LateralAcceleration = 200;
    auto M = NewFlight(C); FString E; M.SetThrottle(1, E); Time(M, 8);
    FFlightInput I; I.Yaw = 1; M.SetInput(I, E); Time(M, 5);
    TestTrue(TEXT("yaw commanded right"), M.GetState().YawDegrees > 25);
    TestTrue(TEXT("rate at bound"), FMath::Abs(M.GetState().YawRateDegrees - C.TurnRateDegrees) < .001);
    const auto& S = M.GetState();
    TestTrue(TEXT("turn has strictly no sideways slip"), NoSlip(S));
    TestTrue(TEXT("bank bounded"), FMath::Abs(S.BankDegrees) <= C.BankDegrees);
    auto BrakeInTurn = M;
    BrakeInTurn.SetInput({0,0,true}, E); Time(BrakeInTurn, 3.6);
    TestTrue(TEXT("brake stops sideways drift too"), BrakeInTurn.GetState().VelocityMetresPerSecond.IsNearlyZero(.001));
    TestEqual(TEXT("braking clears persistent throttle after turn"), BrakeInTurn.GetState().Throttle, 0.0);
    M.SetInput({}, E); Time(M, 8);
    TestTrue(TEXT("rotation input decays"), FMath::Abs(M.GetState().YawRateDegrees) < .001);
    TestTrue(TEXT("bank returns level"), FMath::Abs(M.GetState().BankDegrees) < .01);
    I.Yaw = 0; I.Pitch = 1; M.SetInput(I, E); Time(M, 60);
    TestTrue(TEXT("pitch bounded without flip"), FMath::Abs(M.GetState().PitchDegrees - C.PitchLimitDegrees) < .001);
    TestTrue(TEXT("at limit pitch rate settles gently"), FMath::Abs(M.GetState().PitchRateDegrees) < .001);
    auto Stopped = NewFlight(C); Stopped.SetInput({1,0,false}, E); Time(Stopped, 3);
    TestTrue(TEXT("stationary steering retains full authority"), FMath::Abs(Stopped.GetState().YawRateDegrees - C.TurnRateDegrees) < .01);

    TArray<FFlightConfig> Profiles;
    if (!TestTrue(TEXT("speed-dependent tuning loaded"), ReadSolarProfiles(Profiles))) return false;
    const auto& Tuning = Profiles[1];
    auto SlowTurn = NewFlight(Tuning), FastTurn = NewFlight(Tuning);
    FFlightState FastStart; FastStart.VelocityMetresPerSecond = FVector3d(Tuning.MaxSpeed, 0, 0); FastStart.Throttle = 1;
    TestTrue(TEXT("fast turn initial state valid"), FastTurn.Initialize(Tuning, FastStart, {}, E));
    SlowTurn.SetInput({1,0,false}, E); FastTurn.SetInput({1,0,false}, E);
    Time(SlowTurn, 5); Time(FastTurn, 5);
    TestTrue(TEXT("low speed U turn is faster than full speed"), SlowTurn.GetState().YawRateDegrees > FastTurn.GetState().YawRateDegrees * 1.4);
    TestTrue(TEXT("low speed bank is larger than high speed"), SlowTurn.GetState().BankDegrees > FastTurn.GetState().BankDegrees * 1.5);
    TestTrue(TEXT("full-speed bank matches shallower bound"), FMath::Abs(FastTurn.GetState().BankDegrees - Tuning.BankDegrees * Tuning.HighSpeedBankFraction) < .01);
    TestTrue(TEXT("speed-dependent turn never causes drift"), NoSlip(FastTurn.GetState()));
    auto InvalidTuning = Tuning; InvalidTuning.LowSpeedTurnMultiplier = 0;
    TestFalse(TEXT("invalid speed-dependent multiplier rejected"), InvalidTuning.Validate(E));

    auto Bank = NewFlight(C); Bank.SetThrottle(1, E); Bank.SetInput({1,0,false}, E);
    bool bBankBounds = true, bRiseMonotone = true, bNoHardClamp = true;
    double PreviousBank = 0;
    for (int32 Step = 0; Step < 480; ++Step)
    {
        Bank.Advance(FFlightModel::FixedStepSeconds);
        const double Current = Bank.GetState().BankDegrees;
        bBankBounds &= FMath::Abs(Current) <= C.BankDegrees;
        bRiseMonotone &= Current >= PreviousBank - 1e-10;
        bNoHardClamp &= Current < C.BankDegrees;
        PreviousBank = Current;
    }
    TestTrue(TEXT("sustained banking rises monotonically without hitting a hard clamp"), bBankBounds && bRiseMonotone && bNoHardClamp);
    Bank.SetInput({}, E);
    // Angular input still decelerates before the bank starts levelling out.
    Time(Bank, C.TurnRateDegrees / C.AngularAccelerationDegrees);
    bool bReleaseMonotone = true;
    PreviousBank = Bank.GetState().BankDegrees;
    for (int32 Step = 0; Step < 720; ++Step)
    {
        Bank.Advance(FFlightModel::FixedStepSeconds);
        const double Current = Bank.GetState().BankDegrees;
        bReleaseMonotone &= Current <= PreviousBank + 1e-10 && Current >= 0;
        PreviousBank = Current;
    }
    TestTrue(TEXT("released bank levels monotonically without overshoot"), bReleaseMonotone && PreviousBank < .001);
    Bank.SetInput({1,0,false}, E); Time(Bank, 4); Bank.SetInput({-1,0,false}, E);
    Time(Bank, 2 * C.TurnRateDegrees / C.AngularAccelerationDegrees);
    bool bReversalMonotone = true;
    PreviousBank = Bank.GetState().BankDegrees;
    for (int32 Step = 0; Step < 720; ++Step)
    {
        Bank.Advance(FFlightModel::FixedStepSeconds);
        const double Current = Bank.GetState().BankDegrees;
        bReversalMonotone &= Current <= PreviousBank + 1e-10 && Current > -C.BankDegrees;
        PreviousBank = Current;
    }
    TestTrue(TEXT("reversed bank settles towards opposite bound without overshoot"), bReversalMonotone && FMath::Abs(PreviousBank + C.BankDegrees) < .001);

    auto Pitch = NewFlight(C); Pitch.SetThrottle(1, E); Pitch.SetInput({0,1,false}, E);
    bool bPitchBounds = true, bGentle = true, bSlowingBeforeBoundary = false;
    double PreviousPitch = 0, PreviousRate = 0;
    for (int32 Step = 0; Step < 3600; ++Step)
    {
        Pitch.Advance(FFlightModel::FixedStepSeconds);
        const auto& Current = Pitch.GetState();
        bPitchBounds &= Current.PitchDegrees >= PreviousPitch && Current.PitchDegrees <= C.PitchLimitDegrees;
        bGentle &= FMath::Abs(Current.PitchRateDegrees - PreviousRate) <= C.AngularAccelerationDegrees * FFlightModel::FixedStepSeconds + 1e-8;
        bSlowingBeforeBoundary |= Current.PitchDegrees > C.PitchLimitDegrees - 1 && Current.PitchRateDegrees > 0 && Current.PitchRateDegrees < C.TurnRateDegrees * .5;
        PreviousPitch = Current.PitchDegrees; PreviousRate = Current.PitchRateDegrees;
    }
    TestTrue(TEXT("pitch slows before its boundary without abrupt rate changes"), bPitchBounds && bGentle && bSlowingBeforeBoundary);
    Pitch.SetInput({0,-1,false}, E); Time(Pitch, 60);
    TestTrue(TEXT("pitch reverses and gently settles at lower limit"), FMath::Abs(Pitch.GetState().PitchDegrees + C.PitchLimitDegrees) < .001 && FMath::Abs(Pitch.GetState().PitchRateDegrees) < .001);
    auto MirroredPitch = NewFlight(C); MirroredPitch.SetThrottle(1, E); MirroredPitch.SetInput({0,-1,false}, E);
    Time(MirroredPitch, 30);
    TestTrue(TEXT("lower pitch bound has the same gentle settling as upper bound"),
        FMath::Abs(MirroredPitch.GetState().PitchDegrees + PreviousPitch) < 1e-8
        && FMath::Abs(MirroredPitch.GetState().PitchRateDegrees + PreviousRate) < 1e-8);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFlightClockTest, "Daedalus.Flight.PartitionsPauseAndValidation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFlightClockTest::RunTest(const FString&)
{
    auto A = NewFlight(); auto B = A; FString E;
    A.SetThrottle(.8,E);B.SetThrottle(.8,E);A.SetInput({.7,.2,false},E);B.SetInput({.7,.2,false},E);
    for(int32 N=0;N<600;++N) A.Advance(1.0/60);
    for(int32 N=0;N<1440;++N) B.Advance(1.0/144);
    TestTrue(TEXT("frame partitions same position"),(A.GetState().PositionMetres-B.GetState().PositionMetres).Size()<1e-6);
    TestTrue(TEXT("frame partitions same heading"),FMath::Abs(A.GetState().YawDegrees-B.GetState().YawDegrees)<1e-8);
    TestTrue(TEXT("frame partitions same speed and banking"),(A.GetState().VelocityMetresPerSecond-B.GetState().VelocityMetresPerSecond).Size()<1e-8 && FMath::Abs(A.GetState().BankDegrees-B.GetState().BankDegrees)<1e-8);
    const auto Before=A.GetState();A.SetPaused(true);A.Advance(100);
    TestTrue(TEXT("pause stops clock and pose"),A.GetState().SimulationSeconds==Before.SimulationSeconds && A.GetState().PositionMetres==Before.PositionMetres);
    A.SetInput({},E);A.SetPaused(false);Time(A,2);
    TestTrue(TEXT("release during pause stops turn on resume"),FMath::Abs(A.GetState().YawRateDegrees)<.001);
    const auto Valid=A.GetState();
    TestFalse(TEXT("nan throttle rejected"),A.SetThrottle(std::numeric_limits<double>::quiet_NaN(),E));
    TestFalse(TEXT("invalid input rejected"),A.SetInput({2,0,false},E));
    TestEqual(TEXT("negative time rejected"),A.Advance(-1),0);
    TestTrue(TEXT("errors retain state"),A.GetState().PositionMetres==Valid.PositionMetres);
    auto InvalidState = Valid; InvalidState.BankDegrees = std::numeric_limits<double>::infinity();
    TestFalse(TEXT("nonfinite bank rejected transactionally"), A.Initialize(A.GetConfig(), InvalidState, {}, E));
    TestTrue(TEXT("failed initialization keeps current pose"), A.GetState().PositionMetres == Valid.PositionMetres);
    InvalidState = Valid; InvalidState.VelocityMetresPerSecond = FVector3d(0,20,0); InvalidState.YawDegrees = 0; InvalidState.PitchDegrees = 0;
    TestTrue(TEXT("legacy lateral initialization accepted"), A.Initialize(A.GetConfig(), InvalidState, {}, E));
    TestTrue(TEXT("legacy lateral velocity aligns immediately and preserves speed"), NoSlip(A.GetState()) && FMath::Abs(A.GetState().VelocityMetresPerSecond.Size() - 20) < 1e-8);
    auto Backlog=NewFlight();Backlog.Advance(8);
    TestTrue(TEXT("step cap retains backlog"),Backlog.GetPendingSeconds()>2.9);
    Time(Backlog,0);TestTrue(TEXT("draining retains total clock"),FMath::Abs(Backlog.GetState().SimulationSeconds-8)<1e-7);

    TArray<FFlightConfig> Profiles;
    if (!TestTrue(TEXT("profiles for live transitions loaded"), ReadSolarProfiles(Profiles))) return false;
    auto Live = NewFlight(Profiles[2]);
    Live.SetThrottle(1, E); Live.SetInput({.6,.3,false}, E); Time(Live, 3);
    Live.Advance(.003); Live.SetPaused(true);
    const auto LiveBefore = Live.GetState();
    const double PendingBefore = Live.GetPendingSeconds();
    TestTrue(TEXT("paused reconfiguration accepted"), Live.SetConfig(Profiles[1], E));
    TestTrue(TEXT("reconfiguration preserves all state and pending time"),
        SameState(LiveBefore, Live.GetState()) && Live.GetPendingSeconds() == PendingBefore);
    Live.Advance(100);
    TestTrue(TEXT("reconfiguration preserves pause without accruing time"),
        SameState(LiveBefore, Live.GetState()) && Live.GetPendingSeconds() == PendingBefore);
    auto Reference = Live;
    FFlightConfig BadConfig = Profiles[0]; BadConfig.Acceleration = std::numeric_limits<double>::quiet_NaN();
    TestFalse(TEXT("invalid drive rejected transactionally"), Live.SetConfig(BadConfig, E));
    BadConfig = Profiles[0]; BadConfig.BankDegrees = 0;
    TestFalse(TEXT("incompatible current bank rejected transactionally"), Live.SetConfig(BadConfig, E));
    TestTrue(TEXT("rejected drive preserves configuration state and backlog"),
        Live.GetConfig().MaxSpeed == Reference.GetConfig().MaxSpeed
        && SameState(Live.GetState(), Reference.GetState()) && Live.GetPendingSeconds() == PendingBefore);
    Live.SetPaused(false); Reference.SetPaused(false);
    Reference.SetInput({.6,.3,false}, E);
    for (int32 Frame = 0; Frame < 480; ++Frame) Live.Advance(1.0 / 60);
    for (int32 Frame = 0; Frame < 1152; ++Frame) Reference.Advance(1.0 / 144);
    TestTrue(TEXT("downshift partitions preserve prior input and rollback braking authority"), SameState(Live.GetState(), Reference.GetState()));
    TestTrue(TEXT("downshift settles without an integration error"), Live.GetError().IsEmpty()
        && FMath::Abs(Live.GetState().VelocityMetresPerSecond.Size() - Profiles[1].MaxSpeed) < .001);
    FFlightModel Uninitialized;
    TestFalse(TEXT("uninitialized reconfiguration rejected"), Uninitialized.SetConfig(Profiles[0], E));
    BadConfig = Profiles[2]; BadConfig.MaxSpeed = 299792458.0;
    TestFalse(TEXT("light speed excluded from sublight profiles"), BadConfig.Validate(E));
    BadConfig.MaxSpeed = 299792457.0;
    TestTrue(TEXT("finite sublight range supported"), BadConfig.Validate(E));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFlightContactTest, "Daedalus.Flight.LargeCoordinateSweptContact", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFlightContactTest::RunTest(const FString&)
{
    FFlightConfig C;C.MaxSpeed=250000000;C.Acceleration=1e8;C.ShipRadiusMetres=360;
    FFlightState S;S.PositionMetres=FVector3d(149597870700.0,0,0);S.VelocityMetresPerSecond=FVector3d(C.MaxSpeed,0,0);S.Throttle=1;
    FFlightBody Body{TEXT("earth"),S.PositionMetres+FVector3d(100000,0,0),6371};
    FFlightModel M;FString E;TestTrue(TEXT("large init"),M.Initialize(C,S,{Body},E));Time(M,1);
    TestTrue(TEXT("swept safety cannot tunnel"),(M.GetState().PositionMetres-Body.PositionMetres).Size()>=Body.RadiusMetres+C.ShipRadiusMetres-.001);
    TestEqual(TEXT("contact body reported"),M.GetState().ContactBodyId,Body.Id);
    TestTrue(TEXT("surface stop"),M.GetState().VelocityMetresPerSecond.IsNearlyZero());
    const auto ContactState = M.GetState();
    auto Larger = C; Larger.ShipRadiusMetres *= 2;
    TestFalse(TEXT("live radius increase cannot intersect surface"), M.SetConfig(Larger, E));
    TestTrue(TEXT("rejected shape change preserves contact and radius"), SameState(ContactState, M.GetState())
        && M.GetConfig().ShipRadiusMetres == C.ShipRadiusMetres);
    auto Bad=S;Bad.PositionMetres=Body.PositionMetres;
    TestFalse(TEXT("overlap initialization rejected"),M.Initialize(C,Bad,{Body},E));
    Bad=M.GetState();Bad.Throttle=0;Bad.VelocityMetresPerSecond=FVector3d(-10,0,0);
    TestTrue(TEXT("safe state restart"),M.Initialize(C,Bad,{Body},E));Time(M,.1);
    TestTrue(TEXT("movement away from body permitted"),M.GetState().PositionMetres.X<Bad.PositionMetres.X);
    return true;
}

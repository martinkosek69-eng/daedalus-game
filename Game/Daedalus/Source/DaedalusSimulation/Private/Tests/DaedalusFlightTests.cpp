#include "DaedalusFlightModel.h"
#include "Misc/AutomationTest.h"
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
    const double Slip = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector3d::DotProduct(S.Forward(), S.VelocityMetresPerSecond.GetSafeNormal()), -1.0, 1.0)));
    TestTrue(TEXT("visible moderate drift"), Slip > 2 && Slip < 16);
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
    TestEqual(TEXT("at limit pitch rate stops"), M.GetState().PitchRateDegrees, 0.0);
    auto Stopped = NewFlight(C); Stopped.SetInput({1,0,false}, E); Time(Stopped, 3);
    TestTrue(TEXT("stop retains reduced turning"), FMath::Abs(Stopped.GetState().YawRateDegrees - C.TurnRateDegrees * .5) < .01);
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
    const auto Before=A.GetState();A.SetPaused(true);A.Advance(100);
    TestTrue(TEXT("pause stops clock and pose"),A.GetState().SimulationSeconds==Before.SimulationSeconds && A.GetState().PositionMetres==Before.PositionMetres);
    A.SetInput({},E);A.SetPaused(false);Time(A,2);
    TestTrue(TEXT("release during pause stops turn on resume"),FMath::Abs(A.GetState().YawRateDegrees)<.001);
    const auto Valid=A.GetState();
    TestFalse(TEXT("nan throttle rejected"),A.SetThrottle(std::numeric_limits<double>::quiet_NaN(),E));
    TestFalse(TEXT("invalid input rejected"),A.SetInput({2,0,false},E));
    TestEqual(TEXT("negative time rejected"),A.Advance(-1),0);
    TestTrue(TEXT("errors retain state"),A.GetState().PositionMetres==Valid.PositionMetres);
    auto Backlog=NewFlight();Backlog.Advance(8);
    TestTrue(TEXT("step cap retains backlog"),Backlog.GetPendingSeconds()>2.9);
    Time(Backlog,0);TestTrue(TEXT("draining retains total clock"),FMath::Abs(Backlog.GetState().SimulationSeconds-8)<1e-7);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFlightContactTest, "Daedalus.Flight.LargeCoordinateSweptContact", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFlightContactTest::RunTest(const FString&)
{
    FFlightConfig C;C.MaxSpeed=1e7;C.Acceleration=1e8;C.ShipRadiusMetres=360;
    FFlightState S;S.PositionMetres=FVector3d(149597870700.0,0,0);S.VelocityMetresPerSecond=FVector3d(C.MaxSpeed,0,0);S.Throttle=1;
    FFlightBody Body{TEXT("earth"),S.PositionMetres+FVector3d(100000,0,0),6371};
    FFlightModel M;FString E;TestTrue(TEXT("large init"),M.Initialize(C,S,{Body},E));Time(M,1);
    TestTrue(TEXT("swept safety cannot tunnel"),(M.GetState().PositionMetres-Body.PositionMetres).Size()>=Body.RadiusMetres+C.ShipRadiusMetres-.001);
    TestEqual(TEXT("contact body reported"),M.GetState().ContactBodyId,Body.Id);
    TestTrue(TEXT("surface stop"),M.GetState().VelocityMetresPerSecond.IsNearlyZero());
    auto Bad=S;Bad.PositionMetres=Body.PositionMetres;
    TestFalse(TEXT("overlap initialization rejected"),M.Initialize(C,Bad,{Body},E));
    Bad=M.GetState();Bad.Throttle=0;Bad.VelocityMetresPerSecond=FVector3d(-10,0,0);
    TestTrue(TEXT("safe state restart"),M.Initialize(C,Bad,{Body},E));Time(M,.1);
    TestTrue(TEXT("movement away from body permitted"),M.GetState().PositionMetres.X<Bad.PositionMetres.X);
    return true;
}

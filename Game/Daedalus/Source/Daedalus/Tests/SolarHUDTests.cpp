#include "Misc/AutomationTest.h"
#include "Solar/SolarHUDRadar.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSolarHUDRadarTest, "Daedalus.HUD.RadarCoordinates",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSolarHUDRadarTest::RunTest(const FString&)
{
    using namespace SolarHUD;
    // Large canonical positions must give the same local coordinates as a
    // nearby system; moving the scene origin cannot change the radar.
    const FVector3d Origin(1e15, -1e15, 1e14);
    const FVector3d Delta(1000, 2000, 3000);
    const auto Large = ProjectRadar(Origin + Delta, Origin, 0, 5000);
    const auto Local = ProjectRadar(Delta, FVector3d::ZeroVector, 0, 5000);
    TestTrue(TEXT("Translation invariant"), Large.bValid && Large.UnitPosition.Equals(Local.UnitPosition, 1e-9));
    TestTrue(TEXT("Forward above, starboard right"), Local.UnitPosition.Equals(FVector2D(.4, -.2), 1e-9));
    TestEqual(TEXT("Vertical separation retained"), Local.HeightMetres, 3000.0);

    const auto Turned = ProjectRadar(FVector3d(0, 2500, 0), FVector3d::ZeroVector, 90, 5000);
    TestTrue(TEXT("Yaw rotates heading up"), Turned.UnitPosition.Equals(FVector2D(0, -.5), 1e-9));
    const auto Distant = ProjectRadar(FVector3d(0, 1e12, 1000), FVector3d::ZeroVector, 0, 5000);
    TestTrue(TEXT("Distant bearing clamps to rim"), Distant.bBeyondRange && FMath::IsNearlyEqual(Distant.UnitPosition.Size(), 1.0));
    TestEqual(TEXT("True distance not clamped"), Distant.DistanceMetres, FVector3d(0, 1e12, 1000).Size());
    const auto Overhead = ProjectRadar(FVector3d(0, 0, 10000), FVector3d::ZeroVector, 0, 5000);
    TestTrue(TEXT("Overhead is planar centre with altitude"), Overhead.bValid && !Overhead.bBeyondRange && Overhead.UnitPosition.IsNearlyZero() && Overhead.HeightMetres == 10000);

    TestFalse(TEXT("Invalid range"), ProjectRadar(Delta, Origin, 0, 0).bValid);
    TestFalse(TEXT("Nonfinite heading"), ProjectRadar(Delta, Origin, std::numeric_limits<double>::infinity(), 5000).bValid);
    FVector3d InvalidPosition = FVector3d::ZeroVector;
    InvalidPosition.X = std::numeric_limits<double>::quiet_NaN();
    TestFalse(TEXT("Nonfinite position"), ProjectRadar(InvalidPosition, Origin, 0, 5000).bValid);
    TestEqual(TEXT("Minimum local context"), RadarRange(0), 5e6);
    TestEqual(TEXT("Readable scale"), RadarRange(8e6), 2e7);
    TestTrue(TEXT("Invalid distance safe fallback"), FMath::IsFinite(RadarRange(std::numeric_limits<double>::quiet_NaN())));
    return true;
}

#include "Solar/SolarFlightGameMode.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "UnrealClient.h"

// Small packaged integration check; no player's saves or existing package edited.
void ASolarFlightGameMode::TickIntegrationProbe()
{
    ++ProbeFrame;
    GEngine->bUseFixedFrameRate = true; GEngine->FixedFrameRate = 60;
    auto* PC = GetWorld()->GetFirstPlayerController();
    auto Check = [&](bool Pass, const TCHAR* Label) {
        bProbePassed &= Pass;
        UE_LOG(LogTemp, Display, TEXT("INTEGRATION_PROBE %s %s"), Label, Pass ? TEXT("PASS") : TEXT("FAIL"));
    };
    auto Key = [&](FKey K, EInputEvent E, double Value = 1) {
        if (PC) PC->InputKey(FInputKeyEventArgs::CreateSimulated(K, E, Value));
    };
    auto Tap = [&](FKey K) { Key(K, IE_Pressed); Key(K, IE_Released); };
    auto Shot = [&](const TCHAR* Name) {
        IFileManager::Get().MakeDirectory(*ProbeDirectory, true);
        FScreenshotRequest::RequestScreenshot(ProbeDirectory / Name, true, false);
    };
    if (!bReady)
    {
        Check(false, TEXT("scene ready"));
        FFileHelper::SaveStringToFile(TEXT("{\"passed\":false,\"error\":\"scene not ready\"}"), *(ProbeDirectory / TEXT("result.json")));
        FPlatformMisc::RequestExitWithStatus(false, 1); return;
    }
    if (BeaconLevel == BeaconEmissionOn) ProbeYaw = 1;
    if (BeaconLevel == BeaconEmissionOff) ProbePitch = 1;
    if (ProbeFrame == 120)
    {
        Check(GEngine->GameViewport->Viewport->GetSizeXY() == FIntPoint(3840, 2160), TEXT("native 3840x2160 render"));
        Check(Ship && FMath::Abs(Ship->GetStaticMesh()->GetBounds().BoxExtent.X * 2 - 60000) < 30, TEXT("latest 600 metre hull"));
        Check(HullLights && ShipDetails.Num() == 3 && HangarDoors.Num() == 4 && EngineGlows.Num() == 6 && ShipPointLights.Num() == 10,
            TEXT("latest split meshes and source lights; no duplicate turrets"));
        int32 CloudCount = 0;
        for (const auto& Layer : CloudMeshes) if (Layer) ++CloudCount;
        Check(CloudCount == 1 && PlanetSpheres.Num() == 3 && Systems.Num() == 6, TEXT("reviewed planet layers and six systems"));
        Check(DoorTravel == 0 && !bHangarsOpen, TEXT("doors start closed"));
        Check(IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"))->GetInt() == 100
            && IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality"))->GetInt() == 0, TEXT("full resolution and no motion blur"));
        Shot(TEXT("hud-and-latest-ship.png"));
        Tap(EKeys::H);
    }
    if (ProbeFrame == 250)
    {
        Check(bHangarsOpen && DoorTravel == 1, TEXT("H opens doors smoothly within two seconds"));
        const FQuat Rotation(Flight.GetState().Attitude());
        for (int32 I = 0; I < HangarDoors.Num(); ++I)
            Check(HangarDoors[I]->GetComponentLocation().Equals(Rotation.RotateVector(DoorOpenOffsets[I]), .01), TEXT("baked door frame offset exactly once"));
        for (int32 I = 0; I < ShipPointLights.Num(); ++I)
        {
            Check(ShipPointLights[I]->GetComponentLocation().Equals(Rotation.RotateVector(ShipLightPositions[I]), .01), TEXT("point light follows hull pose"));
            Check(FMath::IsNearlyEqual(ShipPointLights[I]->Intensity,
                ShipLightCandela[I] * (ShipLightIsEngine[I] ? EngineGlowLevel : 1), 1.f), TEXT("source light with game exposure calibration"));
        }
        Shot(TEXT("hud-doors-open.png"));
        Key(EKeys::RightMouseButton, IE_Pressed);
    }
    if (ProbeFrame >= 252 && ProbeFrame <= 280) Key(EKeys::MouseX, IE_Axis, 18);
    if (ProbeFrame == 282) Key(EKeys::RightMouseButton, IE_Released);
    if (ProbeFrame == 310)
    {
        Check(ProbeYaw == 1 && ProbePitch == 1, TEXT("source five second beacon blink has on and off phases"));
        for (const auto& Dynamic : BeaconDynamics)
            Check(FMath::IsNearlyEqual(Dynamic->K2_GetScalarParameterValue(TEXT("BeaconLevel")), BeaconLevel), TEXT("blink reaches rendered material"));
        Shot(TEXT("hud-front-hangars.png"));
        Tap(EKeys::H); // Close, then pause mid-travel.
    }
    if (ProbeFrame == 340) { Tap(EKeys::P); ProbeClock = DoorTravel; }
    if (ProbeFrame == 355) { Tap(EKeys::H); Check(bPaused && DoorTravel == ProbeClock && !bHangarsOpen, TEXT("pause freezes doors and blocks H")); Tap(EKeys::P); }
    if (ProbeFrame == 450) { Check(DoorTravel == 0, TEXT("doors close fully after two seconds excluding pause")); Tap(EKeys::M); ProbeClock = Flight.GetState().SimulationSeconds; }
    if (ProbeFrame == 460)
    {
        Tap(EKeys::H);
        Check(Galaxy.bOpen && !bHangarsOpen && Flight.GetState().SimulationSeconds == ProbeClock, TEXT("map keeps ship presentation paused"));
        Shot(TEXT("galaxy-with-new-hud.png"));
    }
    if (ProbeFrame == 480) { Tap(EKeys::M); Tap(EKeys::R); }
    if (ProbeFrame == 675)
    {
        Check(Flight.GetState().Throttle == 1 && EngineGlowLevel == 1, TEXT("actual R input powers all engine effects"));
        Check(Flight.GetError().IsEmpty(), TEXT("unchanged flight domain remains valid"));
        Shot(TEXT("hud-impulse.png"));
    }
    if (ProbeFrame == 700)
    {
        for (const TCHAR* Name : {TEXT("hud-and-latest-ship.png"), TEXT("hud-doors-open.png"), TEXT("hud-front-hangars.png"), TEXT("galaxy-with-new-hud.png"), TEXT("hud-impulse.png")})
            Check(IFileManager::Get().FileSize(*(ProbeDirectory / Name)) > 100, TEXT("actual HUD render saved"));
        const FString Result = FString::Printf(TEXT("{\"passed\":%s,\"width\":3840,\"height\":2160,\"doors\":%d,\"pointLights\":%d,\"systems\":%d}"),
            bProbePassed ? TEXT("true") : TEXT("false"), HangarDoors.Num(), ShipPointLights.Num(), Systems.Num());
        FFileHelper::SaveStringToFile(Result, *(ProbeDirectory / TEXT("result.json")));
        FPlatformMisc::RequestExitWithStatus(false, bProbePassed ? 0 : 1);
    }
}

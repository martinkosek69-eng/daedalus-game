#include "Solar/SolarFlightGameMode.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Texture2D.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "InputKeyEventArgs.h"
#include "Materials/MaterialInterface.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"

void ASolarFlightGameMode::TickSharpProbe()
{
    ++ProbeFrame;
    GEngine->bUseFixedFrameRate = true; GEngine->FixedFrameRate = 60;
    auto* PC = GetWorld()->GetFirstPlayerController();
    auto* Pawn = PC ? Cast<ASolarFlightPawn>(PC->GetPawn()) : nullptr;
    const auto& State = Flight.GetState();
    auto Check = [&](bool Pass, const TCHAR* Label) {
        bProbePassed &= Pass;
        UE_LOG(LogTemp, Display, TEXT("SHARP_PROBE %s %s"), Label, Pass ? TEXT("PASS") : TEXT("FAIL"));
    };
    auto Key = [&](FKey K, EInputEvent E, double Value = 1) {
        if (PC) PC->InputKey(FInputKeyEventArgs::CreateSimulated(K, E, Value));
    };
    auto Tap = [&](FKey K) { Key(K, IE_Pressed); Key(K, IE_Released); };
    auto Shot = [&](const TCHAR* Name) {
        IFileManager::Get().MakeDirectory(*ProbeDirectory, true);
        FScreenshotRequest::RequestScreenshot(ProbeDirectory / Name, false, false);
    };
    auto CVar = [](const TCHAR* Name) { return IConsoleManager::Get().FindConsoleVariable(Name)->GetFloat(); };
    auto TextureDetail = [&](UStaticMeshComponent* Mesh, const TCHAR* Label) {
        TArray<UTexture*> Textures;
        if (Mesh) Mesh->GetUsedTextures(Textures, EMaterialQualityLevel::High);
        int32 Count = 0;
        for (auto* Texture : Textures) if (auto* Image = Cast<UTexture2D>(Texture)) {
            ++Count;
            UE_LOG(LogTemp, Display, TEXT("SHARP_TEXTURE %s %s size=%dx%d mips=%d/%d"),
                Label, *Image->GetName(), Image->GetSizeX(), Image->GetSizeY(), Image->GetNumResidentMips(), Image->GetNumMips());
            Check(Image->GetNumResidentMips() == Image->GetNumMips(), TEXT("full texture detail in ordinary flight"));
        }
        Check(Count > 0, Label);
    };
    if (ProbeFrame == 120)
    {
        Check(bReady && Pawn && Profiles.Num() == 3, TEXT("scene and three drives ready"));
        const FIntPoint Actual = GEngine->GameViewport->Viewport->GetSizeXY();
        const FIntPoint Expected = FParse::Param(FCommandLine::Get(), TEXT("SolarNative"))
            ? GEngine->GetGameUserSettings()->GetDesktopResolution() : FIntPoint(3840, 2160);
        Check(Actual == Expected, TEXT("viewport equals native display or explicit 4K"));
        Check(CVar(TEXT("EnableHighDPIAwareness")) == 1, TEXT("Windows high DPI enabled before window creation"));
        Check(CVar(TEXT("r.ScreenPercentage")) == 100 && CVar(TEXT("r.SecondaryScreenPercentage.GameViewport")) == 100
            && CVar(TEXT("r.DynamicRes.OperationMode")) == 0, TEXT("no resolution upscaling"));
        Check(CVar(TEXT("r.AntiAliasingMethod")) == 0 && CVar(TEXT("r.MotionBlurQuality")) == 0
            && CVar(TEXT("r.DepthOfFieldQuality")) == 0 && CVar(TEXT("r.SceneColorFringeQuality")) == 0,
            TEXT("no temporal FXAA motion focus or colour blur"));
        UE_LOG(LogTemp, Display, TEXT("SHARP_VIEWPORT %dx%d"), Actual.X, Actual.Y);
        TextureDetail(Ship, TEXT("ship-near")); TextureDetail(Earth, TEXT("earth-normal-flight")); TextureDetail(Sky, TEXT("sky"));
        Shot(TEXT("sharp-near.png"));
    }
    if (ProbeFrame == 140) PC->ConsoleCommand(TEXT("r.AntiAliasingMethod 1"));
    if (ProbeFrame == 155) Shot(TEXT("comparison-fxaa-near.png"));
    if (ProbeFrame == 175) { ApplySharpRenderingSettings(); for (int32 I = 0; I < 18; ++I) Tap(EKeys::MouseScrollDown); }
    if (ProbeFrame == 220) { Check(Pawn && Pawn->CameraDistanceMetres == 6000, TEXT("actual wheel zoom far")); TextureDetail(Ship, TEXT("ship-far")); Shot(TEXT("sharp-far.png")); }
    if (ProbeFrame == 240) PC->ConsoleCommand(TEXT("r.AntiAliasingMethod 1"));
    if (ProbeFrame == 255) Shot(TEXT("comparison-fxaa-far.png"));
    if (ProbeFrame == 275) { ApplySharpRenderingSettings(); Key(EKeys::RightMouseButton, IE_Pressed); }
    if (ProbeFrame >= 280 && ProbeFrame <= 310) Key(EKeys::MouseX, IE_Axis, 20);
    if (ProbeFrame == 305) Shot(TEXT("sharp-moving-orbit.png"));
    if (ProbeFrame == 315) Key(EKeys::RightMouseButton, IE_Released);
    if (ProbeFrame == 340)
    {
        // Isolated high-speed input course, far from a planet, via the domain.
        auto Start = State; Start.PositionMetres = FVector3d(0, -2e12, 0); Start.YawDegrees = 0; Start.PitchDegrees = 0;
        FString Error; Check(Flight.Initialize(Profiles[1].Config, Start, Bodies, Error), TEXT("clear sublight input course"));
        ProfileIndex = 1; if (Pawn) Pawn->ResetCamera();
    }
    if (ProbeFrame == 350) Tap(EKeys::R);
    if (ProbeFrame == 535) Check(State.Throttle == 1 && FMath::Abs(State.VelocityMetresPerSecond.Size() - 250000) < .01, TEXT("R starts ordinary impulse within three seconds"));
    if (ProbeFrame == 540 || ProbeFrame == 740 || ProbeFrame == 1220) Key(EKeys::LeftShift, IE_Pressed);
    if (ProbeFrame == 542 || ProbeFrame == 742 || ProbeFrame == 1222) Key(EKeys::R, IE_Pressed);
    if (ProbeFrame == 544 || ProbeFrame == 744 || ProbeFrame == 1224) Key(EKeys::R, IE_Released);
    if (ProbeFrame == 546 || ProbeFrame == 746 || ProbeFrame == 1226) Key(EKeys::LeftShift, IE_Released);
    if (ProbeFrame == 550) { Check(ProfileIndex == 2 && State.Throttle == 1, TEXT("Shift R engages full impulse without plain R stop")); Key(EKeys::D, IE_Pressed); }
    if (ProbeFrame == 730)
    {
        Check(ProfileIndex == 2 && FMath::Abs(State.VelocityMetresPerSecond.Size() - 250000000) < .01, TEXT("full sublight reaches 250000 km per second in three seconds"));
        Check(FVector3d::CrossProduct(State.Forward(), State.VelocityMetresPerSecond).Size() < 1e-5, TEXT("full sublight turn has no drift"));
        Key(EKeys::D, IE_Released); Shot(TEXT("full-impulse.png"));
    }
    if (ProbeFrame == 750) Check(ProfileIndex == 1 && State.VelocityMetresPerSecond.Size() > 240000000, TEXT("second Shift R smoothly disengages full impulse"));
    if (ProbeFrame == 1180) Check(ProfileIndex == 1 && FMath::Abs(State.VelocityMetresPerSecond.Size() - 250000) < .01, TEXT("return to ordinary impulse completes without speed snap"));
    if (ProbeFrame == 1190) { Tap(EKeys::R); }
    if (ProbeFrame == 1195) Check(State.Throttle == 0, TEXT("R stop works after downshift"));
    if (ProbeFrame == 1200) Tap(EKeys::M);
    if (ProbeFrame == 1230) { Check(Galaxy.bOpen && ProfileIndex == 1 && State.Throttle == 0, TEXT("map prevents Shift R from commanding ship")); Tap(EKeys::M); }
    if (ProbeFrame == 1240) { Tap(EKeys::One); Tap(EKeys::R); }
    if (ProbeFrame == 1250) { Check(ProfileIndex == 0, TEXT("harbour selectable while coasting")); Key(EKeys::SpaceBar, IE_Pressed); }
    if (ProbeFrame == 1630) { Check(State.Throttle == 0 && State.VelocityMetresPerSecond.IsNearlyZero(.01), TEXT("Space brake works after live profile change")); Key(EKeys::SpaceBar, IE_Released); }
    if (ProbeFrame == 1650)
    {
        Check(Flight.GetError().IsEmpty(), TEXT("no domain errors after input sequence"));
        for (const TCHAR* Name : {TEXT("sharp-near.png"), TEXT("sharp-far.png"), TEXT("sharp-moving-orbit.png"), TEXT("full-impulse.png")})
            Check(IFileManager::Get().FileSize(*(ProbeDirectory / Name)) > 100, TEXT("actual render written"));
        const auto Size = GEngine->GameViewport->Viewport->GetSizeXY();
        const FString Result = FString::Printf(TEXT("{\"passed\":%s,\"width\":%d,\"height\":%d,\"frames\":%d}"), bProbePassed ? TEXT("true") : TEXT("false"), Size.X, Size.Y, ProbeFrame);
        FFileHelper::SaveStringToFile(Result, *(ProbeDirectory / TEXT("result.json")));
        FPlatformMisc::RequestExitWithStatus(false, bProbePassed ? 0 : 1);
    }
}

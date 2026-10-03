#include "Solar/SolarFlightGameMode.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "UnrealClient.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Engine/Engine.h"
#include "Camera/CameraComponent.h"

void ASolarFlightGameMode::TickProbe()
{
    ++ProbeFrame;
    // Packaged engines can compile out FApp fixed-time support. This engine
    // setting applies to the world and camera as well as domain advancement.
    if (GEngine) { GEngine->bUseFixedFrameRate = true; GEngine->FixedFrameRate = 60; }
    auto* PC=GetWorld()->GetFirstPlayerController();
    auto Key=[&](const FKey& K,EInputEvent Event,double V=1){if(PC)PC->InputKey(FInputKeyEventArgs::CreateSimulated(K,Event,V));else bProbePassed=false;};
    auto Tap=[&](const FKey& K){Key(K,IE_Pressed);Key(K,IE_Released);};
    auto Check=[&](bool Pass,const TCHAR* Label){bProbePassed&=Pass;UE_LOG(LogTemp,Display,TEXT("SOLAR_PROBE %s %s clock=%.3f yaw=%.3f pitch=%.3f speed=%.3f"),Label,Pass?TEXT("PASS"):TEXT("FAIL"),Flight.GetState().SimulationSeconds,Flight.GetState().YawDegrees,Flight.GetState().PitchDegrees,Flight.GetState().VelocityMetresPerSecond.Size());};
    auto Shot=[&](const TCHAR* File){IFileManager::Get().MakeDirectory(*ProbeDirectory,true);FScreenshotRequest::RequestScreenshot(FPaths::Combine(ProbeDirectory,File),false,false);};
    auto* Pawn=PC?Cast<ASolarFlightPawn>(PC->GetPawn()):nullptr;
    const auto& S=Flight.GetState();
    switch(ProbeFrame)
    {
    case 30:Check(bReady && Ship && Earth && Sun && StarCount()==8920,TEXT("staged model Earth Sun stars"));Shot(TEXT("earth.png"));break;
    case 40:Tap(EKeys::E);break;
    case 45:Check(FMath::Abs(S.Throttle-.2)<.001,TEXT("E persistent20pct"));Tap(EKeys::R);break;
    case 50:Check(S.Throttle==0,TEXT("R stops throttle"));Tap(EKeys::R);break;
    case 70:ProbeYaw=S.YawDegrees;Key(EKeys::D,IE_Pressed);break;
    case 200:Key(EKeys::D,IE_Released);Check(S.YawDegrees>ProbeYaw+8,TEXT("D yaw right"));Shot(TEXT("turn.png"));break;
    case 230:ProbePitch=S.PitchDegrees;Key(EKeys::W,IE_Pressed);break;
    case 300:Key(EKeys::W,IE_Released);Check(S.PitchDegrees>ProbePitch+4,TEXT("W pitch up"));break;
    case 350:Check(S.VelocityMetresPerSecond.Size()>100,TEXT("forward acceleration"));Key(EKeys::SpaceBar,IE_Pressed);break;
    case 610:Key(EKeys::SpaceBar,IE_Released);Check(S.VelocityMetresPerSecond.Size()<.1 && S.Throttle==0,TEXT("Space gradual brake"));break;
    case 615:Tap(EKeys::Q);Tap(EKeys::Q);break;
    case 680:Check(S.Throttle==-.25 && FVector3d::DotProduct(S.Forward(),S.VelocityMetresPerSecond)<0,TEXT("Q reverse clamp"));Tap(EKeys::BackSpace);break;
    case 690:Check(S.PositionMetres==InitialState.PositionMetres && S.Throttle==0,TEXT("reset"));Tap(EKeys::Two);break;
    case 695:Check(ProfileIndex==1,TEXT("fast local profile"));Tap(EKeys::One);Tap(EKeys::P);break;
    case 700:Check(bPaused && ProfileIndex==0,TEXT("profile1 and pause"));ProbeClock=S.SimulationSeconds;break;
    case 720:Check(S.SimulationSeconds==ProbeClock,TEXT("paused clock"));Tap(EKeys::P);Key(EKeys::RightMouseButton,IE_Pressed);break;
    case 722:Key(EKeys::MouseX,IE_Axis,30);break;
    case 725:Key(EKeys::RightMouseButton,IE_Released);Tap(EKeys::MouseScrollUp);break;
    case 750:Check(Pawn && Pawn->CameraDistanceMetres<1100 && FMath::Abs(Pawn->Camera->GetComponentRotation().Roll)<.001,TEXT("zoom and level horizon"));Tap(EKeys::Home);break;
    case 790:Check(Pawn && FMath::Abs(FRotator::NormalizeAxis(Pawn->Camera->GetComponentRotation().Yaw-S.YawDegrees))<1,TEXT("Home behind ship"));
        // Isolated diagnostic reposition changes domain through Initialize,
        // not by moving a visual. Face the actual Sun for visual review.
        {auto Start=S;Start.YawDegrees=(SunPosition-Start.PositionMetres).Rotation().Yaw;Start.PitchDegrees=0;FString E;Check(Flight.Initialize(Profiles[0].Config,Start,Bodies,E),TEXT("sun look diagnostic"));}break;
    case 850:Shot(TEXT("sun.png"));break;
    case 880:
        Check(Flight.GetPendingSeconds()<.02 && Flight.GetError().IsEmpty(),TEXT("stable fixedstep"));
        Check(IFileManager::Get().FileSize(*FPaths::Combine(ProbeDirectory,TEXT("earth.png")))>100 && IFileManager::Get().FileSize(*FPaths::Combine(ProbeDirectory,TEXT("turn.png")))>100 && IFileManager::Get().FileSize(*FPaths::Combine(ProbeDirectory,TEXT("sun.png")))>100,TEXT("rendered images written"));
        bProbePassed=FFileHelper::SaveStringToFile(FString::Printf(TEXT("{\"passed\":%s,\"stars\":%d,\"profile\":%d,\"frames\":%d}"),bProbePassed?TEXT("true"):TEXT("false"),StarCount(),ProfileIndex,ProbeFrame),*FPaths::Combine(ProbeDirectory,TEXT("result.json"))) && bProbePassed;
        FPlatformMisc::RequestExitWithStatus(false,bProbePassed?0:1);break;
    }
}

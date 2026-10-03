#include "Presentation/DaedalusGameMode.h"
#include "World/DaedalusWorldSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "UnrealClient.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

// Explicit opt-in diagnostic, with the launcher supplying an isolated save directory.
// Sends real controller key events through the configured bindings, rather than
// calling pawn actions directly. Screenshots are the rendered game viewport.
void ADaedalusGameMode::TickVisualProbe()
{
    ++ProbeFrame;
    APlayerController* Controller = GetWorld()->GetFirstPlayerController();
    auto Key = [&](const FKey& Value, EInputEvent Event)
    { if (Controller) Controller->InputKey(FInputKeyEventArgs::CreateSimulated(Value, Event, 1)); else bProbePassed = false; };
    auto Check = [&](bool Ok, const TCHAR* Label)
    {
        bProbePassed &= Ok;
        UE_LOG(LogTemp, Display, TEXT("DAEDALUS_VISUAL %s %s"), Label, Ok ? TEXT("PASS") : TEXT("FAIL"));
    };
    auto Screenshot = [&](const TCHAR* Name)
    {
        IFileManager::Get().MakeDirectory(*ProbeDirectory, true);
        FScreenshotRequest::RequestScreenshot(FPaths::Combine(ProbeDirectory, Name), false, false);
    };
    const auto& State = Domain->Simulation.GetSnapshot();
    const auto& Player = State.Ships.FindChecked(State.PlayerShipId);
    switch (ProbeFrame)
    {
    case 30: Check(bVisualAssetsValid && Ships.Num() == 2, TEXT("initial models")); Screenshot(TEXT("space.png")); break;
    case 40: Key(EKeys::W, IE_Pressed); break;
    case 65: Key(EKeys::W, IE_Released); break;
    case 70: Check(Player.PositionMetres.X > 0, TEXT("W movement binding")); Key(EKeys::SpaceBar, IE_Pressed); break;
    case 71: Key(EKeys::SpaceBar, IE_Released); break;
    case 75: Check(State.Ships.FindChecked(TEXT("ship.origin.target")).Shield < 50, TEXT("SPACE fire binding")); Key(EKeys::F5, IE_Pressed); break;
    case 76: Key(EKeys::F5, IE_Released); break;
    case 80: Check(Domain->LastMessage.StartsWith(TEXT("Saved:")), TEXT("F5 save binding")); break;
    case 85: Key(EKeys::T, IE_Pressed); break;
    case 86: Key(EKeys::T, IE_Released); break;
    case 90: Check(Player.SystemId == TEXT("fixture.remote"), TEXT("T travel binding")); Key(EKeys::B, IE_Pressed); break;
    case 91: Key(EKeys::B, IE_Released); break;
    case 100: Check(State.PlayerLocationId == TEXT("fixture.station") && Ships.Num() == 0, TEXT("B transport binding")); Screenshot(TEXT("location.png")); break;
    case 110: Key(EKeys::B, IE_Pressed); break;
    case 111: Key(EKeys::B, IE_Released); break;
    case 115: Check(State.PlayerLocationId.IsEmpty(), TEXT("B return binding")); Key(EKeys::F9, IE_Pressed); break;
    case 116: Key(EKeys::F9, IE_Released); break;
    case 120: Check(Player.SystemId == TEXT("fixture.origin") && State.PlayerLocationId.IsEmpty() && State.Ships.FindChecked(TEXT("ship.origin.target")).Shield < 50, TEXT("F9 load binding")); Key(EKeys::P, IE_Pressed); break;
    case 121: Key(EKeys::P, IE_Released); break;
    case 125: Check(State.bPaused, TEXT("P pause binding")); ProbePausedTime = State.SimulationSeconds; break;
    case 135: Check(State.SimulationSeconds == ProbePausedTime, TEXT("pause clock")); Key(EKeys::P, IE_Pressed); break;
    case 136: Key(EKeys::P, IE_Released); break;
    case 140:
    {
        FString Error;
        Check(!State.bPaused && Domain->Simulation.SpawnShip(TEXT("probe.dynamic"), TEXT("fixture.target"), Player.SystemId, FVector3d(300,300,0), Error), TEXT("dynamic spawn"));
        break;
    }
    case 145:
    {
        Check(bVisualAssetsValid && Ships.Num() == 3 && Ships.Contains(TEXT("probe.dynamic")), TEXT("dynamic presentation reconciled"));
        Check(IFileManager::Get().FileSize(*FPaths::Combine(ProbeDirectory, TEXT("space.png"))) > 100 &&
            IFileManager::Get().FileSize(*FPaths::Combine(ProbeDirectory, TEXT("location.png"))) > 100, TEXT("rendered screenshots written"));
        const FString Result = FString::Printf(TEXT("{\"passed\":%s,\"frames\":%d}"), bProbePassed ? TEXT("true") : TEXT("false"), ProbeFrame);
        bProbePassed = FFileHelper::SaveStringToFile(Result, *FPaths::Combine(ProbeDirectory, TEXT("result.json"))) && bProbePassed;
        FPlatformMisc::RequestExitWithStatus(false, bProbePassed ? 0 : 1);
        break;
    }
    }
}

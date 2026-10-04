#include "Solar/SolarFlightGameMode.h"
#include "GameFramework/PlayerController.h"

void ASolarFlightGameMode::ToggleMap()
{
    if(!bReady)return;
    Galaxy.bOpen=!Galaxy.bOpen;
    Galaxy.bSearchFocused=false;
    Flight.SetPaused(bPaused || Galaxy.bOpen);
    FString Error; Flight.SetInput({},Error);
    if(auto* PC=GetWorld()->GetFirstPlayerController())
    {
        PC->bShowMouseCursor=Galaxy.bOpen;
        if(Galaxy.bOpen)
        {
            FInputModeGameAndUI Mode;
            Mode.SetHideCursorDuringCapture(false);
            Mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
            PC->SetInputMode(Mode);
        }
        else PC->SetInputMode(FInputModeGameOnly());
    }
    RefreshNavigation();
}

void ASolarFlightGameMode::RefreshNavigation()
{
    if(!bReady || !Systems.IsValidIndex(ActiveSystem))return;
    FString Error;
    const auto& State=Flight.GetState();
    if(!Navigation.SetLocation(Systems[ActiveSystem].Id,State.PositionMetres,Error)
        || !PreviewNavigation.SetLocation(Systems[ActiveSystem].Id,State.PositionMetres,Error))Message=Error;
    if(MapSystems.IsValidIndex(Galaxy.SelectedSystem) && MapSystems[Galaxy.SelectedSystem].Bodies.IsValidIndex(Galaxy.SelectedBody))
    {
        const auto& Sys=MapSystems[Galaxy.SelectedSystem];const auto& Body=Sys.Bodies[Galaxy.SelectedBody];
        if(Body.bKnownPosition)PreviewNavigation.SelectTarget(Sys.Id,Body.Id,Error);
    }
}

Daedalus::FNavigationMetrics ASolarFlightGameMode::PreviewMetrics() const
{
    if(!MapSystems.IsValidIndex(Galaxy.SelectedSystem) || !MapSystems[Galaxy.SelectedSystem].Bodies.IsValidIndex(Galaxy.SelectedBody)
        || !MapSystems[Galaxy.SelectedSystem].Bodies[Galaxy.SelectedBody].bKnownPosition)return {};
    return PreviewNavigation.Query(Flight.GetState().VelocityMetresPerSecond.Size(),Flight.GetConfig().MaxSpeed,Galaxy.DesiredSeconds);
}

void ASolarFlightGameMode::HandleMapAction(const FMapAction& Action)
{
    if(Action.Type==FMapAction::EType::None)return;
    if(!MapSystems.IsValidIndex(Action.SystemIndex) || !MapSystems[Action.SystemIndex].Bodies.IsValidIndex(Action.BodyIndex))return;
    const auto& Sys=MapSystems[Action.SystemIndex];const auto& Body=Sys.Bodies[Action.BodyIndex];
    FString Error;
    if(!Body.bKnownPosition){Message=TEXT("Poloha tohoto tělesa není známá.");return;}
    if(Action.Type==FMapAction::EType::Navigate)
    {
        if(!Sys.bAvailable){Message=TEXT("Soustava čeká na převzetí podkladů.");return;}
        if(Navigation.SelectTarget(Sys.Id,Body.Id,Error))
        {
            if(Action.SystemIndex==ActiveSystem)SelectedBody=Action.BodyIndex;
            Message=TEXT("Navigační cíl vybrán. Pohon pro mezihvězdný let navrhneme později.");
        }
        else Message=Error;
    }
    else if(Action.Type==FMapAction::EType::Inspect)
    {
        if(ActivateSystem(Action.SystemIndex,Action.BodyIndex) && Galaxy.bOpen)ToggleMap();
    }
    RefreshNavigation();
}

#include "Solar/SolarFlightGameMode.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"

void ASolarFlightGameMode::TickAncientHUDProbe()
{
    ++ProbeFrame; GEngine->bUseFixedFrameRate=true; GEngine->FixedFrameRate=60;
    FString Phase;FParse::Value(FCommandLine::Get(),TEXT("SolarAncientHUDProbe="),Phase);
    const bool Write=Phase==TEXT("write");
    auto* PC=GetWorld()->GetFirstPlayerController();
    auto Check=[&](bool Pass,const TCHAR* Label){bProbePassed&=Pass;UE_LOG(LogTemp,Display,TEXT("ANCIENT_HUD_PROBE %s %s"),Label,Pass?TEXT("PASS"):TEXT("FAIL"));};
    auto Key=[&](FKey K,EInputEvent E){PC->InputKey(FInputKeyEventArgs::CreateSimulated(K,E,1));};
    auto Tap=[&](FKey K){Key(K,IE_Pressed);Key(K,IE_Released);};
    auto Click=[&](ESolarPauseAction Action){
        const auto V=BuildSolarPauseView(*this,FVector2D(3840,2160));
        const auto* B=V.Buttons.FindByPredicate([&](const auto& Entry){return Entry.Action==Action&&Entry.bEnabled;});
        if(!B){Check(false,TEXT("requested settings action exists and is enabled"));return;}
        const auto C=B->Pixels.GetCenter();PC->SetMouseLocation(C.X,C.Y);Key(EKeys::LeftMouseButton,IE_Pressed);
    };
    auto Shot=[&](const TCHAR* Name){IFileManager::Get().MakeDirectory(*ProbeDirectory,true);FScreenshotRequest::RequestScreenshot(ProbeDirectory/Name,true,false);};
    auto Finish=[&](){
        Check(Flight.GetState().PositionMetres==InitialState.PositionMetres&&Flight.GetState().VelocityMetresPerSecond.IsZero(),TEXT("style/menu changes never mutate flight position or velocity"));
        Check(Flight.GetConfig().TurnRateDegrees==6.8&&Flight.GetConfig().MaxSpeed==350000,TEXT("Aurora flight configuration unchanged"));
        const FString Json=FString::Printf(TEXT("{\"passed\":%s,\"width\":3840,\"height\":2160,\"phase\":\"%s\"}"),bProbePassed?TEXT("true"):TEXT("false"),*Phase);
        FFileHelper::SaveStringToFile(Json,*(ProbeDirectory/TEXT("result.json")));
        FPlatformMisc::RequestExitWithStatus(false,bProbePassed?0:1);
    };
    if(!bReady||!PC||(Phase!=TEXT("write")&&Phase!=TEXT("read")))
    {
        FFileHelper::SaveStringToFile(TEXT("{\"passed\":false,\"error\":\"scene or probe phase invalid\"}"),*(ProbeDirectory/TEXT("result.json")));
        FPlatformMisc::RequestExitWithStatus(false,1);return;
    }
    if(ProbeFrame==20)
    {
        Check(!UsesAncientHUD()&&!HasAncientInterface(),TEXT("Daedalus keeps Earth HUD"));
        Check(SelectShip(1)&&HasAncientInterface(),TEXT("Aurora has mapped Ancient interface family"));
        Check(GEngine->GameViewport->Viewport->GetSizeXY()==FIntPoint(3840,2160),TEXT("actual native 4K output"));
        Check(UsesAncientHUD()==Write,Write?TEXT("new profile defaults to Ancient HUD"):TEXT("Earth preference survived a separate-process restart"));
    }
    if(ProbeFrame==40){Tap(EKeys::P);ProbeClock=Flight.GetState().SimulationSeconds;}
    if(ProbeFrame==45)Click(ESolarPauseAction::Settings);
    if(ProbeFrame==48)Key(EKeys::LeftMouseButton,IE_Released);
    if(ProbeFrame==50)
    {
        Check(bPaused&&PauseMenuPage==ESolarPausePage::Settings,TEXT("P settings exposes both HUD choices"));
        if(Write)Shot(TEXT("ancient-settings.png"));
    }
    if(ProbeFrame==55)Click(Write?ESolarPauseAction::EarthHUD:ESolarPauseAction::AncientHUD);
    if(ProbeFrame==58)Key(EKeys::LeftMouseButton,IE_Released);
    if(ProbeFrame==62)
    {
        Check(UsesAncientHUD()!=Write&&bPaused&&Flight.GetState().SimulationSeconds==ProbeClock,TEXT("mouse switches style without advancing paused flight"));
        Tap(EKeys::P);
    }
    if(!Write)
    {
        if(ProbeFrame==80)Shot(TEXT("ancient-after-restart.png"));
        if(ProbeFrame==90){Check(IFileManager::Get().FileSize(*(ProbeDirectory/TEXT("ancient-after-restart.png")))>100,TEXT("restored Ancient render saved"));Finish();}
        return;
    }
    if(ProbeFrame==75)Shot(TEXT("earth-aurora.png"));
    if(ProbeFrame==85){Tap(EKeys::P);ProbeClock=Flight.GetState().SimulationSeconds;}
    if(ProbeFrame==90)Click(ESolarPauseAction::Settings);
    if(ProbeFrame==93)Key(EKeys::LeftMouseButton,IE_Released);
    if(ProbeFrame==95)Click(ESolarPauseAction::AncientHUD);
    if(ProbeFrame==98)Key(EKeys::LeftMouseButton,IE_Released);
    if(ProbeFrame==102){Check(UsesAncientHUD()&&Flight.GetState().SimulationSeconds==ProbeClock,TEXT("switching back to Ancient preserves pause"));Tap(EKeys::P);}
    if(ProbeFrame==115)Shot(TEXT("ancient-earth.png"));
    if(ProbeFrame==120)Key(EKeys::RightMouseButton,IE_Pressed);
    if(ProbeFrame>=125&&ProbeFrame<=160)PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseX,IE_Axis,18));
    if(ProbeFrame==165)Key(EKeys::RightMouseButton,IE_Released);
    if(ProbeFrame==180)Shot(TEXT("ancient-space.png"));
    if(ProbeFrame==190)Tap(EKeys::M);
    if(ProbeFrame==195){Check(Galaxy.bOpen,TEXT("Ancient map remains operational"));Shot(TEXT("ancient-map.png"));}
    if(ProbeFrame==200)Tap(EKeys::M);
    if(ProbeFrame==205){Tap(EKeys::P);ProbeClock=Flight.GetState().SimulationSeconds;}
    if(ProbeFrame==210)Click(ESolarPauseAction::Settings);
    if(ProbeFrame==213)Key(EKeys::LeftMouseButton,IE_Released);
    if(ProbeFrame==215)Click(ESolarPauseAction::EarthHUD);
    if(ProbeFrame==218)Key(EKeys::LeftMouseButton,IE_Released);
    if(ProbeFrame==225){Check(!UsesAncientHUD()&&Flight.GetState().SimulationSeconds==ProbeClock,TEXT("Earth choice retained for restart"));Tap(EKeys::P);}
    if(ProbeFrame==240)Check(SelectShip(0)&&!UsesAncientHUD(),TEXT("ship swap cannot apply Ancient style to Daedalus"));
    if(ProbeFrame==245){Tap(EKeys::P);Check(!SetAncientHUD(true),TEXT("Earth ship cannot change Ancient preference"));}
    if(ProbeFrame==249)Tap(EKeys::P);
    if(ProbeFrame==250)Check(SelectShip(1)&&!UsesAncientHUD(),TEXT("Aurora preference survives intervening Daedalus swap"));
    if(ProbeFrame==260)
    {
        for(const TCHAR* Name:{TEXT("ancient-settings.png"),TEXT("earth-aurora.png"),TEXT("ancient-earth.png"),TEXT("ancient-space.png"),TEXT("ancient-map.png")})
            Check(IFileManager::Get().FileSize(*(ProbeDirectory/Name))>100,TEXT("native render saved"));
        Finish();
    }
}

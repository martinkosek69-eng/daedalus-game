#include "Solar/SolarFlightGameMode.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "UnrealClient.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"

void ASolarFlightGameMode::TickGalaxyProbe()
{
    auto* PC=GetWorld()->GetFirstPlayerController();
    auto Key=[&](const FKey& K,EInputEvent E,double V=1){PC->InputKey(FInputKeyEventArgs::CreateSimulated(K,E,V));};
    auto Tap=[&](const FKey& K){Key(K,IE_Pressed);Key(K,IE_Released);};
    auto Check=[&](bool P,const TCHAR* Label){bProbePassed&=P;UE_LOG(LogTemp,Display,TEXT("GALAXY_PROBE %s %s frame=%d"),Label,P?TEXT("PASS"):TEXT("FAIL"),ProbeFrame);};
    auto Shot=[&](const FString& Name){FScreenshotRequest::RequestScreenshot(FPaths::Combine(ProbeDirectory,Name),false,false);};
    int32 W=0,H=0;PC->GetViewportSize(W,H);const double Scale=FMath::Min(W/1280.,H/720.);
    auto Click=[&](double X,double Y){PC->SetMouseLocation(FMath::RoundToInt(X*Scale),FMath::RoundToInt(Y*Scale));Key(EKeys::LeftMouseButton,IE_Pressed);};
    auto ViewInput=[&](FVector2D Point,FVector2D Delta,bool Left,bool Right,bool Middle,float Wheel,bool Home){
        HandleMapAction(Galaxy.Input(Point*Scale,Delta,Left,Right,Middle,Wheel,Home,MapSystems,FVector2D(W,H)));};
    const double Width=W/Scale,Height=H/Scale;
    switch(ProbeFrame)
    {
    case 2155:ProbeClock=Flight.GetState().SimulationSeconds;Tap(EKeys::M);break;
    case 2160:ProbeClock=Flight.GetState().SimulationSeconds;Check(Galaxy.bOpen && PC->bShowMouseCursor,TEXT("M opens3D map and mouse cursor"));Shot(TEXT("galaxy-oblique.png"));break;
    case 2180:Check(Flight.GetState().SimulationSeconds==ProbeClock,TEXT("map pauses flight clock"));Click(Width-250,33);break;
    case 2182:Key(EKeys::LeftMouseButton,IE_Released);break;
    case 2190:Check(Galaxy.GetCameraPitchDegrees()==89,TEXT("actual controller top view click"));Shot(TEXT("galaxy-top.png"));break;
    case 2210:Click(Width-160,33);break;
    case 2212:Key(EKeys::LeftMouseButton,IE_Released);break;
    case 2220:Check(Galaxy.GetCameraPitchDegrees()==0,TEXT("actual controller side view click"));Shot(TEXT("galaxy-side.png"));break;
    case 2240:ViewInput(FVector2D(500,300),FVector2D(120,-90),false,true,false,0,false);break;
    case 2245:{Check(Galaxy.GetCameraPitchDegrees()>20,TEXT("3D orbit"));
        const auto PreviousPivot = Galaxy.GetPivotLY();
        ViewInput(FVector2D(500,300),FVector2D(80,40),false,false,true,0,false);
        Check(Galaxy.GetPivotLY() != PreviousPivot, TEXT("3D middle-mouse pan"));
        ViewInput(FVector2D(500,300),{},false,false,false,3,false);break;}
    case 2250:Check(Galaxy.GetCameraDistanceLY()<155000,TEXT("map exponential zoom"));
        Galaxy.SelectedSystem=0;Galaxy.FocusSystem(0,MapSystems);break;
    case 2255:Check(Galaxy.GetPivotLY()==MapSystems[0].GalaxyLightYears && Galaxy.GetCameraDistanceLY()<.01,TEXT("galactic anchor with local system precision"));Shot(TEXT("solar-system-map.png"));
        Click(Width-190,158);break;
    case 2257:Key(EKeys::LeftMouseButton,IE_Released);break;
    case 2260:Check(Galaxy.bSearchFocused,TEXT("actual database search click"));Tap(EKeys::M);break;
    case 2265:Check(Galaxy.bOpen && Galaxy.SearchQuery==TEXT("M"),TEXT("typingM insearch keepsmapopen"));Tap(EKeys::E);Tap(EKeys::S);break;
    case 2270:Check(Flight.GetState().Throttle==0 && Flight.GetState().SimulationSeconds==ProbeClock,TEXT("search does not command ship"));
        Galaxy.bSearchFocused=false;Galaxy.SetSearchQuery(TEXT("Zem"));break;
    case 2275:Click(Width-190,221);break;
    case 2277:Key(EKeys::LeftMouseButton,IE_Released);break;
    case 2280:Check(MapSystems[0].Bodies[Galaxy.SelectedBody].Id==TEXT("sol.earth"),TEXT("Czech search folded and actual row selection"));
        Click(Width-190,Height-226);break;
    case 2282:Key(EKeys::LeftMouseButton,IE_Released);break;
    case 2285:Check(Galaxy.GetCameraDistanceLY()<1e-8,TEXT("deep planet radius focus"));Shot(TEXT("earth-map-detail.png"));break;
    case 2305:Click(Width-190,Height-55);break;
    case 2307:Key(EKeys::LeftMouseButton,IE_Released);break;
    case 2310:Check(PreviewMetrics().bPlannedETA && FMath::Abs(PreviewMetrics().PlannedETASeconds * Profiles[2].Config.MaxSpeed - PreviewMetrics().DistanceMetres) < 1, TEXT("full impulse ETA uses full sublight speed while ship is in harbour profile"));
        Check(Navigation.GetTargetBodyId()==TEXT("sol.earth") && Flight.GetState().SimulationSeconds==ProbeClock,TEXT("navigation target selection leaves flight unchanged"));
        Galaxy.SelectedBody=MapSystems[0].Bodies.IndexOfByPredicate([](const FGalaxyBodyView& B){return !B.bKnownPosition;});
        Check(Galaxy.SelectedBody!=INDEX_NONE,TEXT("unknown position retained in database"));
        HandleMapAction({FMapAction::EType::Inspect,0,Galaxy.SelectedBody});
        Check(!PreviewMetrics().bValid && Flight.GetState().SimulationSeconds==ProbeClock,TEXT("unknown position blocks metrics and teleport"));
        Galaxy.SelectedBody=0;Galaxy.SetSearchQuery(TEXT(""));break;
    case 2315:Tap(EKeys::Escape);break;
    case 2320:Check(!Galaxy.bOpen && !PC->bShowMouseCursor,TEXT("Esc returns to flight"));break;
    }
    // Each available worker system is activated through the explicit domain test
    // command, never by writing a visual transform. Revisit Sol between calls.
    if(ProbeFrame>=2400 && ProbeFrame<3900)
    {
        const int32 Index=1+(ProbeFrame-2400)/300,Phase=(ProbeFrame-2400)%300;
        if(Systems.IsValidIndex(Index) && Systems[Index].bAvailable)
        {
            if(Phase==0)Check(ActivateSystem(Index,1),TEXT("available external system activation"));
            if(Phase==90){Check(ActiveSystem==Index && BodyMeshes.Num()==Systems[Index].Bodies.Num() && Flight.GetError().IsEmpty(),TEXT("active disposable scene matches canonical worker system"));Shot(Systems[Index].Id+TEXT(".png"));}
            if(Phase==100)Check(ActivateSystem(Index,2),TEXT("second authored planet activation"));
            if(Phase==190)
            {
                TArray<UTexture*> Textures;
                BodyDynamics[2]->GetUsedTextures(Textures, EMaterialQualityLevel::High);
                for (auto* Texture : Textures) if (auto* Image = Cast<UTexture2D>(Texture))
                {
                    Check(Image->GetNumResidentMips() >= Image->GetNumMips() - 1, TEXT("visited planet surface loaded at native detail"));
                    UE_LOG(LogTemp, Display, TEXT("SOLAR_TEXTURE %s size=%dx%d residentMips=%d/%d"),
                        *Image->GetName(), Image->GetSizeX(), Image->GetSizeY(), Image->GetNumResidentMips(), Image->GetNumMips());
                }
                Shot(Systems[Index].Id+TEXT("-second.png"));
            }
            if(Phase==210){ToggleMap();Galaxy.SelectedSystem=Index;Galaxy.SelectedBody=1;Galaxy.FocusSystem(Index,MapSystems);}
            if(Phase==220)Shot(Systems[Index].Id+TEXT("-map.png"));
            if(Phase==250)ToggleMap();
            if(Phase==270)Check(ActivateSystem(0,3),TEXT("scene cleanup and return to Sol"));
        }
    }
    if(ProbeFrame==3950)
    {
        Check(ActiveSystem==0 && BodyDefinitions.Num()==506 && MapSystems.Num()==6 && Flight.GetError().IsEmpty(),TEXT("universe stable after visits"));
        Check(IFileManager::Get().FileSize(*FPaths::Combine(ProbeDirectory,TEXT("galaxy-oblique.png")))>100,TEXT("actual galaxy render written"));
        const FString Result=FString::Printf(TEXT("{\"passed\":%s,\"stars\":%d,\"systems\":%d,\"availableSystems\":%d,\"frames\":%d}"),bProbePassed?TEXT("true"):TEXT("false"),StarCount(),Systems.Num(),Systems.FilterByPredicate([](const FSolarSystemDefinition& S){return S.bAvailable;}).Num(),ProbeFrame);
        bProbePassed=FFileHelper::SaveStringToFile(Result,*FPaths::Combine(ProbeDirectory,TEXT("result.json"))) && bProbePassed;
        FPlatformMisc::RequestExitWithStatus(false,bProbePassed?0:1);
    }
}

#include "Solar/SolarFlightGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "InputKeyEventArgs.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"

void ASolarFlightGameMode::TickAuroraProbe()
{
    ++ProbeFrame; GEngine->bUseFixedFrameRate = true; GEngine->FixedFrameRate = 60;
    auto* PC = GetWorld()->GetFirstPlayerController();
    auto Check = [&](bool Pass, const TCHAR* Label) {
        bProbePassed &= Pass; UE_LOG(LogTemp, Display, TEXT("AURORA_PROBE %s %s"), Label, Pass ? TEXT("PASS") : TEXT("FAIL"));
    };
    auto Key = [&](FKey K, EInputEvent E) { if (PC) PC->InputKey(FInputKeyEventArgs::CreateSimulated(K, E, 1)); };
    auto Tap = [&](FKey K) { Key(K, IE_Pressed); Key(K, IE_Released); };
    auto Click = [&](int32 Row) {
        const auto View = BuildSolarPauseView(*this,FVector2D(3840,2160));
        if(!View.Buttons.IsValidIndex(Row)){Check(false,TEXT("menu row exists"));return;}
        const auto Centre=View.Buttons[Row].Pixels.GetCenter();PC->SetMouseLocation(Centre.X,Centre.Y);Key(EKeys::LeftMouseButton,IE_Pressed);
    };
    auto Shot = [&](const TCHAR* Name) {
        IFileManager::Get().MakeDirectory(*ProbeDirectory, true); FScreenshotRequest::RequestScreenshot(ProbeDirectory / Name, true, false);
    };
    if (!bReady)
    {
        FFileHelper::SaveStringToFile(TEXT("{\"passed\":false,\"error\":\"scene not ready\"}"), *(ProbeDirectory / TEXT("result.json")));
        FPlatformMisc::RequestExitWithStatus(false, 1); return;
    }
    if (ProbeFrame == 30) { Tap(EKeys::P); }
    if (ProbeFrame == 35)
    { Check(bPaused && PC->bShowMouseCursor,TEXT("P opens paused clickable menu"));ProbeClock=Flight.GetState().SimulationSeconds;Shot(TEXT("pause-menu.png")); }
    if (ProbeFrame == 40) Click(1);
    if (ProbeFrame == 43) Key(EKeys::LeftMouseButton,IE_Released);
    if (ProbeFrame == 48)
    {
        Check(ActiveShip == 0 && HangarDoors.Num() == 4, TEXT("latest animated Daedalus remains default"));
        Check(PauseMenuPage==ESolarPausePage::Ships,TEXT("mouse opens ship selection"));Click(1);
    }
    if (ProbeFrame == 51) Key(EKeys::LeftMouseButton,IE_Released);
    if (ProbeFrame == 60)
    {
        Check(ActiveShip == 1 && ShipId == TEXT("aurora") && ShipLengthMetres == 3500, TEXT("menu chooses Aurora definition"));
        Check(Ship && Ship->GetNumMaterials() == 3 && FMath::Abs(Ship->GetStaticMesh()->GetBounds().BoxExtent.X * 2 - 350000) < 50, TEXT("3500 metre source hull and all three materials"));
        Check(bPaused && Flight.GetState().SimulationSeconds == ProbeClock && Flight.GetState().PositionMetres == InitialState.PositionMetres,
            TEXT("paused ship change preserves clock and position"));
        Check(HangarDoors.IsEmpty() && ShipPointLights.IsEmpty() && !HullLights && EngineGlows.IsEmpty(), TEXT("no old ship animation actors on Aurora"));
        Check(Flight.GetConfig().TurnRateDegrees == 6.8 && Flight.GetConfig().ShipRadiusMetres == 1950 && Flight.GetConfig().MaxSpeed == 350000,
            TEXT("Aurora owns distinct canonical flight config"));
        Check(Systems.Num() == 6 && BodyDefinitions.Num() == 506, TEXT("existing planets/navigation intact"));
    }
    if (ProbeFrame == 62) Click(2);
    if (ProbeFrame == 65) Key(EKeys::LeftMouseButton,IE_Released);
    if (ProbeFrame == 70) {Check(PauseMenuPage==ESolarPausePage::Main,TEXT("menu back works"));Click(0);}
    if (ProbeFrame == 73) Key(EKeys::LeftMouseButton,IE_Released);
    if (ProbeFrame == 120)
    {
        Check(GEngine->GameViewport->Viewport->GetSizeXY() == FIntPoint(3840,2160), TEXT("actual native 4K ship and HUD"));
        TArray<UTexture*> Textures; Ship->GetUsedTextures(Textures, EMaterialQualityLevel::High);
        int32 AtlasCount = 0;
        for (auto* Texture : Textures) if (auto* Image = Cast<UTexture2D>(Texture))
        {
            const bool bColour = Image->GetName() == TEXT("T_Aurora_BaseColor");
            if (bColour) ++AtlasCount;
            Check(Image->GetSizeX() >= (bColour ? 8192 : 4096) && Image->GetNumResidentMips() == Image->GetNumMips(),
                TEXT("cooked8K/4K source textures retain full resident detail"));
        }
        Check(AtlasCount == 1, TEXT("authored8K colour atlas bound to real rendered hull"));
        Shot(TEXT("aurora-hud.png")); Key(EKeys::RightMouseButton,IE_Pressed);
    }
    if (ProbeFrame >= 122 && ProbeFrame <= 160) PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::MouseX,IE_Axis,18));
    if (ProbeFrame == 162) { Key(EKeys::RightMouseButton,IE_Released); Tap(EKeys::R); }
    if (ProbeFrame == 285)
    {
        Check(FMath::Abs(Flight.GetState().VelocityMetresPerSecond.Size()-350000) < .01 && Flight.GetState().Throttle == 1, TEXT("Aurora impulse reaches max within two seconds"));
        Shot(TEXT("aurora-front.png")); Key(EKeys::A,IE_Pressed);
    }
    if (ProbeFrame == 330)
    {
        const auto& State = Flight.GetState();
        Check(FVector3d::CrossProduct(State.Forward(),State.VelocityMetresPerSecond.GetSafeNormal()).Size()<1e-9, TEXT("actual steering has no sideways drift"));
        Key(EKeys::A,IE_Released);
    }
    if (ProbeFrame == 335) Tap(EKeys::P);
    if (ProbeFrame == 340)
    {
        ProbeClock=Flight.GetState().SimulationSeconds;Click(1);
    }
    if(ProbeFrame==343)Key(EKeys::LeftMouseButton,IE_Released);
    if(ProbeFrame==350)Click(0);
    if(ProbeFrame==353)Key(EKeys::LeftMouseButton,IE_Released);
    if(ProbeFrame==360)
    {
        Check(ActiveShip == 0 && HangarDoors.Num()==4 && EngineGlows.Num()==6 && ShipPointLights.Num()==10,
            TEXT("menu restores latest Daedalus with animations and lights"));
        Check(Flight.GetState().SimulationSeconds==ProbeClock && Flight.GetState().VelocityMetresPerSecond.Size()>0, TEXT("moving swap preserves paused flight state"));
    }
    if(ProbeFrame==365)Tap(EKeys::P);
    if(ProbeFrame==368)Tap(EKeys::H);
    if (ProbeFrame == 495)
    {
        Check(bHangarsOpen && DoorTravel==1, TEXT("Daedalus H animation still works after swaps"));
        Shot(TEXT("daedalus-restored.png"));
        Tap(EKeys::M);
    }
    if (ProbeFrame == 510) { Check(!SelectShip(1) && ActiveShip==0 && Galaxy.bOpen,TEXT("map blocks ship swap")); }
    if (ProbeFrame == 530)
    {
        for(const TCHAR* Name:{TEXT("pause-menu.png"),TEXT("aurora-hud.png"),TEXT("aurora-front.png"),TEXT("daedalus-restored.png")})
            Check(IFileManager::Get().FileSize(*(ProbeDirectory/Name))>100,TEXT("render saved"));
        FFileHelper::SaveStringToFile(bProbePassed?TEXT("{\"passed\":true,\"width\":3840,\"height\":2160,\"ships\":2}"):TEXT("{\"passed\":false}"),*(ProbeDirectory/TEXT("result.json")));
        FPlatformMisc::RequestExitWithStatus(false,bProbePassed?0:1);
    }
}

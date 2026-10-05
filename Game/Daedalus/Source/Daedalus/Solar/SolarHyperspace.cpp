#include "Solar/SolarFlightGameMode.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"

bool ASolarFlightGameMode::BeginHyperspacePreview()
{
    FString Text;
    if(!FFileHelper::LoadFileToString(Text,*(FPaths::ProjectContentDir()/TEXT("Data/Hyperspace/entry.json")))
       ||!HyperTimeline.Load(Text)||ShipId!=TEXT("daedalus"))return false;
    auto* Window=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Hyperspace/Materials/M_HyperWindow.M_HyperWindow"));
    auto* Tunnel=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Hyperspace/Materials/M_HyperTunnel.M_HyperTunnel"));
    auto* PC=GetWorld()->GetFirstPlayerController();
    auto* Pawn=PC?Cast<ASolarFlightPawn>(PC->GetPawn()):nullptr;
    if(!Window||!Tunnel||!Pawn)return false;
    HyperWindow=MakeMesh(PlaneAsset,Window);HyperTunnel=MakeMesh(PlaneAsset,Tunnel);
    HyperWindowMaterial=UMaterialInstanceDynamic::Create(Window,this);
    HyperTunnelMaterial=UMaterialInstanceDynamic::Create(Tunnel,this);
    HyperWindow->SetMaterial(0,HyperWindowMaterial);HyperTunnel->SetMaterial(0,HyperTunnelMaterial);
    HyperWindow->SetWorldLocation(FVector(HyperTimeline.WindowX*100,0,0));
    HyperWindow->SetWorldRotation(FRotationMatrix::MakeFromZ(FVector(-1,0,0)).ToQuat());
    HyperWindow->SetWorldScale3D(FVector(HyperTimeline.WindowRadius*6));
    HyperWindow->SetTranslucentSortPriority(5);
    HyperLight=NewObject<UPointLightComponent>(this);AddInstanceComponent(HyperLight);
    HyperLight->SetMobility(EComponentMobility::Movable);
    HyperLight->SetIntensityUnits(ELightUnits::Candelas);
    HyperLight->SetAttenuationRadius(400000);HyperLight->SetLightColor(FLinearColor(.34f,1,.74f));
    HyperLight->SetCastShadows(true);HyperLight->RegisterComponent();
    HyperLight->SetWorldLocation(FVector(HyperTimeline.WindowX*100-1000,0,0));
    // Existing meshes are only hidden for this separate process; canonical catalogs stay intact.
    for(const auto& M:BodyMeshes)if(M)M->SetVisibility(false);
    for(const auto& M:AirMeshes)if(M)M->SetVisibility(false);
    for(const auto& M:CloudMeshes)if(M)M->SetVisibility(false);
    for(const auto& M:RingMeshes)if(M)M->SetVisibility(false);
    if(Belt)Belt->SetVisibility(false);if(Sky)Sky->SetVisibility(false);if(Dust)Dust->SetVisibility(false);
    if(UnknownMarker)UnknownMarker->SetVisibility(false);
    SolarLight->SetActorRotation(FRotator(-38,-55,0));
    Pawn->DisableInput(PC);EnableInput(PC);
    InputComponent->BindKey(EKeys::R,IE_Pressed,this,&ASolarFlightGameMode::RestartHyperspacePreview);
    InputComponent->BindKey(EKeys::SpaceBar,IE_Pressed,this,&ASolarFlightGameMode::PauseHyperspacePreview);
    InputComponent->BindKey(EKeys::Two,IE_Pressed,this,&ASolarFlightGameMode::InspectHyperspaceTransit);
    InputComponent->BindKey(EKeys::One,IE_Pressed,this,&ASolarFlightGameMode::RestartHyperspacePreview);
    InputComponent->BindKey(EKeys::Escape,IE_Pressed,this,&ASolarFlightGameMode::ExitHyperspacePreview);
    // Optical glow belongs to the luminous effect; ship/scene stay native, without motion blur.
    PC->ConsoleCommand(TEXT("r.BloomQuality 5"));
    Pawn->Camera->PostProcessSettings.BloomIntensity=.24f;
    Pawn->Camera->PostProcessSettings.bOverride_BloomThreshold=true;
    Pawn->Camera->PostProcessSettings.BloomThreshold=1.6f;
    FParse::Value(FCommandLine::Get(),TEXT("HyperCapture="),HyperCaptureDirectory);
    bHyperStills=FParse::Param(FCommandLine::Get(),TEXT("HyperStills"));
    if(!HyperCaptureDirectory.IsEmpty())
    {
        IFileManager::Get().MakeDirectory(*HyperCaptureDirectory,true);
        GEngine->bUseFixedFrameRate=true;GEngine->FixedFrameRate=30;
    }
    PresentHyperspace(0);
    UE_LOG(LogTemp,Display,TEXT("HYPER_PREVIEW_READY model=daedalus native-render sequence=%.2fs"),HyperTimeline.End);
    return true;
}

void ASolarFlightGameMode::RestartHyperspacePreview(){if(HyperCaptureDirectory.IsEmpty()){HyperSeconds=0;bHyperPaused=false;}}
void ASolarFlightGameMode::PauseHyperspacePreview(){if(HyperCaptureDirectory.IsEmpty())bHyperPaused=!bHyperPaused;}
void ASolarFlightGameMode::InspectHyperspaceTransit(){if(HyperCaptureDirectory.IsEmpty()){HyperSeconds=HyperTimeline.Transit+.4;bHyperPaused=false;}}
void ASolarFlightGameMode::ExitHyperspacePreview(){FPlatformMisc::RequestExit(false);}

void ASolarFlightGameMode::PresentHyperspace(double Seconds)
{
    const auto F=HyperTimeline.Sample(Seconds);
    const bool Inside=Seconds>=HyperTimeline.Transit;
    const bool ShowShip=Inside||F.bExteriorShip;
    const FVector Location=Inside?FVector::ZeroVector:FVector(F.ShipX*100,0,0);
    auto Pose=[&](UStaticMeshComponent* M){if(M){M->SetVisibility(ShowShip);M->SetWorldLocationAndRotation(Location,FQuat::Identity);}};
    Pose(Ship);Pose(HullLights);
    for(const auto& M:EngineGlows)Pose(M);
    for(const auto& M:ShipDetails)Pose(M);
    for(const auto& M:HangarDoors)Pose(M);
    for(const auto& M:EngineDynamics)M->SetScalarParameterValue(TEXT("EngineLevel"),Inside?.8:F.Engine*1.5);
    for(const auto& M:BeaconDynamics)M->SetScalarParameterValue(TEXT("BeaconLevel"),FMath::Fmod(Seconds,BeaconPeriod)<BeaconOnSeconds?BeaconEmissionOn:0);
    for(int32 I=0;I<ShipPointLights.Num();++I)
    {
        ShipPointLights[I]->SetWorldLocation(Location+ShipLightPositions[I]);
        ShipPointLights[I]->SetVisibility(ShowShip);
        ShipPointLights[I]->SetIntensity(ShipLightCandela[I]*(ShipLightIsEngine[I]?F.Engine:1));
    }
    auto* Pawn=Cast<ASolarFlightPawn>(GetWorld()->GetFirstPlayerController()->GetPawn());
    FVector Camera(-225000,-195000,100000),Target(43000,0,1500);
    if(Inside)
    {
        const double U=FMath::Clamp((Seconds-HyperTimeline.Transit)/(HyperTimeline.End-HyperTimeline.Transit),0.,1.);
        Camera=FMath::Lerp(FVector(155000,-120000,37000),FVector(112000,-87000,29000),U);
        Target=FVector(-12000,0,0);
    }
    const FRotator Rotation=(Target-Camera).Rotation();
    Pawn->Camera->SetFieldOfView(48);Pawn->Camera->SetWorldLocationAndRotation(Camera,Rotation);
    Stars->SetWorldLocation(Camera);Stars->SetVisibility(!Inside);
    HyperWindow->SetVisibility(F.Radius>.001&&!Inside);
    HyperWindowMaterial->SetScalarParameterValue(TEXT("PhaseTime"),Seconds);
    HyperWindowMaterial->SetScalarParameterValue(TEXT("Radius"),F.Radius);
    HyperWindowMaterial->SetScalarParameterValue(TEXT("Strength"),F.Strength);
    HyperLight->SetIntensity(Inside?0:2000000*F.Strength*F.Radius);
    HyperTunnel->SetVisibility(Inside);
    HyperTunnelMaterial->SetScalarParameterValue(TEXT("PhaseTime"),Seconds-HyperTimeline.Transit);
    const FVector Forward=Rotation.Vector();
    HyperTunnel->SetWorldLocation(Camera+Forward*1000000);
    // Plane local X follows camera right, local Y follows camera up.
    const FQuat PlaneRotation=FRotationMatrix::MakeFromXY(FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y),FRotationMatrix(Rotation).GetUnitAxis(EAxis::Z)).ToQuat();
    HyperTunnel->SetWorldRotation(PlaneRotation);
    HyperTunnel->SetWorldScale3D(FVector(2*10000*FMath::Tan(FMath::DegreesToRadians(24.)),2*10000*FMath::Tan(FMath::DegreesToRadians(24.))*9/16,1));
    // Cool illumination in transit is an authored look, not simulated stellar light.
    SolarLight->GetLightComponent()->SetLightColor(Inside?FLinearColor(.58f,.78f,1):FLinearColor(1,.92f,.8f));
}

void ASolarFlightGameMode::TickHyperspacePreview(float DeltaSeconds)
{
    ++HyperFrame;
    if(HyperCaptureDirectory.IsEmpty())
    {
        if(!bHyperPaused)HyperSeconds=FMath::Fmod(HyperSeconds+DeltaSeconds,HyperTimeline.End+.5);
        PresentHyperspace(HyperSeconds);return;
    }
    constexpr int32 Warmup=60;
    const double Moments[]={0,.8,HyperTimeline.Full,HyperTimeline.Nose-.05,(HyperTimeline.Nose+HyperTimeline.Tail)*.5,
        HyperTimeline.Collapse,HyperTimeline.Closed-.4,HyperTimeline.Closed,HyperTimeline.Transit+1,HyperTimeline.Transit+4};
    const int32 Count=bHyperStills?UE_ARRAY_COUNT(Moments):FMath::CeilToInt(HyperTimeline.End*30);
    const int32 Local=HyperFrame-Warmup;
    const int32 Index=bHyperStills?Local/10:Local;
    if(Local>=0&&Index<Count)
    {
        HyperSeconds=bHyperStills?Moments[Index]:Index/30.;
        PresentHyperspace(HyperSeconds);
        if(!bHyperStills||Local%10==5)
            FScreenshotRequest::RequestScreenshot(HyperCaptureDirectory/FString::Printf(TEXT("%04d.png"),Index),false,false);
    }
    else if(Local>=(bHyperStills?Count*10:Count)+5)
    {
        const auto Size=GEngine->GameViewport->Viewport->GetSizeXY();
        const auto& State=Flight.GetState();
        bool Passed=Size==FIntPoint(3840,2160)&&State.PositionMetres==InitialState.PositionMetres
            &&State.VelocityMetresPerSecond.IsZero()&&State.SimulationSeconds==InitialState.SimulationSeconds;
        for(int32 I=0;I<Count;++I)
            Passed&=IFileManager::Get().FileSize(*(HyperCaptureDirectory/FString::Printf(TEXT("%04d.png"),I)))>100;
        FFileHelper::SaveStringToFile(FString::Printf(TEXT("{\"passed\":%s,\"width\":%d,\"height\":%d,\"frames\":%d,\"flightStateUnchanged\":%s}"),
            Passed?TEXT("true"):TEXT("false"),Size.X,Size.Y,Count,Passed?TEXT("true"):TEXT("false")),*(HyperCaptureDirectory/TEXT("result.json")));
        UE_LOG(LogTemp,Display,TEXT("HYPER_CAPTURE_%s frames=%d"),Passed?TEXT("PASS"):TEXT("FAIL"),Count);
        FPlatformMisc::RequestExitWithStatus(false,Passed?0:1);
    }
}

#include "Solar/SolarFlightGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

bool ASolarFlightGameMode::LoadSystem()
{
    FString Text; TSharedPtr<FJsonObject> Json;
    if (!FFileHelper::LoadFileToString(Text, *(FPaths::ProjectContentDir()/TEXT("Data/Solar/system.json"))) ||
        !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text),Json) || !Json) return false;
    double Version=0; const TArray<TSharedPtr<FJsonValue>>* Rows=nullptr;
    if (!Json->TryGetNumberField(TEXT("version"),Version) || Version!=1 || !Json->TryGetArrayField(TEXT("bodies"),Rows) || Rows->IsEmpty() || Rows->Num()>10000) return false;
    TSet<FString> Ids;
    auto Vec=[](const TSharedPtr<FJsonObject>& O,const TCHAR* Key,FVector3d& V){
        const TArray<TSharedPtr<FJsonValue>>* A=nullptr;
        if (!O->TryGetArrayField(Key,A) || A->Num()!=3) return false;
        for(int32 I=0;I<3;++I) if(!(*A)[I]->TryGetNumber(V[I]) || !FMath::IsFinite(V[I])) return false;
        return true;
    };
    for(const auto& Row:*Rows)
    {
        const auto O=Row->AsObject(); FSolarBodyDefinition D;
        if(!O || !O->TryGetStringField(TEXT("id"),D.Id) || !D.Id.StartsWith(TEXT("sol.")) || Ids.Contains(D.Id)
            || !O->TryGetStringField(TEXT("name"),D.Name) || !O->TryGetStringField(TEXT("parentId"),D.ParentId)
            || !O->TryGetStringField(TEXT("kind"),D.Kind) || !O->TryGetNumberField(TEXT("radiusMetres"),D.Radius)
            || !FMath::IsFinite(D.Radius) || D.Radius<=0 || !Vec(O,TEXT("positionMetres"),D.Position)
            || !O->TryGetNumberField(TEXT("rotationHours"),D.RotationHours) || !FMath::IsFinite(D.RotationHours)) return false;
        O->TryGetNumberField(TEXT("tiltDegrees"),D.Tilt);
        if(O->HasField(TEXT("shapeScale")) && !Vec(O,TEXT("shapeScale"),D.Shape)) return false;
        if(D.Shape.GetMin()<=0 || !FMath::IsFinite(D.Tilt)) return false;
        O->TryGetNumberField(TEXT("ringInnerMetres"),D.RingInner); O->TryGetNumberField(TEXT("ringOuterMetres"),D.RingOuter);
        if(!FMath::IsFinite(D.RingInner) || !FMath::IsFinite(D.RingOuter) || D.RingInner<0 || D.RingOuter<D.RingInner) return false;
        D.bAtmosphere=O->HasField(TEXT("atmosphereColor"));
        Ids.Add(D.Id); BodyDefinitions.Add(D);
        // Conservative enclosing sphere protects visibly flattened/irregular surfaces.
        Bodies.Add({D.Id,D.Position,D.Radius*D.Shape.GetMax()});
        if(D.Id==TEXT("sol.sun")){SunPosition=D.Position;SunRadius=D.Radius;}
        if(D.Id==TEXT("sol.earth")){EarthRadius=D.Radius;SelectedBody=BodyDefinitions.Num()-1;}
    }
    for(const auto& D:BodyDefinitions) if(!D.ParentId.IsEmpty() && !Ids.Contains(D.ParentId)) return false;
    if(!Ids.Contains(TEXT("sol.earth")) || !Ids.Contains(TEXT("sol.sun"))) return false;
    const auto BeltJson=Json->GetObjectField(TEXT("asteroidBelt"));
    double Inner=0,Outer=0,Count=0,Seed=0;
    if(!BeltJson || !BeltJson->TryGetNumberField(TEXT("innerMetres"),Inner) || !BeltJson->TryGetNumberField(TEXT("outerMetres"),Outer)
        || !BeltJson->TryGetNumberField(TEXT("count"),Count) || !BeltJson->TryGetNumberField(TEXT("seed"),Seed)
        || !FMath::IsFinite(Inner) || !FMath::IsFinite(Outer) || Inner<=0 || Outer<=Inner || Count!=600 || !FMath::IsFinite(Seed)) return false;
    FRandomStream Random{int32(Seed)};
    for(int32 I=0;I<int32(Count);++I){double A=Random.FRand()*2*PI,R=Inner+(Outer-Inner)*Random.FRand();
        BeltPositions.Add(SunPosition+FVector3d(FMath::Cos(A)*R,FMath::Sin(A)*R,Random.FRandRange(-1e8,1e8)));
        BeltRadii.Add(Random.FRandRange(200,2000));}
    return true;
}
bool ASolarFlightGameMode::CreateSystem()
{
    auto* Rock=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/Solar/SM_SolarRock.SM_SolarRock"));
    auto* SkyMat=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Solar/Materials/M_Sky.M_Sky"));
    auto* RockMat=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Solar/Materials/M_Rock.M_Rock"));
    if(!Rock || !SkyMat || !RockMat) return false;
    Sky=MakeMesh(SphereAsset,SkyMat); Sky->SetWorldScale3D(FVector(2.5e7));
    for(int32 I=0;I<BodyDefinitions.Num();++I)
    {
        const auto& D=BodyDefinitions[I]; const FString Name=D.Id.Mid(4);
        const FString Path=FString::Printf(TEXT("/Game/Solar/Materials/M_Body_%s.M_Body_%s"),*Name,*Name);
        auto* Material=LoadObject<UMaterialInterface>(nullptr,*Path); if(!Material) return false;
        auto* Dynamic=UMaterialInstanceDynamic::Create(Material,this);
        BodyDynamics.Add(Dynamic); BodyMeshes.Add(MakeMesh(D.Kind==TEXT("asteroid")?Rock:SphereAsset.Get(),Dynamic));
        UStaticMeshComponent* Air=nullptr; UMaterialInstanceDynamic* AirDyn=nullptr;
        if(D.bAtmosphere){const FString AP=FString::Printf(TEXT("/Game/Solar/Materials/M_Air_%s.M_Air_%s"),*Name,*Name);
            auto* AM=LoadObject<UMaterialInterface>(nullptr,*AP);if(!AM)return false;
            AirDyn=UMaterialInstanceDynamic::Create(AM,this);Air=MakeMesh(SphereAsset,AirDyn);}
        AirMeshes.Add(Air); AirDynamics.Add(AirDyn);
        if(D.RingOuter>0){const FString RP=FString::Printf(TEXT("/Game/Solar/Materials/M_Ring_%s.M_Ring_%s"),*Name,*Name);
            auto* RM=LoadObject<UMaterialInterface>(nullptr,*RP);if(!RM)return false;
            RingBodies.Add(I);RingMeshes.Add(MakeMesh(PlaneAsset,RM));}
        if(D.Id==TEXT("sol.earth")){Earth=BodyMeshes[I];EarthDynamic=Dynamic;Atmosphere=Air;AtmosphereDynamic=AirDyn;}
        if(D.Id==TEXT("sol.sun"))Sun=BodyMeshes[I];
    }
    Belt=NewObject<UInstancedStaticMeshComponent>(this);Belt->RegisterComponent();Belt->SetStaticMesh(Rock);Belt->SetMaterial(0,RockMat);
    Belt->SetCollisionEnabled(ECollisionEnabled::NoCollision);Belt->SetCastShadow(false);
    for(const auto& P:BeltPositions) Belt->AddInstance(FTransform::Identity);
    return true;
}
void ASolarFlightGameMode::UpdateSystem(const FVector& CameraPosition)
{
    const auto& S=Flight.GetState();
    auto Project=[&](const FVector3d& P){const auto Rel=P-S.PositionMetres-FVector3d(CameraPosition)/100;
        const double Factor=FMath::Min(1.,3e6/FMath::Max(Rel.Size(),1.));
        return TPair<FVector,double>(CameraPosition+FVector(Rel*100*Factor),Factor);};
    for(int32 I=0;I<BodyDefinitions.Num();++I)
    {
        const auto& D=BodyDefinitions[I];const auto P=Project(D.Position);
        const double Angle=D.RotationHours==0?0:FMath::Fmod(S.SimulationSeconds*360/(D.RotationHours*3600),360.);
        const FQuat Rotation=FQuat(FVector::ForwardVector,FMath::DegreesToRadians(D.Tilt))*FQuat(FVector::UpVector,FMath::DegreesToRadians(Angle));
        BodyMeshes[I]->SetWorldLocationAndRotation(P.Key,Rotation);
        BodyMeshes[I]->SetWorldScale3D(FVector(D.Shape*D.Radius*P.Value));
        const auto Dir=(SunPosition-D.Position).GetSafeNormal();
        const FLinearColor LightDir(Dir.X,Dir.Y,Dir.Z,0);
        BodyDynamics[I]->SetVectorParameterValue(TEXT("SunDirection"),LightDir);
        if(AirMeshes[I]){AirMeshes[I]->SetWorldLocationAndRotation(P.Key,Rotation);AirMeshes[I]->SetWorldScale3D(FVector(D.Shape*D.Radius*P.Value*1.012));
            AirDynamics[I]->SetVectorParameterValue(TEXT("SunDirection"),LightDir);}
    }
    for(int32 I=0;I<RingMeshes.Num();++I){const int32 B=RingBodies[I];const auto& D=BodyDefinitions[B];const auto P=Project(D.Position);
        RingMeshes[I]->SetWorldLocationAndRotation(P.Key,BodyMeshes[B]->GetComponentQuat());
        RingMeshes[I]->SetWorldScale3D(FVector(D.RingOuter*P.Value*2,D.RingOuter*P.Value*2,1));}
    for(int32 I=0;I<BeltPositions.Num();++I){const auto P=Project(BeltPositions[I]);
        Belt->UpdateInstanceTransform(I,FTransform(FQuat(FVector::UpVector,I*.73),P.Key,FVector(BeltRadii[I]*P.Value)),false,I==BeltPositions.Num()-1);}
    Sky->SetWorldLocation(CameraPosition);
    if(SolarLight)SolarLight->SetActorRotation((-FVector(SunPosition-S.PositionMetres).GetSafeNormal()).Rotation());
}
void ASolarFlightGameMode::SelectBody(int32 Step)
{
    if(!bReady || BodyDefinitions.IsEmpty())return;
    SelectedBody=(SelectedBody+Step+BodyDefinitions.Num())%BodyDefinitions.Num();Message.Empty();
}
void ASolarFlightGameMode::InspectSelectedBody()
{
    if(!bReady || !BodyDefinitions.IsValidIndex(SelectedBody))return;
    const auto& D=BodyDefinitions[SelectedBody];
    FVector3d Direction=(SunPosition-D.Position).GetSafeNormal();
    if(D.Id==TEXT("sol.sun"))Direction=FVector3d(-1,1,.3).GetSafeNormal();
    // Oblique inspection exposes surface, pole, and rings together.
    Direction=(Direction+FVector3d(0,0,.6)).GetSafeNormal();
    Daedalus::FFlightState Start;Start.PositionMetres=D.Position+Direction*(D.Radius*D.Shape.GetMax()*2.6+5000);
    const auto Look=(-Direction).Rotation();Start.YawDegrees=Look.Yaw;Start.PitchDegrees=FMath::Clamp(double(Look.Pitch),-Profiles[ProfileIndex].Config.PitchLimitDegrees,Profiles[ProfileIndex].Config.PitchLimitDegrees);
    FString Error;if(Flight.Initialize(Profiles[ProfileIndex].Config,Start,Bodies,Error)){
        bPaused=false;Message=TEXT("Testovací přesun k tělesu — nejde o cestování pohonem.");
        if(auto* P=Cast<ASolarFlightPawn>(GetWorld()->GetFirstPlayerController()->GetPawn()))P->ResetCamera();
    }else Message=Error;
}


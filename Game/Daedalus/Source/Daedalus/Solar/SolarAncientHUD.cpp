#include "Solar/SolarHUDCanvas.h"

namespace SolarHUDUI
{
namespace
{
const FLinearColor Ice = FLinearColor::FromSRGBColor(FColor(149,185,226));
const FLinearColor Jade = FLinearColor::FromSRGBColor(FColor(106,212,190));
const FLinearColor Sand = FLinearColor::FromSRGBColor(FColor(229,196,145));
const FLinearColor Ember = FLinearColor::FromSRGBColor(FColor(209,128,83));
const FLinearColor Rust = FLinearColor::FromSRGBColor(FColor(139,65,57));
const FLinearColor TextColor = FLinearColor::FromSRGBColor(FColor(229,230,215));
const FLinearColor Quiet = FLinearColor::FromSRGBColor(FColor(145,159,169));
const FLinearColor Glass(.004f,.011f,.022f,.93f), Cell(.0015f,.004f,.010f,.92f);

FString HUDShipName(const ASolarFlightGameMode& Lab)
{
    return Lab.ShipId==TEXT("aurora")?TEXT("Aurora Class"):Lab.ShipName;
}

// Original geometric ornaments inspired by the block-script vocabulary.
// They are not a downloaded font, a translated message or invented telemetry.
void Glyphs(const FInstruments& D, float X, float Y, int32 Count, int32 Seed, FLinearColor Color)
{
    const uint16 Forms[] = {0x1b5,0x16d,0x0f3,0x1a6,0x15b,0x12e,0x0bd,0x1c9,0x137,0x0da};
    const FVector2D Segments[][2] = {
        {{0,0},{6,0}},{{0,0},{0,8}},{{6,0},{6,8}},{{0,8},{6,8}},{{0,4},{6,4}},
        {{3,0},{3,4}},{{3,4},{3,8}},{{0,0},{3,4}},{{3,4},{6,8}}
    };
    for (int32 I=0;I<Count;++I)
    {
        const uint16 Form=Forms[(I+Seed)%UE_ARRAY_COUNT(Forms)];
        for(int32 S=0;S<UE_ARRAY_COUNT(Segments);++S)
            if(Form&(1<<S))D.Line(FVector2D(X+I*10,Y)+Segments[S][0],FVector2D(X+I*10,Y)+Segments[S][1],Color,.65f);
    }
}

TArray<FVector2D> Chamfer(float X,float Y,float W,float H,float Cut)
{
    return {{X+Cut,Y},{X+W-Cut,Y},{X+W,Y+Cut},{X+W,Y+H-Cut},
        {X+W-Cut,Y+H},{X+Cut,Y+H},{X,Y+H-Cut},{X,Y+Cut}};
}

void Crystal(const FInstruments& D,FVector2D C,float R,FLinearColor Color)
{
    D.Polygon({C+FVector2D(0,-R),C+FVector2D(R*.55f,-R*.35f),C+FVector2D(R*.55f,R*.35f),
        C+FVector2D(0,R),C+FVector2D(-R*.55f,R*.35f),C+FVector2D(-R*.55f,-R*.35f)},Alpha(Color,.12f),Color);
    D.Line(C+FVector2D(0,-R),C+FVector2D(0,R),Alpha(Color,.5f));
    D.Line(C+FVector2D(-R*.55f,-R*.35f),C+FVector2D(R*.55f,R*.35f),Alpha(Color,.35f));
}

void DrawAncientDock(const FInstruments& D,const ASolarFlightGameMode& Lab)
{
    constexpr float W=938,H=214;
    const float Y=D.Height-H-20;
    float X=(D.Width-W)*.5f;
    // Crystal-channel module. Dormant gameplay channels have no fake power values.
    AncientPanel(D,X,Y,124,H,TEXT("ENERGIE"));
    const TCHAR* Channels[]={TEXT("DRN"),TEXT("ŠTÍ"),TEXT("MOT"),TEXT("SYS")};
    const FLinearColor Colors[]={Ember,Ice,Sand,Jade};
    for(int32 I=0;I<4;++I)
    {
        const float A=X+15+I*25;
        D.Text(Channels[I],A,Y+46,Colors[I],8,23);
        Crystal(D,{A+8,Y+73},10,Alpha(Colors[I],.5f));
        for(int32 J=0;J<13;++J)D.Rect(A,Y+92+J*5.4f,17,3.4f,Alpha(Colors[I],.16f));
        D.Text(TEXT("—"),A+8,Y+169,Quiet,11,20,true);
    }
    D.Text(TEXT("NEZAPOJENO"),X+62,Y+195,Quiet,8,100,true);
    X+=136;

    // Orion's circular bands, angular Ancient frame and a real hull projection.
    AncientPanel(D,X,Y,214,H,HUDShipName(Lab).ToUpper()+TEXT(" · TRUP"));
    const FVector2D C(X+107,Y+117);
    TArray<FVector2D> Octagon;
    for(int32 I=0;I<8;++I)
    {
        const double A=FMath::DegreesToRadians(22.5+45*I);
        Octagon.Add(C+FVector2D(FMath::Cos(A),FMath::Sin(A))*77);
    }
    D.Polygon(Octagon,Cell,Alpha(Sand,.75f));
    D.Arc(C,68,0,360,Alpha(Ember,.65f),1.3f);
    D.Arc(C,65,0,360,Alpha(Sand,.6f),.65f);
    for(int32 I=0;I<4;++I)
    {
        D.Arc(C,61,-131+I*90,-49+I*90,Ice,2.2f);
        D.Arc(C,57,-128+I*90,-52+I*90,Alpha(Ice,.35f),.65f);
    }
    for(int32 I=0;I<24;++I)
    {
        const double A=FMath::DegreesToRadians(I*15.);
        const FVector2D V(FMath::Cos(A),FMath::Sin(A));
        D.Line(C+V*69,C+V*(I%3==0?75:72),Alpha(Sand,.6f),.65f);
    }
    D.Line(C-FVector2D(0,49),C+FVector2D(0,49),Alpha(Jade,.17f));
    D.Hull(C,101,Jade,INDEX_NONE,Lab.ActiveShip==1);
    D.Text(TEXT("4 SEKTORY ŠTÍTU · —"),C.X,Y+195,Quiet,8,192,true);
    X+=226;

    AncientPanel(D,X,Y,206,H,TEXT("POHON"));
    const auto& State=Lab.Flight.GetState();
    const double Speed=State.VelocityMetresPerSecond.Size();
    const float Power=FMath::Clamp(float(FMath::Abs(State.Throttle)),0.f,1.f);
    D.Rect(X+51,Y+43,144,51,Cell);
    D.Text(FString::Printf(TEXT("%.0f"),Speed>=1000?Speed/1000:Speed),X+61,Y+46,TextColor,25,122);
    D.Text(Speed>=1000?TEXT("km/s · SKUTEČNÁ RYCHLOST"):TEXT("m/s · SKUTEČNÁ RYCHLOST"),X+60,Y+83,Quiet,7,126);
    D.Text(FString::Printf(TEXT("TAH %.0f %%"),State.Throttle*100),X+59,Y+109,Sand,12,136);
    D.Line({X+11,Y+46},{X+43,Y+46},Sand,.7f);
    for(int32 I=0;I<24;++I)
    {
        const float A=Y+52+I*5.5f;
        const bool Lit=(24-I)/24.f<=Power;
        D.Polygon({{X+13,A},{X+29,A},{X+34,A+3.6f},{X+18,A+3.6f}},Lit?Sand:Alpha(Ember,.18f),FLinearColor::Transparent);
    }
    D.Text(TEXT("100"),X+34,Y+47,Quiet,7,18);
    D.Text(TEXT("0"),X+35,Y+177,Quiet,7,15);
    const TCHAR* Modes[]={TEXT("MANÉVROVÁNÍ"),TEXT("IMPULS"),TEXT("PLNÝ IMPULS")};
    for(int32 I=0;I<3;++I)
    {
        const float A=Y+133+I*18;
        const bool Active=I==Lab.ProfileIndex;
        D.Polygon({{X+52,A},{X+188,A},{X+196,A+7},{X+188,A+15},{X+52,A+15}},
            Active?Alpha(Rust,.35f):Cell,Active?Alpha(Sand,.65f):Alpha(Ice,.15f));
        D.Text(Modes[I],X+61,A+3,Active?Sand:Quiet,8,126);
    }
    D.Text(State.Throttle<0?TEXT("ZPĚTNÝ CHOD"):TEXT("ASISTOVANÝ LET"),X+112,Y+195,Quiet,8,166,true);
    X+=218;

    AncientPanel(D,X,Y,358,H,TEXT("OBRANNÉ SYSTÉMY"));
    const TCHAR* Names[]={TEXT("DRONY"),TEXT("PULZY"),TEXT("REZERVA")};
    const FLinearColor Accents[]={Sand,Jade,Ice};
    for(int32 I=0;I<3;++I)
    {
        const float A=X+11+I*114;
        D.Polygon(Chamfer(A,Y+42,108,143,9),Cell,Alpha(Accents[I],.3f));
        D.Polygon({{A+3,Y+47},{A+105,Y+47},{A+105,Y+66},{A+14,Y+66},{A+3,Y+57}},Alpha(I==0?Rust:Ice,.14f),FLinearColor::Transparent);
        D.Text(Names[I],A+12,Y+50,Accents[I],10,88);
        const FVector2D WeaponCentre(A+54,Y+108);
        D.Arc(WeaponCentre,29,0,360,Alpha(Accents[I],.35f));
        if(I==0)
        {
            D.Arc(WeaponCentre,22,0,360,Sand,1.3f);
            for(int32 J=0;J<6;++J)
            {
                const double R=FMath::DegreesToRadians(J*60.);
                const FVector2D U(FMath::Cos(R),FMath::Sin(R));
                D.Line(WeaponCentre+U*12,WeaponCentre+U*21,Ember,1.3f);
            }
            Crystal(D,WeaponCentre,13,Sand);
        }
        else if(I==1)
        {
            for(int32 J=0;J<3;++J)
                D.Polygon({WeaponCentre+FVector2D(-20+J*6,-10+J*8),WeaponCentre+FVector2D(9+J*6,-10+J*8),
                    WeaponCentre+FVector2D(19+J*6,-5+J*8),WeaponCentre+FVector2D(9+J*6,J*8),WeaponCentre+FVector2D(-20+J*6,J*8)},
                    Alpha(Jade,.08f),Alpha(Jade,.55f));
        }
        else
        {
            D.Line(WeaponCentre+FVector2D(-13,-13),WeaponCentre+FVector2D(13,13),Alpha(Ice,.4f));
            D.Line(WeaponCentre+FVector2D(13,-13),WeaponCentre+FVector2D(-13,13),Alpha(Ice,.4f));
        }
        Glyphs(D,A+23,Y+151,6,2+I,Alpha(Accents[I],.4f));
        D.Text(TEXT("—"),A+54,Y+168,Quiet,10,40,true);
    }
    D.Text(TEXT("VÝZBROJ / ŠTÍTY — NEZAPOJENO"),X+179,Y+195,Quiet,8,332,true);
}

void DrawAncientComputer(const FInstruments& D,const ASolarFlightGameMode& Lab)
{
    const float X=D.Width-308,Y=D.Height*.035f;
    AncientPanel(D,X,Y,256,252,TEXT("ANTICKÝ POČÍTAČ"));
    Glyphs(D,X+19,Y+44,21,3,Alpha(Ice,.55f));
    D.Rect(X+14,Y+64,228,37,Cell);
    D.Text(HUDShipName(Lab).ToUpper(),X+23,Y+69,TextColor,12,208);
    D.Text(TEXT("LANTSKÉ ROZHRANÍ · PŘEKLAD CZ"),X+23,Y+88,Quiet,7,208);
    const TCHAR* Items[]={TEXT("Navigace"),TEXT("Skenování"),TEXT("Lodní systémy"),TEXT("Hyperpohon")};
    for(int32 I=0;I<4;++I)
    {
        const float A=X+14+(I%2)*117,B=Y+112+(I/2)*32;
        D.Polygon(Chamfer(A,B,111,27,5),Alpha(Ice,.06f),Alpha(Ice,.3f));
        D.Rect(A+3,B+6,2,15,I==0?Jade:Alpha(Ember,.5f));
        D.Text(Items[I],A+10,B+7,Quiet,9,94);
    }
    const auto Nav=Lab.Navigation.Query(Lab.Flight.GetState().VelocityMetresPerSecond.Size(),Lab.Flight.GetConfig().MaxSpeed,Lab.Galaxy.DesiredSeconds);
    FString Target=TEXT("NAVIGACE BEZ CÍLE");
    if(Nav.bValid)
    {
        Target=Lab.Navigation.GetTargetBodyId();
        for(const auto& System:Lab.MapSystems)if(System.Id==Lab.Navigation.GetTargetSystemId())
            for(const auto& Body:System.Bodies)if(Body.Id==Target){Target=System.Name+TEXT(" / ")+Body.Name;break;}
    }
    const FString Range=Nav.DistanceMetres>=Daedalus::FNavigationPlan::LightYearMetres*.01
        ?FString::Printf(TEXT("%.2f světelných let"),Nav.DistanceMetres/Daedalus::FNavigationPlan::LightYearMetres):DistanceText(Nav.DistanceMetres);
    D.Polygon(Chamfer(X+14,Y+182,228,55,7),Cell,Alpha(Sand,.4f));
    D.Text(Target,X+24,Y+190,Jade,10,208);
    D.Text(Nav.bValid?Range:TEXT("Další funkce doplníme později"),X+24,Y+213,Quiet,9,208);
}
}

void AncientPanel(const FInstruments& D,float X,float Y,float W,float H,const FString& Title)
{
    D.Polygon(Chamfer(X,Y,W,H,13),Glass,Alpha(Ice,.8f));
    D.Polygon(Chamfer(X+3,Y+3,W-6,H-6,11),FLinearColor::Transparent,Alpha(Ice,.38f));
    D.Polygon({{X+13,Y+7},{X+W-24,Y+7},{X+W-11,Y+20},{X+W-11,Y+32},
        {X+24,Y+32},{X+11,Y+19}},Alpha(Rust,.44f),Alpha(Ember,.75f));
    D.Text(Title,X+22,Y+13,Sand,11,W-54);
    D.Line({X+11,Y+36},{X+W-11,Y+36},Alpha(Sand,.38f));
    D.Line({X+7,Y+47},{X+7,Y+H-28},Alpha(Ember,.5f),1.5f);
    D.Line({X+W-7,Y+47},{X+W-7,Y+H-28},Alpha(Ice,.5f),1.5f);
    D.Polygon({{X+17,Y+H-9},{X+W*.42f,Y+H-9},{X+W*.42f+8,Y+H-4},{X+22,Y+H-4}},Alpha(Ember,.6f),FLinearColor::Transparent);
    D.Line({X+W*.65f,Y+H-6},{X+W-20,Y+H-6},Alpha(Ice,.6f),1.5f);
}

void DrawAncientHUD(const FInstruments& D,const ASolarFlightGameMode& Lab)
{
    DrawRadar(D,Lab,true);
    const float X=D.Width*.03f,Y=D.Height*.035f;
    Glyphs(D,X,Y+47,16,4,Alpha(Ice,.55f));
    const FVector2D C(X+83,Y+143);
    for(int32 I=0;I<24;++I)
    {
        const double A=FMath::DegreesToRadians(I*15.);
        const FVector2D U(FMath::Cos(A),FMath::Sin(A));
        D.Line(C+U*82,C+U*(I%3==0?87:84),Alpha(Sand,.5f),.65f);
    }
    DrawAncientDock(D,Lab);
    DrawAncientComputer(D,Lab);
}
}

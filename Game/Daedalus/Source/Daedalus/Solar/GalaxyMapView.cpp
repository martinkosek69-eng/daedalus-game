#include "Solar/GalaxyMapView.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "CanvasItem.h"
#include "Materials/MaterialInterface.h"

namespace
{
constexpr double LY = Daedalus::FNavigationPlan::LightYearMetres;
constexpr double AU = 149597870700.0;
const FLinearColor Ink(.003f, .008f, .015f, 1), Plate(.008f, .022f, .035f, .98f);
const FLinearColor Pale(.52f, .67f, .76f), Cyan(.32f, .83f, 1), White(.88f, .94f, 1);

void MapPlate(UCanvas* Canvas, FVector2D Position, FVector2D Size, FLinearColor Color)
{
    FCanvasTileItem Item(Position, Size, Color);
    Item.BlendMode = SE_BLEND_Translucent;
    Canvas->DrawItem(Item);
}
void Line(UCanvas* Canvas, FVector2D A, FVector2D B, FLinearColor Color, float Width = 1)
{
    FCanvasLineItem Item(A, B); Item.SetColor(Color); Item.LineThickness = Width;
    Canvas->DrawItem(Item);
}
void Ring(UCanvas* Canvas, FVector2D Center, float Radius, FLinearColor Color, float Width)
{
    constexpr int32 Steps = 40;
    for (int32 I = 0; I < Steps; ++I)
    {
        const double A = 2 * PI * I / Steps, B = 2 * PI * (I + 1) / Steps;
        Line(Canvas, Center + FVector2D(FMath::Cos(A), FMath::Sin(A)) * Radius,
             Center + FVector2D(FMath::Cos(B), FMath::Sin(B)) * Radius, Color, Width);
    }
}
void SoftDisc(UCanvas* Canvas, FVector2D Center, float Radius, FLinearColor Color)
{
    TArray<FCanvasUVTri> Triangles;
    for (int32 I = 0; I < 12; ++I)
    {
        FCanvasUVTri Triangle;
        const double A = 2 * PI * I / 12, B = 2 * PI * (I + 1) / 12;
        Triangle.V0_Pos = Center;
        Triangle.V1_Pos = Center + FVector2D(FMath::Cos(A), FMath::Sin(A)) * Radius;
        Triangle.V2_Pos = Center + FVector2D(FMath::Cos(B), FMath::Sin(B)) * Radius;
        Triangle.V0_UV = Triangle.V1_UV = Triangle.V2_UV = FVector2D::ZeroVector;
        Triangle.V0_Color = Color;
        Triangle.V1_Color = Triangle.V2_Color = FLinearColor(Color.R, Color.G, Color.B, 0);
        Triangles.Add(Triangle);
    }
    // UE's triangle draw asserts on null textures. Reuse the Engine-owned white
    // texture exposed by the plain tile constructor without a RenderCore import.
    const FCanvasTileItem WhiteTile(FVector2D::ZeroVector, FVector2D::UnitVector, FLinearColor::White);
    FCanvasTriangleItem Item(Triangles, WhiteTile.Texture); Item.BlendMode = SE_BLEND_Translucent;
    Canvas->DrawItem(Item);
}
FString Short(const FString& Text, int32 Limit)
{
    return Text.Len() > Limit ? Text.Left(Limit - 1) + TEXT("…") : Text;
}
FString SearchKey(FString Text)
{
    Text = Text.ToLower();
    const FString Accents = TEXT("áčďéěíňóřšťúůýžäöü"), Plain = TEXT("acdeeinorstuuyzaou");
    for (TCHAR& Character : Text)
    {
        const int32 Index = Accents.Find(FString::Chr(Character));
        if (Index != INDEX_NONE) Character = Plain[Index];
    }
    return Text;
}
FString KindName(const FString& Kind)
{
    if (Kind == TEXT("star")) return TEXT("Hvězda");
    if (Kind == TEXT("planet")) return TEXT("Planeta");
    if (Kind == TEXT("moon")) return TEXT("Měsíc");
    if (Kind == TEXT("dwarf") || Kind == TEXT("dwarf_planet") || Kind == TEXT("dwarf-planet")) return TEXT("Trpasličí planeta");
    if (Kind == TEXT("comet")) return TEXT("Kometa");
    return TEXT("Malé těleso");
}
FString QualityName(const FString& Quality)
{
    if (Quality.Contains(TEXT("spacecraft"))) return TEXT("Povrch podle snímků sond");
    if (Quality.Contains(TEXT("cloud"))) return TEXT("Oblačnost · orientační obrazová mapa");
    if (Quality.Contains(TEXT("schematic"))) return TEXT("Schematický povrch · bez detailních snímků");
    if (Quality.Contains(TEXT("fictional"))) return TEXT("Autorský povrch fiktivního světa");
    if (Quality.Contains(TEXT("New-Horizons"))) return TEXT("Částečná mapa New Horizons · černá = nepozorováno");
    if (Quality.Contains(TEXT("catalog")) || Quality.Contains(TEXT("orbital"))) return TEXT("Katalogové údaje · bez mapy povrchu");
    return Quality.IsEmpty() ? TEXT("Původ povrchu neuveden") : Quality;
}
FString Distance(double Metres)
{
    if (!FMath::IsFinite(Metres) || Metres < 0) return TEXT("—");
    if (Metres >= LY * .01) return FString::Printf(TEXT("%.3f ly"), Metres / LY);
    if (Metres >= AU * .1) return FString::Printf(TEXT("%.3f AU"), Metres / AU);
    if (Metres >= 1000) return FString::Printf(TEXT("%.1f km"), Metres / 1000);
    return FString::Printf(TEXT("%.1f m"), Metres);
}
FString Duration(double Seconds)
{
    if (!FMath::IsFinite(Seconds) || Seconds < 0) return TEXT("—");
    if (Seconds >= 31557600) return FString::Printf(TEXT("%.2f roku"), Seconds / 31557600);
    if (Seconds >= 86400) return FString::Printf(TEXT("%.2f dne"), Seconds / 86400);
    if (Seconds >= 3600) return FString::Printf(TEXT("%.2f h"), Seconds / 3600);
    if (Seconds >= 60) return FString::Printf(TEXT("%.1f min"), Seconds / 60);
    return FString::Printf(TEXT("%.1f s"), Seconds);
}
FString RequiredSpeed(double Speed)
{
    if (Speed > 299792458) return FString::Printf(TEXT("%.2f × c"), Speed / 299792458);
    if (Speed >= 1000) return FString::Printf(TEXT("%.1f km/s"), Speed / 1000);
    return FString::Printf(TEXT("%.1f m/s"), Speed);
}
}

void FGalaxyMapView::Layout(FVector2D Size)
{
    ViewSize = Size;
    UIScale = FMath::Max(.55f, FMath::Min(float(Size.X / 1280), float(Size.Y / 720)));
    MapRect = FBox2D(FVector2D(266 * UIScale, 90 * UIScale),
                    FVector2D(Size.X - 346 * UIScale, Size.Y - 65 * UIScale));
    DatabaseRect = FBox2D(FVector2D(Size.X - 330 * UIScale, 210 * UIScale),
                         FVector2D(Size.X - 18 * UIScale, Size.Y - 320 * UIScale));
}
double FGalaxyMapView::FocalPixels() const
{
    return FMath::Max(1., MapRect.GetSize().Y * .5 / FMath::Tan(FMath::DegreesToRadians(25.)));
}
void FGalaxyMapView::CameraBasis(FVector3d& Forward, FVector3d& Right, FVector3d& Up) const
{
    const double Yaw = FMath::DegreesToRadians(double(CameraYaw));
    const double Pitch = FMath::DegreesToRadians(double(CameraPitch));
    Forward = -FVector3d(FMath::Cos(Pitch) * FMath::Cos(Yaw), FMath::Cos(Pitch) * FMath::Sin(Yaw), FMath::Sin(Pitch));
    Right = FVector3d::CrossProduct(Forward, FVector3d::UpVector).GetSafeNormal();
    Up = FVector3d::CrossProduct(Right, Forward).GetSafeNormal();
}
bool FGalaxyMapView::Project(const FVector3d& Relative, FVector2D& Screen, double& Depth, bool bClip) const
{
    FVector3d Forward, Right, Up; CameraBasis(Forward, Right, Up);
    const FVector3d FromEye = Relative + Forward * CameraDistanceLY;
    Depth = FVector3d::DotProduct(FromEye, Forward);
    if (!FMath::IsFinite(Depth) || Depth <= CameraDistanceLY * .00001) return false;
    const double Factor = FocalPixels() / Depth;
    Screen = MapRect.GetCenter() + FVector2D(FVector3d::DotProduct(FromEye, Right), -FVector3d::DotProduct(FromEye, Up)) * Factor;
    return FMath::IsFinite(Screen.X) && FMath::IsFinite(Screen.Y) && (!bClip || MapRect.IsInside(Screen));
}
FVector3d FGalaxyMapView::RelativeBody(const FGalaxySystemView& System, const FGalaxyBodyView& Body) const
{
    // Never add local metres to a 26000 ly position: focus keeps a separate
    // local offset, preserving tiny body separations even in distant systems.
    return (System.GalaxyLightYears - PivotLY) + (Body.PositionMetres - PivotLocalMetres) / LY;
}
void FGalaxyMapView::ResetGalaxy()
{
    PivotLY = FVector3d::ZeroVector; PivotLocalMetres = FVector3d::ZeroVector;
    CameraDistanceLY = 155000; CameraYaw = -48; CameraPitch = 58;
}
void FGalaxyMapView::FocusSystem(int32 Index, const TArray<FGalaxySystemView>& Systems)
{
    if (!Systems.IsValidIndex(Index)) return;
    if (SelectedSystem != Index) { SelectedBody = 0; ScrollRow = 0; SearchQuery.Reset(); }
    SelectedSystem = Index;
    if (!Systems[Index].Bodies.IsValidIndex(SelectedBody)) SelectedBody = 0;
    PivotLY = Systems[Index].GalaxyLightYears; PivotLocalMetres = FVector3d::ZeroVector;
    double Extent = 30 * AU;
    for (const auto& Body : Systems[Index].Bodies)
        if (Body.bKnownPosition && (Body.Kind == TEXT("planet") || Body.Kind == TEXT("star")))
            Extent = FMath::Max(Extent, Body.PositionMetres.Size());
    CameraDistanceLY = FMath::Clamp(Extent * 4.5 / LY, 1e-8, 300000.);
    CameraPitch = 58;
}
void FGalaxyMapView::FocusBody(const TArray<FGalaxySystemView>& Systems)
{
    if (!Systems.IsValidIndex(SelectedSystem) || !Systems[SelectedSystem].Bodies.IsValidIndex(SelectedBody)) return;
    const auto& Body = Systems[SelectedSystem].Bodies[SelectedBody];
    if (!Body.bKnownPosition) return;
    PivotLY = Systems[SelectedSystem].GalaxyLightYears; PivotLocalMetres = Body.PositionMetres;
    CameraDistanceLY = Body.bKnownRadius ? FMath::Max(Body.RadiusMetres * 5 / LY, 1e-12) : 1e-8;
}
void FGalaxyMapView::SetSearchQuery(const FString& Query)
{
    SearchQuery = Query.Left(80); ScrollRow = 0;
}
TArray<int32> FGalaxyMapView::FilteredBodies(const TArray<FGalaxySystemView>& Systems) const
{
    TArray<int32> Result;
    if (!Systems.IsValidIndex(SelectedSystem)) return Result;
    const FString Key = SearchKey(SearchQuery);
    const auto& Bodies = Systems[SelectedSystem].Bodies;
    for (int32 I = 0; I < Bodies.Num(); ++I)
    {
        const auto& Body = Bodies[I];
        if ((KindFilter == 1 && Body.Kind != TEXT("planet")) || (KindFilter == 2 && Body.Kind != TEXT("moon"))
            || (KindFilter == 3 && (Body.Kind == TEXT("planet") || Body.Kind == TEXT("moon")))) continue;
        if (!Key.IsEmpty() && !SearchKey(Body.Name + TEXT(" ") + Body.Id + TEXT(" ") + KindName(Body.Kind)).Contains(Key)) continue;
        Result.Add(I);
    }
    return Result;
}
bool FGalaxyMapView::MarkerScreenPosition(int32 System, FVector2D& Out) const
{
    if (!SystemMarkers.IsValidIndex(System) || !SystemMarkerVisible[System]) return false;
    Out = SystemMarkers[System]; return true;
}

FMapAction FGalaxyMapView::Input(FVector2D Cursor, FVector2D Delta, bool LeftPressed,
                               bool RightHeld, bool MiddleHeld, float Wheel, bool Home,
                               const TArray<FGalaxySystemView>& Systems, FVector2D ViewportSize)
{
    FMapAction Result;
    if (!bOpen) return Result;
    Layout(ViewportSize); LastCursor = Cursor;
    auto Action = [&](FMapAction::EType Type) {
        Result.Type = Type; Result.SystemIndex = SelectedSystem;
        Result.BodyIndex = Systems.IsValidIndex(SelectedSystem) && Systems[SelectedSystem].Bodies.IsValidIndex(SelectedBody) ? SelectedBody : INDEX_NONE;
    };
    if (Home) ResetGalaxy();
    if (RightHeld && MapRect.IsInside(Cursor))
    {
        CameraYaw = FRotator::NormalizeAxis(CameraYaw + Delta.X * .28);
        CameraPitch = FMath::Clamp(CameraPitch - float(Delta.Y * .28), -89.f, 89.f);
    }
    if (MiddleHeld && MapRect.IsInside(Cursor))
    {
        FVector3d Forward, Right, Up; CameraBasis(Forward, Right, Up);
        const FVector3d Offset = (Right * -Delta.X + Up * Delta.Y) * CameraDistanceLY / FocalPixels();
        if (CameraDistanceLY < .1) PivotLocalMetres += Offset * LY;
        else PivotLY += Offset;
    }
    if (Wheel != 0)
    {
        if (DatabaseRect.IsInside(Cursor))
        {
            const int32 Rows = FMath::Max(1, FMath::FloorToInt(DatabaseRect.GetSize().Y / (24 * UIScale)));
            ScrollRow = FMath::Clamp(ScrollRow - FMath::RoundToInt(Wheel * 3), 0, FMath::Max(0, FilteredBodies(Systems).Num() - Rows));
        }
        else if (MapRect.IsInside(Cursor)) CameraDistanceLY = FMath::Clamp(CameraDistanceLY * FMath::Exp(-double(Wheel) * .55), 1e-12, 300000.);
    }
    if (!LeftPressed) return Result;
    bSearchFocused = false;
    for (const auto& Button : Buttons)
    {
        if (!Button.Rect.IsInside(Cursor)) continue;
        switch (Button.Action)
        {
        case EButton::Galaxy: ResetGalaxy(); break;
        case EButton::Top: CameraPitch = 89; break;
        case EButton::Side: CameraPitch = 0; break;
        case EButton::FocusSystem: FocusSystem(SelectedSystem, Systems); break;
        case EButton::FocusBody: FocusBody(Systems); break;
        case EButton::Hour: DesiredSeconds = 3600; Action(FMapAction::EType::Browse); break;
        case EButton::Day: DesiredSeconds = 86400; Action(FMapAction::EType::Browse); break;
        case EButton::Week: DesiredSeconds = 604800; Action(FMapAction::EType::Browse); break;
        case EButton::Navigate:
        case EButton::Inspect:
            if (Systems.IsValidIndex(SelectedSystem) && Systems[SelectedSystem].bAvailable
                && Systems[SelectedSystem].Bodies.IsValidIndex(SelectedBody) && Systems[SelectedSystem].Bodies[SelectedBody].bKnownPosition)
                Action(Button.Action == EButton::Navigate ? FMapAction::EType::Navigate : FMapAction::EType::Inspect);
            break;
        case EButton::Search: bSearchFocused = true; break;
        case EButton::All: KindFilter = 0; ScrollRow = 0; break;
        case EButton::Planets: KindFilter = 1; ScrollRow = 0; break;
        case EButton::Moons: KindFilter = 2; ScrollRow = 0; break;
        case EButton::Other: KindFilter = 3; ScrollRow = 0; break;
        }
        return Result;
    }
    for (const auto& Row : SystemRows)
        if (Row.Rect.IsInside(Cursor))
        {
            SelectedSystem = Row.Index; SelectedBody = 0; ScrollRow = 0; SearchQuery.Reset();
            Action(FMapAction::EType::Browse); return Result;
        }
    for (const auto& Row : BodyRows)
        if (Row.Rect.IsInside(Cursor))
        {
            SelectedBody = Row.Index; Action(FMapAction::EType::Browse); return Result;
        }
    if (!MapRect.IsInside(Cursor)) return Result;
    double BestDistance = FMath::Square(16 * UIScale); int32 Hit = INDEX_NONE;
    if (CameraDistanceLY < .2)
    {
        for (int32 I = 0; I < BodyMarkers.Num(); ++I)
            if (BodyMarkerVisible[I] && FVector2D::DistSquared(Cursor, BodyMarkers[I]) < BestDistance)
            { BestDistance = FVector2D::DistSquared(Cursor, BodyMarkers[I]); Hit = I; }
        if (Hit != INDEX_NONE) { SelectedBody = Hit; Action(FMapAction::EType::Browse); return Result; }
    }
    for (int32 I = 0; I < SystemMarkers.Num(); ++I)
        if (SystemMarkerVisible[I] && FVector2D::DistSquared(Cursor, SystemMarkers[I]) < BestDistance)
        { BestDistance = FVector2D::DistSquared(Cursor, SystemMarkers[I]); Hit = I; }
    if (Hit != INDEX_NONE)
    {
        SelectedSystem = Hit; SelectedBody = 0; ScrollRow = 0; SearchQuery.Reset();
        Action(FMapAction::EType::Browse);
    }
    return Result;
}

void FGalaxyMapView::GenerateGalaxy()
{
    if (!GalaxySamples.IsEmpty()) return;
    FRandomStream Random(1800304);
    auto Gaussian = [&]() { return FMath::Sqrt(-2 * FMath::Loge(FMath::Max(double(Random.FRand()), 1e-7))) * FMath::Cos(Random.FRand() * 2 * PI); };
    // Seeded illustrative barred spiral, not an invented observed star catalog.
    GalaxySamples.Reserve(35000);
    for (int32 I = 0; I < 22000; ++I)
    {
        const double R = 4400 + FMath::Pow(double(Random.FRand()), .66) * 44200;
        const double Angle = FMath::Loge(R / 6500) / FMath::Tan(.235) + (I % 4) * PI * .5 + Gaussian() * .09;
        const double Radius = R + Gaussian() * (450 + R * .019);
        const double Luminosity = .30 + Random.FRand() * .43;
        FSample Sample;
        Sample.Position = FVector3d(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, Gaussian() * (100 + R * .0025));
        const double Warm = Random.FRand();
        Sample.Color = FLinearColor(Luminosity * (.65 + Warm * .35), Luminosity * .83, Luminosity, .85f);
        Sample.Size = .55f + Random.FRand() * 1.2f; Sample.bDust = false;
        GalaxySamples.Add(Sample);
    }
    for (int32 I = 0; I < 6200; ++I)
    {
        FSample Sample;
        const double X = Gaussian() * 3850, Y = Gaussian() * 1050, Angle = .33;
        Sample.Position = FVector3d(X * FMath::Cos(Angle) - Y * FMath::Sin(Angle), X * FMath::Sin(Angle) + Y * FMath::Cos(Angle), Gaussian() * 650);
        const double Light = .4 + Random.FRand() * .45;
        Sample.Color = FLinearColor(Light, Light * .76, Light * .47, .88f);
        Sample.Size = .8f + Random.FRand(); Sample.bDust = false;
        GalaxySamples.Add(Sample);
    }
    for (int32 I = 0; I < 2400; ++I)
    {
        const double R = 9000 + Random.FRand() * 52000, Angle = Random.FRand() * 2 * PI;
        FSample Sample;
        Sample.Position = FVector3d(R * FMath::Cos(Angle), R * FMath::Sin(Angle), Gaussian() * 2400);
        Sample.Color = FLinearColor(.3f, .4f, .54f, .28f); Sample.Size = .55f; Sample.bDust = false;
        GalaxySamples.Add(Sample);
    }
    for (int32 I = 0; I < 1800; ++I)
    {
        FSample Sample;
        Sample.Position = FVector3d(Gaussian() * 1700, Gaussian() * 1700, Gaussian() * 1450);
        const double Light = .65 + Random.FRand() * .3;
        Sample.Color = FLinearColor(Light, Light * .82, Light * .60, .92f);
        Sample.Size = .9f + Random.FRand() * .8f; Sample.bDust = false;
        GalaxySamples.Add(Sample);
    }
    for (int32 I = 0; I < 1700; ++I)
    {
        const double R = 6000 + Random.FRand() * 36500;
        const double A = FMath::Loge(R / 6500) / FMath::Tan(.235) + (I % 4) * PI * .5 - .065 + Gaussian() * .035;
        FSample Sample;
        Sample.Position = FVector3d(R * FMath::Cos(A), R * FMath::Sin(A), Gaussian() * 35);
        Sample.Color = FLinearColor(.001f, .004f, .008f, .20f); Sample.Size = 5 + Random.FRand() * 7; Sample.bDust = true;
        GalaxySamples.Add(Sample);
    }
}

void FGalaxyMapView::Draw(UCanvas* Canvas, UFont* Font, const TArray<FGalaxySystemView>& Systems,
                         int32 ActiveSystem, const Daedalus::FNavigationMetrics& Metrics, bool bAncientInterface)
{
    if (!bOpen || !Canvas || !Font) return;
    Layout(FVector2D(Canvas->SizeX, Canvas->SizeY)); GenerateGalaxy();
    Buttons.Reset(); SystemRows.Reset(); BodyRows.Reset();
    SystemMarkers.SetNum(Systems.Num()); SystemMarkerVisible.Init(false, Systems.Num());
    BodyMarkers.Reset(); BodyMarkerVisible.Reset();
    const float S = UIScale, Width = ViewSize.X / S, Height = ViewSize.Y / S;
    const auto ThemeAccent=bAncientInterface?FLinearColor::FromSRGBColor(FColor(229,196,145)): ::Cyan;
    const auto ThemePale=bAncientInterface?FLinearColor::FromSRGBColor(FColor(165,184,199)): ::Pale;
    const auto ThemeWhite=bAncientInterface?FLinearColor::FromSRGBColor(FColor(229,230,215)): ::White;
    const auto ThemePlate=bAncientInterface?FLinearColor(.004f,.010f,.021f,.98f): ::Plate;
    auto Text = [&](const FString& Value, float X, float Y, FLinearColor Color = FLinearColor::Transparent, float Size = 13) {
        if(Color.A==0)Color=ThemePale;
        FCanvasTextItem Item(FVector2D(X * S, Y * S), FText::FromString(Value), FSlateFontInfo(Font, FMath::Max(8, FMath::RoundToInt(Size * S))), Color);
        Item.EnableShadow(FLinearColor::Black, FVector2D(S, S)); Canvas->DrawItem(Item);
    };
    auto Button = [&](EButton Action, const FString& Label, float X, float Y, float W, bool bSelected = false, bool bEnabled = true) {
        FBox2D Bounds(FVector2D(X * S, Y * S), FVector2D((X + W) * S, (Y + 27) * S));
        const bool Hover = Bounds.IsInside(LastCursor);
        MapPlate(Canvas, Bounds.Min, Bounds.GetSize(), bSelected ? (bAncientInterface?FLinearColor(.12f,.045f,.03f):FLinearColor(.04f,.19f,.25f)) : (Hover && bEnabled ? FLinearColor(.035f, .105f, .15f) : FLinearColor(.015f, .043f, .063f)));
        Line(Canvas, Bounds.Min, Bounds.Min + FVector2D(Bounds.GetSize().X, 0), bSelected ? ThemeAccent : FLinearColor(.1f, .22f, .28f), S);
        Text(Label, X + 8, Y + 6, bEnabled ? (bSelected || Hover ? ThemeWhite : ThemePale) : FLinearColor(.25f, .32f, .36f), 11);
        Buttons.Add({Bounds, Action});
    };
    MapPlate(Canvas, FVector2D::ZeroVector, ViewSize, Ink);
    struct FProjected { int32 Index; FVector2D Screen = FVector2D::ZeroVector; double Depth; };
    TArray<FProjected> Samples;
    for (int32 I = 0; I < GalaxySamples.Num(); ++I)
    {
        const auto& Sample = GalaxySamples[I]; FVector2D Screen = FVector2D::ZeroVector; double Depth;
        if (Project((Sample.Position - PivotLY) - PivotLocalMetres / LY, Screen, Depth)) Samples.Add({I, Screen, Depth});
    }
    Samples.Sort([](const FProjected& A, const FProjected& B) { return A.Depth > B.Depth; });
    const float GalacticFade = FMath::Clamp(float(CameraDistanceLY / 500), 0.f, 1.f);
    for (const auto& Projected : Samples)
    {
        const auto& Point = GalaxySamples[Projected.Index];
        const float Radius = FMath::Clamp(Point.Size * S * float(155000 / FMath::Max(Projected.Depth, 10000.)), .5f, 2.1f * S);
        auto Color = Point.Color; Color.A *= GalacticFade;
        if (Point.bDust)
        {
            if (CameraDistanceLY > 1500) SoftDisc(Canvas, Projected.Screen, FMath::Min(14 * S, Radius * 5), Color);
        }
        else
        {
            if (Point.Size > 1.55 && CameraDistanceLY > 1500) SoftDisc(Canvas, Projected.Screen, Radius * 3.5, FLinearColor(Color.R, Color.G, Color.B, Color.A * .14));
            MapPlate(Canvas, Projected.Screen - FVector2D(Radius * .5), FVector2D(Radius), Color);
        }
    }

    // True 3D catalogue markers and local metre detail share the projection.
    for (int32 I = 0; I < Systems.Num() && CameraDistanceLY >= .1; ++I)
    {
        FVector2D Screen = FVector2D::ZeroVector; double Depth;
        if (!Project((Systems[I].GalaxyLightYears - PivotLY) - PivotLocalMetres / LY, Screen, Depth)) continue;
        SystemMarkers[I] = Screen; SystemMarkerVisible[I] = true;
        const bool bSelected = I == SelectedSystem, bActive = I == ActiveSystem;
        const FLinearColor Color = Systems[I].bAvailable ? (bActive ? FLinearColor(.5f, 1, .74f) : ThemeAccent) : FLinearColor(.72f, .48f, .25f);
        SoftDisc(Canvas, Screen, (bSelected ? 18 : 12) * S, FLinearColor(Color.R, Color.G, Color.B, .48f));
        Ring(Canvas, Screen, (bSelected ? 8 : 5) * S, Color, S);
        MapPlate(Canvas, Screen - FVector2D(S), FVector2D(2 * S), ThemeWhite);
        if (bSelected || bActive || CameraDistanceLY < 5000)
        {
            const FVector2D Label = Screen / S + FVector2D(12, -8);
            MapPlate(Canvas, FVector2D(Label.X * S, Label.Y * S), FVector2D(125 * S, 22 * S), FLinearColor(.005f, .017f, .024f, .86f));
            Text(Short(Systems[I].Name, 18), Label.X + 4, Label.Y + 3, Color, 12);
        }
    }
    const bool HasSystem = Systems.IsValidIndex(SelectedSystem);
    const bool HasBody = HasSystem && Systems[SelectedSystem].Bodies.IsValidIndex(SelectedBody);
    const bool BodyDetail = HasBody && Systems[SelectedSystem].Bodies[SelectedBody].bKnownRadius
        && CameraDistanceLY < Systems[SelectedSystem].Bodies[SelectedBody].RadiusMetres / LY * 12;
    if (HasSystem && CameraDistanceLY < 100 && !BodyDetail)
    {
        const auto& System = Systems[SelectedSystem];
        for (const auto& Region : System.Regions)
        {
            if (!FMath::IsFinite(Region.OuterMetres) || Region.OuterMetres <= 0) continue;
            const bool Sphere = Region.Geometry.Contains(TEXT("spher"), ESearchCase::IgnoreCase);
            const FVector3d Center = (System.GalaxyLightYears - PivotLY) + (Region.CentreMetres - PivotLocalMetres) / LY;
            for (int32 Plane = 0; Plane < (Sphere ? 3 : 1); ++Plane)
            {
                for (double Metres : {Region.InnerMetres, Region.OuterMetres})
                {
                    if (!FMath::IsFinite(Metres) || Metres <= 0) continue;
                    const double Radius = Metres / LY;
                    if (Radius * FocalPixels() / CameraDistanceLY < 8 * S) continue;
                    FVector2D Last = FVector2D::ZeroVector; bool Previous = false;
                    for (int32 J = 0; J <= 128; ++J)
                    {
                        const double A = 2 * PI * J / 128;
                        FVector3d Offset(Radius * FMath::Cos(A), Radius * FMath::Sin(A), 0);
                        if (Plane == 1) Offset = FVector3d(Offset.X, 0, Offset.Y);
                        if (Plane == 2) Offset = FVector3d(0, Offset.X, Offset.Y);
                        FVector2D Screen = FVector2D::ZeroVector; double Depth; const bool Current = Project(Center + Offset, Screen, Depth);
                        if (Previous && Current) Line(Canvas, Last, Screen, Region.Color, S);
                        Last = Screen; Previous = Current;
                    }
                }
            }
            FVector2D Label = FVector2D::ZeroVector; double Depth;
            if (Region.OuterMetres / LY * FocalPixels() / CameraDistanceLY > MapRect.GetSize().X * .25
                && Project(Center + FVector3d(Region.OuterMetres / LY, 0, 0), Label, Depth))
            {
                MapPlate(Canvas, Label + FVector2D(6 * S, -5 * S), FVector2D(235 * S, 20 * S), FLinearColor(.003f, .012f, .018f, .88f));
                Text(Short(Region.Name, 36), Label.X / S + 10, Label.Y / S - 2, ThemePale, 9);
            }
        }
    }
    if (HasSystem && CameraDistanceLY < .2)
    {
        const auto& System = Systems[SelectedSystem];
        BodyMarkers.SetNum(System.Bodies.Num()); BodyMarkerVisible.Init(false, System.Bodies.Num());
        // Guides describe catalogue locations, not a second editable orbit model.
        for (int32 I = 0; I < System.Bodies.Num(); ++I)
        {
            if (BodyDetail) break; // Surface detail is already identified in the database.
            const auto& Body = System.Bodies[I];
            if (!Body.bKnownPosition || Body.Kind == TEXT("star")) continue;
            const int32 Parent = System.Bodies.IndexOfByPredicate([&](const FGalaxyBodyView& B) { return B.Id == Body.ParentId; });
            if (!System.Bodies.IsValidIndex(Parent) || !System.Bodies[Parent].bKnownPosition) continue;
            const auto& ParentBody = System.Bodies[Parent];
            if (Body.Kind != TEXT("planet") && I != SelectedBody && Parent != SelectedBody) continue;
            const FVector3d Orbit = Body.PositionMetres - ParentBody.PositionMetres;
            const double Radius = FMath::Sqrt(Orbit.X * Orbit.X + Orbit.Y * Orbit.Y);
            if (Radius / LY * FocalPixels() / CameraDistanceLY < 7 * S) continue;
            FVector2D Last = FVector2D::ZeroVector; bool bLast = false;
            for (int32 J = 0; J <= 96; ++J)
            {
                const double Angle = 2 * PI * J / 96;
                const FVector3d Relative = (System.GalaxyLightYears - PivotLY)
                    + (ParentBody.PositionMetres - PivotLocalMetres + FVector3d(Radius * FMath::Cos(Angle), Radius * FMath::Sin(Angle), Orbit.Z)) / LY;
                FVector2D Current = FVector2D::ZeroVector; double Depth; const bool bCurrent = Project(Relative, Current, Depth);
                if (bLast && bCurrent) Line(Canvas, Last, Current, I == SelectedBody ? FLinearColor(.15f, .40f, .53f, .65f) : FLinearColor(.055f, .13f, .20f, .38f), S);
                Last = Current; bLast = bCurrent;
            }
        }
        TArray<FProjected> Bodies;
        for (int32 I = 0; I < System.Bodies.Num(); ++I)
        {
            if (!System.Bodies[I].bKnownPosition) continue;
            FVector2D Screen = FVector2D::ZeroVector; double Depth;
            if (Project(RelativeBody(System, System.Bodies[I]), Screen, Depth))
            { Bodies.Add({I, Screen, Depth}); BodyMarkers[I] = Screen; BodyMarkerVisible[I] = true; }
        }
        Bodies.Sort([](const FProjected& A, const FProjected& B) { return A.Depth > B.Depth; });
        struct FBodyLabel { FVector2D Position; FString Name; bool Selected; };
        TArray<FBodyLabel> CandidateLabels;
        for (const auto& Projected : Bodies)
        {
            const auto& Body = System.Bodies[Projected.Index]; const bool Selected = Projected.Index == SelectedBody;
            const double RealRadius = Body.bKnownRadius ? Body.RadiusMetres / LY * FocalPixels() / Projected.Depth : 0;
            const float Radius = FMath::Clamp(float(RealRadius), 0.f, float(MapRect.GetSize().GetMax() * .8));
            const FLinearColor Color = Body.Kind == TEXT("star") ? FLinearColor(1, .82f, .4f) : (Body.Kind == TEXT("moon") ? FLinearColor(.64f, .74f, .8f) : ThemeAccent);
            if (Body.MapMaterial && Radius >= 3 * S)
                Canvas->K2_DrawMaterial(Body.MapMaterial, Projected.Screen - FVector2D(Radius), FVector2D(2 * Radius), FVector2D::ZeroVector, FVector2D::UnitVector);
            else
            {
                const float Size = Selected ? 3 * S : (Body.Kind == TEXT("moon") ? S : 1.7f * S);
                SoftDisc(Canvas, Projected.Screen, Size * 2.5, FLinearColor(Color.R, Color.G, Color.B, Selected ? .6f : .25f));
                MapPlate(Canvas, Projected.Screen - FVector2D(Size * .5), FVector2D(Size), Color);
            }
            if (Selected) Ring(Canvas, Projected.Screen, FMath::Max(Radius + 5 * S, 8 * S), ThemeAccent, 1.25f * S);
            if (!BodyDetail && (Selected || Body.Kind == TEXT("planet") || Radius > 8 * S))
            {
                FVector2D Position = Projected.Screen / S + FVector2D(FMath::Max(Radius / S + 8, 10.f), -6);
                if (Position.X + 140 > MapRect.Max.X / S) Position.X = Projected.Screen.X / S - Radius / S - 148;
                if (Position.X >= MapRect.Min.X / S && Position.Y >= MapRect.Min.Y / S && Position.Y + 21 < MapRect.Max.Y / S)
                    CandidateLabels.Add({Position, Short(Body.Name, 20), Selected});
            }
        }
        // Label collision uses the actual text plates; selected target has priority.
        CandidateLabels.StableSort([](const FBodyLabel& A, const FBodyLabel& B) { return A.Selected && !B.Selected; });
        TArray<FBox2D> LabelBounds;
        for (const auto& Label : CandidateLabels)
        {
            const FBox2D Bounds(Label.Position * S, (Label.Position + FVector2D(140, 21)) * S);
            if (LabelBounds.ContainsByPredicate([&](const FBox2D& Other) { return Bounds.Intersect(Other); })) continue;
            MapPlate(Canvas, Bounds.Min, Bounds.GetSize(), FLinearColor(.003f, .014f, .023f, .9f));
            Text(Label.Name, Label.Position.X + 4, Label.Position.Y + 2, Label.Selected ? ThemeWhite : ThemePale, 11);
            LabelBounds.Add(Bounds);
        }
    }

    // Opaque plates isolate crisp native-resolution text from the star field.
    MapPlate(Canvas, FVector2D::ZeroVector, FVector2D(ViewSize.X, 82 * S), ThemePlate);
    MapPlate(Canvas, FVector2D(10 * S, 92 * S), FVector2D(244 * S, ViewSize.Y - 107 * S), ThemePlate);
    MapPlate(Canvas, FVector2D(ViewSize.X - 338 * S, 92 * S), FVector2D(328 * S, ViewSize.Y - 99 * S), ThemePlate);
    Text(bAncientInterface?TEXT("ANTICKÁ HVĚZDNÁ MAPA"):TEXT("HVĚZDNÁ MAPA"), 24, 17, ThemeWhite, 24);
    Text(bAncientInterface?TEXT("Lantská databáze · soustavy a navigace"):TEXT("Prostorová mapa · soustavy a lodní databáze"), 25, 51, ThemePale, 12);
    if(bAncientInterface)
    {
        Line(Canvas,{14*S,78*S},{ViewSize.X-14*S,78*S},ThemeAccent,S);
        for(const float X:{10.f,Width-338})
        {
            const float W=X==10.f?244.f:328.f;
            const FVector2D Points[]={{(X+10)*S,92*S},{(X+W-10)*S,92*S},{(X+W)*S,102*S},
                {(X+W)*S,ViewSize.Y-18*S},{(X+W-10)*S,ViewSize.Y-8*S},{(X+10)*S,ViewSize.Y-8*S},
                {X*S,ViewSize.Y-18*S},{X*S,102*S}};
            for(int32 I=0;I<UE_ARRAY_COUNT(Points);++I)Line(Canvas,Points[I],Points[(I+1)%UE_ARRAY_COUNT(Points)],FLinearColor(.25f,.40f,.61f,.6f),S);
        }
    }
    Button(EButton::Galaxy, TEXT("Celá galaxie"), Width - 436, 20, 134);
    Button(EButton::Top, TEXT("Shora"), Width - 294, 20, 85, CameraPitch > 88);
    Button(EButton::Side, TEXT("Z boku"), Width - 201, 20, 90, FMath::Abs(CameraPitch) < .1);
    Text(TEXT("M zavřít"), Width - 99, 26, ThemeAccent, 12);
    Text(TEXT("SOUSTAVY"), 23, 107, ThemeAccent, 13);
    for (int32 I = 0; I < Systems.Num(); ++I)
    {
        const float Y = 134 + I * 45;
        FBox2D Bounds(FVector2D(20 * S, Y * S), FVector2D(244 * S, (Y + 40) * S));
        const bool Selected = I == SelectedSystem, Hover = Bounds.IsInside(LastCursor);
        MapPlate(Canvas, Bounds.Min, Bounds.GetSize(), Selected ? FLinearColor(.028f, .13f, .18f) : (Hover ? FLinearColor(.015f, .065f, .095f) : FLinearColor(.01f, .029f, .043f)));
        Text(Short(Systems[I].Name, 24), 29, Y + 5, Selected ? ThemeWhite : ThemePale, 14);
        Text(I == ActiveSystem ? TEXT("Zde je tvoje loď") : (Systems[I].bAvailable ? FString::Printf(TEXT("%d těles · připraveno"), Systems[I].Bodies.Num()) : TEXT("Čeká na podklady")), 29, Y + 24, I == ActiveSystem ? FLinearColor(.45f, .88f, .65f) : ThemePale, 9);
        SystemRows.Add({Bounds, I});
    }
    Button(EButton::FocusSystem, TEXT("Přiblížit vybranou soustavu"), 23, 424, 218, false, HasSystem);
    Text(TEXT("O MAPĚ"), 24, 473, ThemeAccent, 12);
    Text(TEXT("3D ilustrace spirální galaxie"), 24, 497, ThemePale, 10);
    Text(TEXT("Přibližný průměr 100 000 ly"), 24, 517, ThemePale, 10);
    Text(TEXT("Barva hvězd ≠ dostupné světy"), 24, 537, ThemePale, 10);
    Text(TEXT("Polohy soustav jsou fiktivní."), 24, 557, ThemePale, 10);
    Text(TEXT("Dráhy jsou orientační vodítka."), 24, 577, ThemePale, 10);
    Text(TEXT("Obálky: odhadované hranice."), 24, 597, ThemePale, 10);
    Text(TEXT("Výběr cíle loď nepřemístí."), 24, Height - 47, ThemeAccent, 10);

    const float Right = Width - 326;
    Text(HasSystem ? Short(Systems[SelectedSystem].Name, 22) + TEXT(" / DATABÁZE") : TEXT("DATABÁZE"), Right, 106, ThemeWhite, 15);
    const auto Filtered = FilteredBodies(Systems);
    Text(FString::Printf(TEXT("%d výsledků · hledání podle jména / typu"), Filtered.Num()), Right, 129, ThemePale, 10);
    Button(EButton::Search, SearchQuery.IsEmpty() ? TEXT("Hledat… klikni a piš") : Short(SearchQuery, 37) + (bSearchFocused ? TEXT(" |") : TEXT("")), Right, 145, 302, bSearchFocused);
    Button(EButton::All, TEXT("Vše"), Right, 176, 49, KindFilter == 0);
    Button(EButton::Planets, TEXT("Planety"), Right + 53, 176, 78, KindFilter == 1);
    Button(EButton::Moons, TEXT("Měsíce"), Right + 135, 176, 77, KindFilter == 2);
    Button(EButton::Other, TEXT("Ostatní"), Right + 216, 176, 86, KindFilter == 3);
    // The chips finish at203; leave a gap before the first searchable row.
    DatabaseRect.Min.Y = 210 * S;
    const int32 RowCount = FMath::Max(1, FMath::FloorToInt(DatabaseRect.GetSize().Y / (24 * S)));
    ScrollRow = FMath::Clamp(ScrollRow, 0, FMath::Max(0, Filtered.Num() - RowCount));
    for (int32 Row = 0; Row < RowCount && ScrollRow + Row < Filtered.Num(); ++Row)
    {
        const int32 Index = Filtered[ScrollRow + Row]; const auto& Body = Systems[SelectedSystem].Bodies[Index];
        const float Y = 210 + Row * 24;
        FBox2D Bounds(FVector2D(Right * S, Y * S), FVector2D((Right + 302) * S, (Y + 22) * S));
        MapPlate(Canvas, Bounds.Min, Bounds.GetSize(), Index == SelectedBody ? FLinearColor(.025f, .15f, .20f) : (Bounds.IsInside(LastCursor) ? FLinearColor(.02f, .067f, .09f) : FLinearColor(.008f, .026f, .039f)));
        Text(Short(Body.Name, 25), Right + 7, Y + 4, Index == SelectedBody ? ThemeWhite : ThemePale, 11);
        const int32 Parent = Systems[SelectedSystem].Bodies.IndexOfByPredicate([&](const FGalaxyBodyView& B) { return B.Id == Body.ParentId; });
        const FString Family = Body.Kind == TEXT("moon") && Systems[SelectedSystem].Bodies.IsValidIndex(Parent) ? Short(Systems[SelectedSystem].Bodies[Parent].Name, 9) : (Body.Kind == TEXT("moon") ? TEXT("měsíc") : (Body.Kind == TEXT("planet") ? TEXT("planeta") : TEXT("")));
        Text(!Body.bKnownPosition ? TEXT("? poloha") : Family, Right + 237, Y + 6, Body.bKnownPosition ? ThemePale : FLinearColor(.92f, .63f, .3f), 8);
        BodyRows.Add({Bounds, Index});
    }
    if (Filtered.IsEmpty()) Text(TEXT("Nenalezeno. Zkus kratší jméno."), Right + 8, 225, ThemePale, 11);
    if (Filtered.Num() > RowCount)
    {
        const float Total = DatabaseRect.GetSize().Y, Thumb = FMath::Max(12 * S, Total * RowCount / Filtered.Num());
        const float Y = DatabaseRect.Min.Y + (Total - Thumb) * ScrollRow / FMath::Max(1, Filtered.Num() - RowCount);
        MapPlate(Canvas, FVector2D((Right + 305) * S, Y), FVector2D(3 * S, Thumb), ThemeAccent);
    }
    Text(HasBody ? Short(Systems[SelectedSystem].Bodies[SelectedBody].Name, 28) : TEXT("Vyber těleso"), Right, Height - 306, ThemeWhite, 15);
    if (HasBody)
    {
        const auto& Body = Systems[SelectedSystem].Bodies[SelectedBody];
        Text(KindName(Body.Kind) + TEXT(" · R ") + (Body.bKnownRadius ? Distance(Body.RadiusMetres) : TEXT("neznámý")), Right, Height - 282, ThemePale, 10);
        Text(!Body.bKnownPosition ? TEXT("Poloha neurčena — přesun není dostupný") : Short(QualityName(Body.SourceQuality), 48), Right, Height - 262, Body.bKnownPosition ? ThemePale : FLinearColor(.92f, .63f, .3f), 9);
    }
    Button(EButton::FocusBody, TEXT("Detail vybraného tělesa"), Right, Height - 240, 302, false, HasBody && Systems[SelectedSystem].Bodies[SelectedBody].bKnownPosition);
    Text(TEXT("NAVIGACE · VZDÁLENOST MEZI STŘEDY"), Right, Height - 201, ThemeAccent, 10);
    Text(TEXT("Vzdálenost: ") + (Metrics.bValid ? Distance(Metrics.DistanceMetres) : TEXT("—")), Right, Height - 182, ThemeWhite, 12);
    Text(TEXT("Při aktuální rychlosti: ") + (Metrics.bCurrentETA ? Duration(Metrics.CurrentETASeconds) : TEXT("loď stojí / nedostupné")), Right, Height - 161, ThemePale, 10);
    Text(TEXT("Plný impuls: ") + (Metrics.bPlannedETA ? Duration(Metrics.PlannedETASeconds) : TEXT("—")), Right, Height - 143, ThemePale, 10);
    Text(TEXT("Potřebná rychlost: ") + (Metrics.bRequiredSpeed ? RequiredSpeed(Metrics.RequiredSpeedMetresPerSecond) : TEXT("—")), Right, Height - 125, ThemeAccent, 10);
    Button(EButton::Hour, TEXT("1 hodina"), Right, Height - 102, 96, DesiredSeconds == 3600);
    Button(EButton::Day, TEXT("1 den"), Right + 101, Height - 102, 94, DesiredSeconds == 86400);
    Button(EButton::Week, TEXT("1 týden"), Right + 200, Height - 102, 102, DesiredSeconds == 604800);
    const bool Ready = HasBody && Systems[SelectedSystem].bAvailable && Systems[SelectedSystem].Bodies[SelectedBody].bKnownPosition;
    Button(EButton::Navigate, TEXT("Nastavit navigační cíl"), Right, Height - 68, 302, false, Ready);
    Button(EButton::Inspect, Ready ? TEXT("TEST: přesunout loď k cíli") : TEXT("TEST: přesun není dostupný"), Right, Height - 36, 302, false, Ready);

    MapPlate(Canvas, FVector2D(MapRect.Min.X, ViewSize.Y - 55 * S), FVector2D(MapRect.GetSize().X, 43 * S), ThemePlate);
    Text(TEXT("Pravá myš: oběh · prostřední: posun · kolečko: zoom"), MapRect.Min.X / S + 12, Height - 47, ThemePale, 10);
    Text(TEXT("Kolečko nad seznamem: posun databáze · Home: galaxie"), MapRect.Min.X / S + 12, Height - 29, ThemePale, 10);
    const FString CameraLabel = TEXT("Šířka záběru ≈ ") + Distance(MapRect.GetSize().X / FocalPixels() * CameraDistanceLY * LY);
    Text(CameraLabel, MapRect.Min.X / S + 12, 97, ThemePale, 10);
}

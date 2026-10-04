#include "Solar/SolarFlightGameMode.h"
#include "Solar/SolarHUDRadar.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "GameFramework/PlayerController.h"
#include "CanvasItem.h"
#include "Solar/SolarHUDShipData.inl"
#include "Solar/SolarHUDAuroraData.inl"

namespace
{
const FLinearColor Green = FLinearColor::FromSRGBColor(FColor(169, 212, 176));
const FLinearColor Blue = FLinearColor::FromSRGBColor(FColor(135, 184, 217));
const FLinearColor White = FLinearColor::FromSRGBColor(FColor(215, 224, 230));
const FLinearColor Muted = FLinearColor::FromSRGBColor(FColor(154, 170, 181));
const FLinearColor Amber = FLinearColor::FromSRGBColor(FColor(212, 180, 112));
const FLinearColor Copper = FLinearColor::FromSRGBColor(FColor(207, 154, 120));
const FLinearColor Beam = FLinearColor::FromSRGBColor(FColor(146, 203, 229));
const FLinearColor Metal = FLinearColor::FromSRGBColor(FColor(96, 123, 139));
const FLinearColor Plate(.005f, .012f, .020f, .96f), Ink(.001f, .004f, .008f, .98f);

// UE keeps measurement protected. Expose it for fitting this particular HUD's
// runtime text; do not guess widths from character count or an offline atlas.
class FHudTextItem final : public FCanvasTextItem
{
public:
    using FCanvasTextItem::FCanvasTextItem;
    using FCanvasTextItem::GetTextSize;
};

FLinearColor Alpha(FLinearColor Color, float Value) { Color.A = Value; return Color; }

FString DistanceText(double Metres)
{
    const double Kilometres = FMath::Max(0.0, Metres) / 1000;
    if (Kilometres >= 1e6) return FString::Printf(TEXT("%.1f mil. km"), Kilometres / 1e6);
    return FString::Printf(TEXT("%.0f km"), Kilometres);
}

/** All geometry stays vector based. Runtime font sizes are final output pixels,
 * never a small pre-rendered HUD scaled up to the monitor. */
struct FInstruments
{
    UCanvas* Canvas;
    float Scale, Width, Height;

    explicit FInstruments(UCanvas* InCanvas) : Canvas(InCanvas)
    {
        Scale = FMath::Max(.35f, FMath::Min(Canvas->SizeX / 1920.f, Canvas->SizeY / 1080.f));
        Width = Canvas->SizeX / Scale; Height = Canvas->SizeY / Scale;
    }

    void Rect(float X, float Y, float W, float H, FLinearColor Color) const
    {
        FCanvasTileItem Item(FVector2D(X, Y) * Scale, FVector2D(W, H) * Scale, Color);
        Item.BlendMode = SE_BLEND_Translucent;
        Canvas->DrawItem(Item);
    }

    void Line(FVector2D A, FVector2D B, FLinearColor Color, float Thickness = 1) const
    {
        FCanvasLineItem Item(A * Scale, B * Scale);
        Item.SetColor(Color); Item.LineThickness = FMath::Max(1.f, Thickness * Scale);
        Canvas->DrawItem(Item);
    }

    void Polygon(const TArray<FVector2D>& Points, FLinearColor Fill, FLinearColor Stroke) const
    {
        if (Points.Num() < 3) return;
        TArray<FCanvasUVTri> Triangles;
        for (int32 I = 1; I + 1 < Points.Num(); ++I)
        {
            FCanvasUVTri T;
            T.V0_Pos = Points[0] * Scale; T.V1_Pos = Points[I] * Scale; T.V2_Pos = Points[I + 1] * Scale;
            T.V0_UV = T.V1_UV = T.V2_UV = FVector2D::ZeroVector;
            T.V0_Color = T.V1_Color = T.V2_Color = Fill;
            Triangles.Add(T);
        }
        const FCanvasTileItem Tile(FVector2D::ZeroVector, FVector2D::UnitVector, FLinearColor::White);
        FCanvasTriangleItem Item(Triangles, Tile.Texture); Item.BlendMode = SE_BLEND_Translucent;
        Canvas->DrawItem(Item);
        if (Stroke.A > 0) for (int32 I = 0; I < Points.Num(); ++I)
            Line(Points[I], Points[(I + 1) % Points.Num()], Stroke);
    }

    FVector2D Text(const FString& Value, float X, float Y, FLinearColor Color = White,
        float Size = 12, float MaxWidth = 0, bool Centre = false) const
    {
        FHudTextItem Item(FVector2D(X, Y) * Scale, FText::FromString(Value),
            FSlateFontInfo(GEngine->GetLargeFont(), FMath::Max(11, FMath::RoundToInt(Size * Scale))), Color);
        if (MaxWidth > 0 && Item.GetTextSize(1).X > MaxWidth * Scale)
        {
            FString Short = Value;
            do { Short.LeftChopInline(1); Item.Text = FText::FromString(Short + TEXT("…")); }
            while (!Short.IsEmpty() && Item.GetTextSize(1).X > MaxWidth * Scale);
        }
        Item.bCentreX = Centre;
        Item.EnableShadow(FLinearColor::Black, FVector2D(1, 1));
        const FVector2D TextSize = Item.GetTextSize(1) / Scale;
        Canvas->DrawItem(Item);
        return TextSize;
    }

    void Arc(FVector2D Centre, float Radius, double Start, double End, FLinearColor Color, float Thickness = 1) const
    {
        const int32 Steps = FMath::Max(8, FMath::CeilToInt((End - Start) / 3));
        for (int32 I = 0; I < Steps; ++I)
        {
            const double A = FMath::DegreesToRadians(Start + (End - Start) * I / Steps);
            const double B = FMath::DegreesToRadians(Start + (End - Start) * (I + 1) / Steps);
            Line(Centre + FVector2D(FMath::Cos(A), FMath::Sin(A)) * Radius,
                 Centre + FVector2D(FMath::Cos(B), FMath::Sin(B)) * Radius, Color, Thickness);
        }
    }

    void Panel(float X, float Y, float W, float H, const FString& Title, const FString& Tag) const
    {
        const float Cut = 6;
        Polygon({{X + Cut, Y}, {X + W - Cut, Y}, {X + W, Y + Cut}, {X + W, Y + H - Cut},
                 {X + W - Cut, Y + H}, {X + Cut, Y + H}, {X, Y + H - Cut}, {X, Y + Cut}}, Plate, Metal);
        Line({X + Cut, Y + 3}, {X + W - Cut, Y + 3}, Alpha(White, .5f), 2);
        Line({X + 3, Y + Cut}, {X + 3, Y + H - Cut}, Alpha(Metal, .65f), 2);
        Rect(X + 6, Y + 6, W - 12, 23, FLinearColor(.020f, .040f, .057f, .9f));
        Text(Title, X + 12, Y + 11, White, 11, W - (Tag.IsEmpty() ? 24 : 64));
        Text(Tag, X + W - 42, Y + 12, Muted, 8, 34);
        Line({X + 10, Y + 31}, {X + W - 10, Y + 31}, Alpha(Metal, .75f));
    }

    void Hull(FVector2D Centre, float H, FLinearColor Color, int32 Weapon = INDEX_NONE, bool bAurora = false) const
    {
        const float W = H * (bAurora ? SolarHUDAuroraData::HullAspect : SolarHUDShipData::HullAspect);
        const FVector2D Top = Centre - FVector2D(W, H) * .5;
        const auto* HullPoints = bAurora ? SolarHUDAuroraData::Hull : SolarHUDShipData::Hull;
        const int32 Count = bAurora ? UE_ARRAY_COUNT(SolarHUDAuroraData::Hull) : UE_ARRAY_COUNT(SolarHUDShipData::Hull);
        for (int32 I = 0; I < Count; ++I)
        {
            const auto& A = HullPoints[I];
            const auto& B = HullPoints[(I + 1) % Count];
            Line(Top + FVector2D(A.X * W, A.Y * H), Top + FVector2D(B.X * W, B.Y * H), Color);
        }
        if (Weapon != INDEX_NONE && !bAurora) for (const auto& Mount : SolarHUDShipData::Mounts)
        {
            if (Mount.Group != Weapon) continue;
            const FVector2D Point = Top + FVector2D(Mount.Position.X * W, Mount.Position.Y * H);
            Rect(Point.X - 1.2f, Point.Y - 1.2f, 2.4f, 2.4f, Weapon == 0 ? Copper : Weapon == 1 ? Amber : Beam);
        }
    }
};

struct FRadarBody { int32 Index; double CentreDistance; };

void DrawRadar(const FInstruments& D, const ASolarFlightGameMode& Lab)
{
    const auto& State = Lab.Flight.GetState();
    const float Left = D.Width * .03f, Top = D.Height * .035f;
    const FVector2D Centre(Left + 83, Top + 143);
    constexpr float Radius = 80;
    TArray<FRadarBody> Bodies;
    for (int32 I = 0; I < Lab.BodyDefinitions.Num(); ++I)
    {
        const auto& B = Lab.BodyDefinitions[I];
        if (!B.bKnownPosition || B.Position.ContainsNaN()) continue;
        const double Distance = (B.Position - State.PositionMetres).Size();
        if (FMath::IsFinite(Distance)) Bodies.Add({I, Distance});
    }
    Bodies.Sort([](const FRadarBody& A, const FRadarBody& B) { return A.CentreDistance < B.CentreDistance; });
    int32 Nearest = INDEX_NONE;
    double NearestDistance = 1e7;
    for (const auto& Body : Bodies)
    {
        const auto& B = Lab.BodyDefinitions[Body.Index];
        if (B.bKnownRadius && B.Radius > 0) { Nearest = Body.Index; NearestDistance = Body.CentreDistance; break; }
    }
    const double Range = SolarHUD::RadarRange(NearestDistance);
    const FString SystemName = Lab.Systems.IsValidIndex(Lab.ActiveSystem) ? Lab.Systems[Lab.ActiveSystem].Name : TEXT("Soustava");
    D.Text(SystemName.ToUpper(), Left, Top, White, 13, 330);
    if (Nearest != INDEX_NONE)
    {
        const auto& B = Lab.BodyDefinitions[Nearest];
        const double Altitude = FMath::Max(0.0, NearestDistance - B.Radius * B.Shape.GetMax());
        D.Text(B.Name + TEXT(" · nad povrchem ") + DistanceText(Altitude), Left, Top + 23, Muted, 10, 330);
    }
    for (const float Fraction : {1.f, 2.f / 3, 1.f / 3}) D.Arc(Centre, Radius * Fraction, 0, 360, Alpha(Green, .40f));
    D.Line(Centre - FVector2D(Radius, 0), Centre + FVector2D(Radius, 0), Alpha(Green, .3f));
    D.Line(Centre - FVector2D(0, Radius), Centre + FVector2D(0, Radius), Alpha(Green, .3f));

    TArray<FBox2D> Labels;
    int32 Drawn = 0;
    auto DrawBody = [&](int32 Index)
    {
        const auto& B = Lab.BodyDefinitions[Index];
        const auto Point = SolarHUD::ProjectRadar(B.Position, State.PositionMetres, State.YawDegrees, Range);
        if (!Point.bValid) return;
        const bool Selected = Index == Lab.SelectedBody;
        if (Point.bBeyondRange && !Selected) return;
        const FVector2D P = Centre + Point.UnitPosition * (Point.bBeyondRange ? Radius - 5 : Radius);
        const FLinearColor Color = Selected ? Amber : B.Kind == TEXT("star") ? Amber : B.Kind == TEXT("moon") ? Blue : Green;
        if (Point.bBeyondRange)
        {
            const FVector2D Unit = Point.UnitPosition.GetSafeNormal();
            const FVector2D Side(-Unit.Y, Unit.X);
            D.Line(P - Unit * 5 + Side * 3, P, Color);
            D.Line(P - Unit * 5 - Side * 3, P, Color);
        }
        else D.Rect(P.X - 1.7f, P.Y - 1.7f, 3.4f, 3.4f, Color);
        if (Selected) D.Arc(P, 5, 0, 360, Color);
        if (FMath::Abs(Point.HeightMetres) > Range * .03)
            D.Text(Point.HeightMetres > 0 ? TEXT("+") : TEXT("−"), P.X + 5, P.Y - 11, Color, 9);
        if (!Selected && Index != Nearest) return;
        const float LabelX = FMath::Clamp(float(P.X + 8), Left, Left + 105);
        const float LabelY = P.Y > Centre.Y ? P.Y + 6 : P.Y - 17;
        const FBox2D Box(FVector2D(LabelX, LabelY), FVector2D(LabelX + 75, LabelY + 15));
        for (const auto& Existing : Labels) if (Existing.Intersect(Box)) return;
        Labels.Add(Box);
        D.Text(B.Name, LabelX, LabelY, Color, 9, 75);
    };
    // Selected bearing stays visible even beyond local range. Other bodies are
    // actual known positions inside the radar, not fictitious nearby contacts.
    if (Lab.BodyDefinitions.IsValidIndex(Lab.SelectedBody) && Lab.BodyDefinitions[Lab.SelectedBody].bKnownPosition)
        DrawBody(Lab.SelectedBody);
    for (const auto& Body : Bodies)
    {
        if (Body.Index == Lab.SelectedBody) continue;
        DrawBody(Body.Index);
        if (++Drawn >= 32) break;
    }
    D.Polygon({Centre + FVector2D(0, -7), Centre + FVector2D(-4, 5), Centre + FVector2D(0, 2), Centre + FVector2D(4, 5)}, Green, Alpha(Green, 0));
    D.Text(TEXT("PŮDORYS · ") + DistanceText(Range), Left, Top + 236, Muted, 9, 230);
    const double Heading = FMath::Fmod(FMath::Fmod(State.YawDegrees, 360.0) + 360, 360.0);
    D.Text(TEXT("SMĚR ") + FString::Printf(TEXT("%.0f°"), Heading), Left, Top + 253, Green, 9, 230);
}

void DrawDock(const FInstruments& D, const ASolarFlightGameMode& Lab)
{
    const float DockWidth = 868, Y = D.Height - 228, H = 208;
    float X = (D.Width - DockWidth) * .5f;
    D.Panel(X, Y, 116, H, TEXT("ENERGIE"), TEXT(""));
    const TCHAR* Names[] = {TEXT("ZBR"), TEXT("ŠTÍ"), TEXT("MOT"), TEXT("SYS")};
    const FLinearColor Channels[] = {Copper, Blue, Amber, Muted};
    for (int32 I = 0; I < 4; ++I)
    {
        const float Column = X + 11 + I * 25;
        D.Text(Names[I], Column, Y + 43, Channels[I], 7, 22);
        for (int32 Row = 0; Row < 18; ++Row) D.Rect(Column + 2, Y + 64 + Row * 5.4f, 17, 3.9f, Alpha(Metal, .35f));
        D.Rect(Column, Y + 168, 22, 19, Ink); D.Text(TEXT("—"), Column + 5, Y + 170, Muted, 10);
    }
    D.Text(TEXT("NEZAPOJENO"), X + 58, Y + 192, Muted, 8, 96, true);
    X += 124;

    const FVector2D C(X + 94, Y + 94);
    // A dark instrument face keeps the green ship readable over bright planets.
    // The separate local minimap remains completely transparent.
    TArray<FVector2D> Disc;
    for (int32 I = 0; I < 64; ++I)
    {
        const double A = I * 2 * PI / 64;
        Disc.Add(C + FVector2D(FMath::Cos(A), FMath::Sin(A)) * 82);
    }
    D.Polygon(Disc, Plate, FLinearColor::Transparent);
    D.Arc(C, 88, 0, 360, Metal, 3); D.Arc(C, 85, 0, 360, White, .7f);
    D.Arc(C, 82, 0, 360, Alpha(Metal, .7f), 2);
    for (int32 I = 0; I < 4; ++I) D.Arc(C, 77, -132 + I * 90, -48 + I * 90, Blue, 2.4f);
    D.Arc(C, 57, 0, 360, Alpha(Green, .12f));
    D.Line(C - FVector2D(0, 58), C + FVector2D(0, 58), Alpha(Green, .18f));
    D.Hull(C, 115, Green, INDEX_NONE, Lab.ActiveShip == 1);
    D.Text(TEXT("PŘÍĎ"), C.X, C.Y - 67, Muted, 8, 40, true);
    D.Text(TEXT("ZÁĎ"), C.X, C.Y + 60, Muted, 8, 40, true);
    D.Rect(X + 17, Y + 187, 154, 21, Plate);
    D.Line({X + 17, Y + 187}, {X + 171, Y + 187}, Metal);
    D.Text(Lab.ActiveShip == 1 ? TEXT("AURORA") : TEXT("DAEDALUS"), C.X, Y + 191, White, 12, 140, true);
    X += 196;

    D.Panel(X, Y, 175, H, TEXT("POHON"), TEXT("ENG"));
    const auto& State = Lab.Flight.GetState();
    const double Speed = State.VelocityMetresPerSecond.Size();
    const float Fill = FMath::Clamp(float(FMath::Abs(State.Throttle)), 0.f, 1.f);
    for (int32 Row = 0; Row < 25; ++Row)
        D.Rect(X + 11, Y + 46 + Row * 5.4f, 13, 4.1f, (25 - Row) / 25.f <= Fill ? Amber : Alpha(Amber, .2f));
    for (int32 I = 0; I < 5; ++I) D.Text(FString::FromInt(100 - I * 25), X + 29, Y + 44 + I * 33, Muted, 8);
    D.Rect(X + 53, Y + 41, 110, 43, Ink);
    D.Text(FString::Printf(TEXT("%.0f"), Speed >= 1000 ? Speed / 1000 : Speed), X + 59, Y + 43, White, 23, 98);
    D.Text(Speed >= 1000 ? TEXT("km/s") : TEXT("m/s"), X + 59, Y + 87, Muted, 9, 98);
    D.Text(FString::Printf(TEXT("TAH %.0f %%"), State.Throttle * 100), X + 59, Y + 104, Amber, 12, 100);
    const TCHAR* Modes[] = {TEXT("MANÉVROVÁNÍ"), TEXT("IMPULS"), TEXT("PLNÝ IMPULS")};
    for (int32 I = 0; I < 3; ++I)
    {
        D.Rect(X + 53, Y + 127 + I * 21, 110, 18, I == Lab.ProfileIndex ? FLinearColor(.045f, .035f, .017f) : Plate);
        D.Rect(X + 53, Y + 127 + I * 21, 2, 18, I == Lab.ProfileIndex ? Amber : Metal);
        D.Text(Modes[I], X + 60, Y + 130 + I * 21, I == Lab.ProfileIndex ? Amber : Muted, 9, 98);
    }
    if (State.Throttle < 0) D.Text(TEXT("ZPĚTNÝ CHOD"), X + 88, Y + 192, Amber, 8, 130, true);
    X += 183;

    D.Panel(X, Y, 365, H, TEXT("ZBRAŇOVÉ SYSTÉMY"), TEXT("WPN"));
    const TCHAR* Groups[] = {Lab.ActiveShip == 1 ? TEXT("DRONY") : TEXT("VĚŽE"),
        Lab.ActiveShip == 1 ? TEXT("PULZY") : TEXT("RAKETY"), Lab.ActiveShip == 1 ? TEXT("REZERVA") : TEXT("PAPRSKY")};
    const TCHAR* Labels[] = {Lab.ActiveShip == 1 ? TEXT("Antické drony") : TEXT("Railguny"),
        Lab.ActiveShip == 1 ? TEXT("Emitory") : TEXT("Příďová sila"), Lab.ActiveShip == 1 ? TEXT("—") : TEXT("Emitory")};
    const FLinearColor Colors[] = {Copper, Amber, Beam};
    for (int32 I = 0; I < 3; ++I)
    {
        const float Left = X + 11 + I * 116;
        D.Rect(Left, Y + 42, 110, 141, FLinearColor(.014f, .026f, .038f));
        D.Line({Left, Y + 42}, {Left + 110, Y + 42}, Metal);
        D.Line({Left, Y + 42}, {Left, Y + 183}, Alpha(Metal, .7f));
        D.Text(Groups[I], Left + 7, Y + 49, Colors[I], 10, 96);
        D.Rect(Left + 6, Y + 69, 98, 76, Ink);
        D.Hull({Left + 55, Y + 107}, 67, Muted, I, Lab.ActiveShip == 1);
        D.Line({Left + 9, Y + 74}, {Left + 9, Y + 87}, Colors[I]);
        D.Line({Left + 9, Y + 74}, {Left + 18, Y + 74}, Colors[I]);
        D.Line({Left + 101, Y + 129}, {Left + 101, Y + 140}, Colors[I]);
        D.Line({Left + 91, Y + 140}, {Left + 101, Y + 140}, Colors[I]);
        D.Text(Labels[I], Left + 7, Y + 153, White, 9, 96);
        D.Text(TEXT("—"), Left + 55, Y + 170, Muted, 9, 96, true);
    }
    D.Text(TEXT("VÝZBROJ / ŠTÍTY — NEZAPOJENO"), X + 182, Y + 192, Muted, 8, 340, true);
}

void DrawComputer(const FInstruments& D, const ASolarFlightGameMode& Lab)
{
    const float X = D.Width - 282, Y = D.Height * .035f;
    D.Panel(X, Y, 230, 174, TEXT("PALUBNÍ POČÍTAČ"), Lab.ActiveShip == 1 ? TEXT("ANC") : TEXT("304"));
    const TCHAR* Labels[] = {TEXT("Navigace"), TEXT("Skenování"), TEXT("Údaje o lodi"), TEXT("Hyperpohon")};
    for (int32 I = 0; I < 4; ++I)
    {
        const float Left = X + 10 + (I % 2) * 107, Top = Y + 42 + (I / 2) * 32;
        D.Rect(Left, Top, 102, 27, FLinearColor(.018f, .032f, .045f));
        D.Line({Left, Top}, {Left + 102, Top}, Alpha(Metal, .8f));
        D.Text(Labels[I], Left + 7, Top + 7, Muted, 9, 88);
    }
    D.Rect(X + 10, Y + 112, 210, 48, Ink);
    const auto Nav = Lab.Navigation.Query(Lab.Flight.GetState().VelocityMetresPerSecond.Size(), Lab.Flight.GetConfig().MaxSpeed, Lab.Galaxy.DesiredSeconds);
    // Preserve the already implemented map/navigation readout. The new
    // computer sections are intentionally display-only, with no input actions.
    FString TargetName = TEXT("SYSTÉMY PŘIPRAVENÉ");
    if (Nav.bValid)
    {
        TargetName = Lab.Navigation.GetTargetBodyId();
        for (const auto& System : Lab.MapSystems) if (System.Id == Lab.Navigation.GetTargetSystemId())
            for (const auto& Body : System.Bodies) if (Body.Id == TargetName) { TargetName = System.Name + TEXT(" / ") + Body.Name; break; }
    }
    const FString RangeText = Nav.DistanceMetres >= Daedalus::FNavigationPlan::LightYearMetres * .01
        ? FString::Printf(TEXT("%.2f světelných let"), Nav.DistanceMetres / Daedalus::FNavigationPlan::LightYearMetres)
        : DistanceText(Nav.DistanceMetres);
    D.Text(TargetName, X + 18, Y + 119, Blue, 9, 194);
    D.Text(Nav.bValid ? RangeText : TEXT("Funkce doplníme později"), X + 18, Y + 139, Muted, 9, 194);
}

void DrawPauseMenu(const FInstruments& D, const ASolarFlightGameMode& Lab)
{
    const auto V = BuildSolarPauseView(Lab, FVector2D(D.Canvas->SizeX, D.Canvas->SizeY));
    D.Rect(0,0,D.Width,D.Height,FLinearColor(0,0,0,.75f));
    D.Panel(V.X,V.Y,V.Width,V.Height,V.Title,TEXT("PAUSE"));
    D.Text(V.Description,V.X+18,V.Y+45,Muted,10,V.Width-36);
    float MouseX=0,MouseY=0;
    if(auto* PC=Lab.GetWorld()->GetFirstPlayerController())PC->GetMousePosition(MouseX,MouseY);
    for(const auto& B:V.Buttons)
    {
        const FVector2D A=B.Pixels.Min/V.Scale,Size=B.Pixels.GetSize()/V.Scale;
        const bool Hover=B.bEnabled&&B.Pixels.IsInside(FVector2D(MouseX,MouseY));
        D.Rect(A.X,A.Y,Size.X,Size.Y,Hover?FLinearColor(.04f,.075f,.10f):Ink);
        D.Line(A,A+FVector2D(Size.X,0),B.bSelected?Green:Hover?Blue:Metal);
        D.Rect(A.X,A.Y,3,Size.Y,B.bSelected?Green:Hover?Blue:Metal);
        D.Text(B.Label,A.X+14,A.Y+8,B.bEnabled||B.bSelected?White:Muted,12,205);
        D.Text(B.Hint,A.X+14,A.Y+27,B.bSelected?Green:Muted,9,Size.X-28);
        if(B.bSelected)D.Text(TEXT("AKTIVNÍ"),A.X+Size.X-86,A.Y+9,Green,10,72);
    }
    D.Text(TEXT("P — ZPĚT DO HRY"),V.X+18,V.Y+V.Height-25,Muted,9,V.Width-36);
    if(!Lab.Message.IsEmpty())D.Text(Lab.Message,V.X,V.Y+V.Height+10,Amber,11,V.Width);
}
}

void ASolarFlightHUD::DrawHUD()
{
    Super::DrawHUD();
    const auto* Lab = GetWorld() ? Cast<ASolarFlightGameMode>(GetWorld()->GetAuthGameMode()) : nullptr;
    if (!Lab || !Canvas || !GEngine) return;
    if (Lab->bReady && Lab->Galaxy.bOpen)
    {
        // The existing map owns its interaction; HUD adds no map hit targets.
        auto* MutableLab = Cast<ASolarFlightGameMode>(GetWorld()->GetAuthGameMode());
        MutableLab->Galaxy.Draw(Canvas, GEngine->GetLargeFont(), Lab->MapSystems, Lab->ActiveSystem, Lab->PreviewMetrics());
        return;
    }
    const FInstruments D(Canvas);
    if (!Lab->bReady) { D.Text(Lab->Message, 30, 30, FLinearColor::Red, 16, D.Width - 60); return; }
    DrawRadar(D, *Lab);
    DrawDock(D, *Lab);
    DrawComputer(D, *Lab);
    if(Lab->bPaused){DrawPauseMenu(D,*Lab);return;}
    float WarningY = 35;
    auto Warning = [&](const FString& Message, FLinearColor Color)
    {
        D.Rect((D.Width - 620) * .5f, WarningY, 620, 28, Plate);
        D.Text(Message, D.Width * .5f, WarningY + 6, Color, 12, 594, true);
        WarningY += 33;
    };
    if (Lab->bPaused) Warning(TEXT("POZASTAVENO"), Blue);
    if (!Lab->Flight.GetState().ContactBodyId.IsEmpty()) Warning(TEXT("Povrchová ochrana: otoč se a odleť."), Amber);
    if (!Lab->Message.IsEmpty()) Warning(Lab->Message, Muted);
}

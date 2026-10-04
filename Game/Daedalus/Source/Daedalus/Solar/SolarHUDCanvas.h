#pragma once
#include "Solar/SolarFlightGameMode.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "CanvasItem.h"
#include "Solar/SolarHUDShipData.inl"
#include "Solar/SolarHUDAuroraData.inl"

namespace SolarHUDUI
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

inline FLinearColor Alpha(FLinearColor Color, float Value) { Color.A = Value; return Color; }

inline FString DistanceText(double Metres)
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

void DrawRadar(const FInstruments& D, const ASolarFlightGameMode& Lab, bool bAncient = false);
void DrawAncientHUD(const FInstruments& D, const ASolarFlightGameMode& Lab);
void AncientPanel(const FInstruments& D, float X, float Y, float W, float H, const FString& Title);
}

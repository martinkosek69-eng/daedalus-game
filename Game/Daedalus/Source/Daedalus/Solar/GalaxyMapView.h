#pragma once

#include "CoreMinimal.h"
#include "DaedalusNavigationModel.h"

class UCanvas;
class UFont;
class UMaterialInterface;

struct FGalaxyBodyView
{
    FString Id, Name, Kind, SourceQuality, ParentId;
    FVector3d PositionMetres = FVector3d::ZeroVector;
    double RadiusMetres = 0;
    bool bKnownRadius = false;
    bool bKnownPosition = true;
    UMaterialInterface* MapMaterial = nullptr; // Root owns the UObject lifetime.
};

struct FGalaxyRegionView
{
    FString Name, Geometry;
    FVector3d CentreMetres = FVector3d::ZeroVector;
    double InnerMetres = 0, OuterMetres = 0;
    FLinearColor Color = FLinearColor(.2f, .45f, .65f, .25f);
};

struct FGalaxySystemView
{
    FString Id, Name;
    FVector3d GalaxyLightYears = FVector3d::ZeroVector;
    bool bAvailable = false;
    TArray<FGalaxyBodyView> Bodies;
    TArray<FGalaxyRegionView> Regions;
};

struct FMapAction
{
    enum class EType : uint8 { None, Browse, Navigate, Inspect };
    EType Type = EType::None;
    int32 SystemIndex = INDEX_NONE, BodyIndex = INDEX_NONE;
};

/** A disposable map view. Selection/camera never modify canonical ship state. */
class FGalaxyMapView
{
public:
    bool bOpen = false;
    int32 SelectedSystem = 0, SelectedBody = 0;
    double DesiredSeconds = 3600;
    FString SearchQuery;
    bool bSearchFocused = false;

    FMapAction Input(FVector2D Cursor, FVector2D Delta, bool LeftPressed,
                     bool RightHeld, bool MiddleHeld, float Wheel, bool Home,
                     const TArray<FGalaxySystemView>& Systems, FVector2D ViewportSize);
    void Draw(UCanvas* Canvas, UFont* Font, const TArray<FGalaxySystemView>& Systems,
              int32 ActiveSystem, const Daedalus::FNavigationMetrics& PreviewMetrics, bool bAncientInterface = false);
    bool MarkerScreenPosition(int32 System, FVector2D& Out) const;
    void FocusSystem(int32 Index, const TArray<FGalaxySystemView>& Systems);
    void SetSearchQuery(const FString& Query);
    FVector3d GetPivotLY() const { return PivotLY; }
    double GetCameraDistanceLY() const { return CameraDistanceLY; }
    float GetCameraPitchDegrees() const { return CameraPitch; }

private:
    enum class EButton : uint8
    {
        Galaxy, Top, Side, FocusSystem, FocusBody, Hour, Day, Week,
        Navigate, Inspect, Search, All, Planets, Moons, Other
    };
    struct FButton { FBox2D Rect; EButton Action; };
    struct FRow { FBox2D Rect; int32 Index; };
    struct FSample { FVector3d Position; FLinearColor Color; float Size; bool bDust; };

    FVector3d PivotLY = FVector3d::ZeroVector;
    FVector3d PivotLocalMetres = FVector3d::ZeroVector;
    double CameraDistanceLY = 155000;
    float CameraYaw = -48, CameraPitch = 58;
    FVector2D LastCursor = FVector2D::ZeroVector;
    FVector2D ViewSize = FVector2D(1280, 720);
    FBox2D MapRect, DatabaseRect;
    TArray<FSample> GalaxySamples;
    TArray<FButton> Buttons;
    TArray<FRow> SystemRows, BodyRows;
    TArray<FVector2D> SystemMarkers, BodyMarkers;
    TArray<bool> SystemMarkerVisible, BodyMarkerVisible;
    int32 ScrollRow = 0, KindFilter = 0;
    float UIScale = 1;

    void Layout(FVector2D Size);
    void GenerateGalaxy();
    void ResetGalaxy();
    void FocusBody(const TArray<FGalaxySystemView>& Systems);
    TArray<int32> FilteredBodies(const TArray<FGalaxySystemView>& Systems) const;
    FVector3d RelativeBody(const FGalaxySystemView& System, const FGalaxyBodyView& Body) const;
    void CameraBasis(FVector3d& Forward, FVector3d& Right, FVector3d& Up) const;
    bool Project(const FVector3d& Relative, FVector2D& Screen, double& Depth,
                 bool bClip = true) const;
    double FocalPixels() const;
};

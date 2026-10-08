#pragma once

#include "CoreMinimal.h"

namespace SolarHUD
{
/** Disposable view coordinates. Canonical system positions are always metres.
 * The planar radar follows heading, independent of camera orbit/pitch/bank.
 * Its outer ring represents RangeMetres in the system's XY plane. */
struct FRadarPoint
{
    FVector2D UnitPosition = FVector2D::ZeroVector;
    double DistanceMetres = 0;
    double HeightMetres = 0;
    bool bValid = false;
    bool bBeyondRange = false;
};

FRadarPoint ProjectRadar(const FVector3d& BodyMetres, const FVector3d& ShipMetres,
    double HeadingDegrees, double RangeMetres);

/** Readable 1/2/5 decade scale; no persistent simulation parameter. */
double RadarRange(double NearestCentreMetres);
}

#include "Solar/SolarHUDRadar.h"

namespace SolarHUD
{
FRadarPoint ProjectRadar(const FVector3d& BodyMetres, const FVector3d& ShipMetres,
    double HeadingDegrees, double RangeMetres)
{
    FRadarPoint Result;
    if (BodyMetres.ContainsNaN() || ShipMetres.ContainsNaN()
        || !FMath::IsFinite(HeadingDegrees) || !FMath::IsFinite(RangeMetres) || RangeMetres <= 0)
        return Result;
    const FVector3d Delta = BodyMetres - ShipMetres;
    const double Radians = FMath::DegreesToRadians(FMath::Fmod(HeadingDegrees, 360.0));
    const double Cos = FMath::Cos(Radians), Sin = FMath::Sin(Radians);
    const FVector2D Planar((-Delta.X * Sin + Delta.Y * Cos) / RangeMetres,
                          (-Delta.X * Cos - Delta.Y * Sin) / RangeMetres);
    const double Length = Planar.Size();
    if (!FMath::IsFinite(Length) || !FMath::IsFinite(Delta.Size())) return Result;
    Result.UnitPosition = Length > 1 ? Planar / Length : Planar;
    Result.DistanceMetres = Delta.Size();
    Result.HeightMetres = Delta.Z;
    Result.bBeyondRange = Length > 1;
    Result.bValid = true;
    return Result;
}

double RadarRange(double NearestCentreMetres)
{
    // Keep a useful local context even around a tiny asteroid. Scale follows
    // the nearest physical body, not low-detail mesh bounds or selected targets.
    const double Desired = FMath::Clamp(FMath::IsFinite(NearestCentreMetres)
        ? NearestCentreMetres * 1.6 : 1e7, 5e6, 1e16);
    const double Decade = FMath::Pow(10.0, FMath::Floor(FMath::LogX(10.0, Desired)));
    for (const double Factor : {1.0, 2.0, 5.0, 10.0})
        if (Decade * Factor >= Desired * (1 - 1e-12)) return Decade * Factor;
    return Decade * 10;
}
}

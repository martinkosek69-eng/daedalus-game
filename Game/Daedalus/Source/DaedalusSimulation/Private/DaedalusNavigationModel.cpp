#include "DaedalusNavigationModel.h"
#include <limits>

namespace Daedalus
{
namespace
{
bool Fail(FString& Error, const TCHAR* Message)
{
    Error = Message;
    return false;
}

bool ValidId(const FString& Id)
{
    if (Id.IsEmpty() || Id.Len() > 128) return false;
    for (TCHAR Character : Id)
    {
        if (!((Character >= 'a' && Character <= 'z') || (Character >= 'A' && Character <= 'Z')
            || (Character >= '0' && Character <= '9') || Character == '-' || Character == '_' || Character == '.')) return false;
    }
    return true;
}

bool ValidName(const FString& Name)
{
    return !Name.TrimStartAndEnd().IsEmpty() && Name.Len() <= 1024;
}

bool ValidPosition(const FVector3d& Position, double Limit)
{
    return FMath::IsFinite(Position.X) && FMath::IsFinite(Position.Y) && FMath::IsFinite(Position.Z)
        && Position.GetAbsMax() <= Limit;
}

bool AcyclicParents(const FNavigationSystem& System, const TMap<FString, int32>& Indices)
{
    // Each body has at most one parent. Iterative coloring keeps validation
    // linear and avoids recursion/stack exhaustion on a 10000-body chain.
    TArray<uint8> Colors;
    Colors.SetNumZeroed(System.Bodies.Num());
    TArray<int32> Path;
    for (int32 Root = 0; Root < System.Bodies.Num(); ++Root)
    {
        if (Colors[Root] == 2) continue;
        Path.Reset();
        int32 Cursor = Root;
        while (Cursor != INDEX_NONE)
        {
            if (Colors[Cursor] == 1) return false;
            if (Colors[Cursor] == 2) break;
            Colors[Cursor] = 1;
            Path.Add(Cursor);
            const FString& Parent = System.Bodies[Cursor].ParentId;
            Cursor = Parent.IsEmpty() ? INDEX_NONE : Indices.FindChecked(Parent);
        }
        for (int32 Visited : Path) Colors[Visited] = 2;
    }
    return true;
}

bool FiniteNonNegativeQuotient(double Numerator, double Denominator, double& Result)
{
    if (!FMath::IsFinite(Denominator) || Denominator <= 0) return false;
    const double Value = Numerator / Denominator;
    if (!FMath::IsFinite(Value) || Value < 0) return false;
    Result = Value;
    return true;
}
}

bool FNavigationPlan::Configure(const TArray<FNavigationSystem>& InSystems, FString& Error)
{
    if (InSystems.IsEmpty() || InSystems.Num() > MaxSystems)
        return Fail(Error, TEXT("Navigation requires a bounded non-empty system catalog."));

    TMap<FString, int32> NewSystemIndices;
    TArray<TMap<FString, int32>> NewBodyIndices;
    NewBodyIndices.SetNum(InSystems.Num());
    int32 TotalBodies = 0;
    for (int32 SystemIndex = 0; SystemIndex < InSystems.Num(); ++SystemIndex)
    {
        const FNavigationSystem& System = InSystems[SystemIndex];
        if (!ValidId(System.Id) || !ValidName(System.Name) || NewSystemIndices.Contains(System.Id)
            || !ValidPosition(System.GalaxyPositionLightYears, MaxGalaxyCoordinateLightYears)
            || System.Bodies.IsEmpty() || System.Bodies.Num() > MaxBodiesPerSystem)
            return Fail(Error, TEXT("Navigation system IDs, names, galaxy coordinates or body counts are invalid."));
        TotalBodies += System.Bodies.Num();
        if (TotalBodies > MaxTotalBodies)
            return Fail(Error, TEXT("Navigation total body limit exceeded."));
        NewSystemIndices.Add(System.Id, SystemIndex);
        auto& Indices = NewBodyIndices[SystemIndex];
        for (int32 BodyIndex = 0; BodyIndex < System.Bodies.Num(); ++BodyIndex)
        {
            const FNavigationBody& Body = System.Bodies[BodyIndex];
            if (!ValidId(Body.Id) || !ValidName(Body.Name) || Indices.Contains(Body.Id)
                || (!Body.ParentId.IsEmpty() && !ValidId(Body.ParentId))
                || !ValidPosition(Body.PositionMetres, MaxLocalCoordinateMetres)
                || !FMath::IsFinite(Body.RadiusMetres)
                || (Body.bKnownRadius ? Body.RadiusMetres <= 0 || Body.RadiusMetres > MaxLocalCoordinateMetres : Body.RadiusMetres != 0))
                return Fail(Error, TEXT("Navigation body IDs, names, local coordinates or known/unknown radii are invalid."));
            Indices.Add(Body.Id, BodyIndex);
        }
        for (const FNavigationBody& Body : System.Bodies)
            if (!Body.ParentId.IsEmpty() && !Indices.Contains(Body.ParentId))
                return Fail(Error, TEXT("Navigation body parent must exist in the same system."));
        if (!AcyclicParents(System, Indices))
            return Fail(Error, TEXT("Navigation body parent graph contains a cycle."));
    }

    // Finish all validation/copies before replacing any catalog or selection.
    TArray<FNavigationSystem> NewSystems = InSystems;
    const FString NewSystemId = NewSystems[0].Id;
    const FString NewBodyId = NewSystems[0].Bodies[0].Id;
    Systems = MoveTemp(NewSystems);
    SystemIndices = MoveTemp(NewSystemIndices);
    BodyIndices = MoveTemp(NewBodyIndices);
    LocationSystemId = NewSystemId;
    TargetSystemId = NewSystemId;
    TargetBodyId = NewBodyId;
    LocationMetres = FVector3d::ZeroVector;
    Error.Reset();
    return true;
}

bool FNavigationPlan::SetLocation(const FString& SystemId, const FVector3d& PositionMetres, FString& Error)
{
    if (!SystemIndices.Contains(SystemId) || !ValidPosition(PositionMetres, MaxLocalCoordinateMetres))
        return Fail(Error, TEXT("Navigation location needs a configured system and finite bounded local metres."));
    LocationSystemId = SystemId;
    LocationMetres = PositionMetres;
    Error.Reset();
    return true;
}

bool FNavigationPlan::SelectTarget(const FString& SystemId, const FString& BodyId, FString& Error)
{
    const int32* SystemIndex = SystemIndices.Find(SystemId);
    if (!SystemIndex || !BodyIndices[*SystemIndex].Contains(BodyId))
        return Fail(Error, TEXT("Navigation target must identify a configured system/body pair."));
    TargetSystemId = SystemId;
    TargetBodyId = BodyId;
    Error.Reset();
    return true;
}

FNavigationMetrics FNavigationPlan::Query(double CurrentSpeed, double PlannedSpeed, double DesiredSeconds) const
{
    FNavigationMetrics Result;
    const int32* LocationIndex = SystemIndices.Find(LocationSystemId);
    const int32* TargetIndex = SystemIndices.Find(TargetSystemId);
    if (!LocationIndex || !TargetIndex) return Result;
    const int32* BodyIndex = BodyIndices[*TargetIndex].Find(TargetBodyId);
    if (!BodyIndex) return Result;

    const auto& Target = Systems[*TargetIndex];
    // Never first construct two huge absolute metre positions: doing so loses
    // centimetres even when both points are in the SAME distant galaxy sector.
    FVector3d Delta = Target.Bodies[*BodyIndex].PositionMetres - LocationMetres;
    if (*LocationIndex != *TargetIndex)
        Delta += (Target.GalaxyPositionLightYears - Systems[*LocationIndex].GalaxyPositionLightYears) * LightYearMetres;
    if (!ValidPosition(Delta, std::numeric_limits<double>::max())) return Result;
    const double Largest = Delta.GetAbsMax();
    Result.DistanceMetres = Largest > 0 ? Largest * (Delta / Largest).Size() : 0;
    if (!FMath::IsFinite(Result.DistanceMetres)) return FNavigationMetrics();
    Result.Direction = Result.DistanceMetres > 0 ? Delta / Result.DistanceMetres : FVector3d::ZeroVector;
    Result.bValid = true;
    Result.bCurrentETA = FiniteNonNegativeQuotient(Result.DistanceMetres, CurrentSpeed, Result.CurrentETASeconds);
    Result.bPlannedETA = FiniteNonNegativeQuotient(Result.DistanceMetres, PlannedSpeed, Result.PlannedETASeconds);
    Result.bRequiredSpeed = FiniteNonNegativeQuotient(Result.DistanceMetres, DesiredSeconds, Result.RequiredSpeedMetresPerSecond);
    return Result;
}
}

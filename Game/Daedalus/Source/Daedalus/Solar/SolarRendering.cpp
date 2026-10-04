#include "Solar/SolarFlightGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"

void ASolarFlightGameMode::ApplySharpRenderingSettings()
{
    auto* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;
    // Apply AFTER user scalability/fullscreen settings. No history or broad
    // post-AA filter over the fine hull plating and single-pixel distant stars.
    // High-DPI awareness is separately enabled in Engine.ini before window creation.
    for (const TCHAR* Command : {
        TEXT("r.AntiAliasingMethod 0"), TEXT("r.MotionBlurQuality 0"),
        TEXT("r.DepthOfFieldQuality 0"), TEXT("r.SceneColorFringeQuality 0"),
        TEXT("r.ScreenPercentage 100"), TEXT("r.SecondaryScreenPercentage.GameViewport 100"),
        TEXT("r.DynamicRes.OperationMode 0"), TEXT("r.Tonemapper.Sharpen 0.25"),
        TEXT("r.MaxAnisotropy 16"), TEXT("r.Streaming.MipBias 0"),
        TEXT("r.ForceLOD 0"), TEXT("r.BloomQuality 0")})
        PC->ConsoleCommand(Command);
}

void ASolarFlightGameMode::UpdateTextureDetail()
{
    // Quality-first default loads all mip levels. The selective policy below
    // remains available if texture streaming is explicitly enabled later.
    if (IConsoleManager::Get().FindConsoleVariable(TEXT("r.TextureStreaming"))->GetInt() == 0) return;
    // Projected celestial meshes change scale at runtime and have no baked
    // streaming build data. Choose by true angular size instead of relying on
    // that missing UV density or the user's selected navigation target.
    TArray<TPair<int32, double>> Candidates;
    for (int32 Index = 0; Index < BodyDefinitions.Num(); ++Index)
    {
        if (!BodyMeshes.IsValidIndex(Index) || !BodyMeshes[Index]) continue;
        const auto& Body = BodyDefinitions[Index];
        const double AngularRadius = Body.Radius / FMath::Max(Body.Radius,
            (Body.Position - Flight.GetState().PositionMetres).Size());
        if (AngularRadius > .004) Candidates.Emplace(Index, AngularRadius);
    }
    Candidates.Sort([](const auto& A, const auto& B) { return A.Value > B.Value; });
    TSet<int32> Wanted;
    // Bound VRAM use: pin only the four largest nearby discs, plus ship/sky.
    // Remaining bodies still use the normal screen-size texture streamer.
    for (int32 I = 0; I < FMath::Min(4, Candidates.Num()); ++I) Wanted.Add(Candidates[I].Key);
    for (int32 Old : DetailedBodies)
        if (!Wanted.Contains(Old) && BodyMeshes.IsValidIndex(Old) && BodyMeshes[Old])
            BodyMeshes[Old]->SetTextureForceResidentFlag(false);
    for (int32 Current : Wanted) BodyMeshes[Current]->SetTextureForceResidentFlag(true);
    for (int32 I = 0; I < RingMeshes.Num(); ++I)
        if (DetailedBodies.Contains(RingBodies[I]) != Wanted.Contains(RingBodies[I]))
            RingMeshes[I]->SetTextureForceResidentFlag(Wanted.Contains(RingBodies[I]));
    DetailedBodies = MoveTemp(Wanted);
}

void ASolarFlightGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (Ship) Ship->SetTextureForceResidentFlag(false);
    if (Sky) Sky->SetTextureForceResidentFlag(false);
    for (const auto& Detail : ShipDetails) if (Detail) Detail->SetTextureForceResidentFlag(false);
    for (const auto& Mesh : BodyMeshes) if (Mesh) Mesh->SetTextureForceResidentFlag(false);
    for (const auto& Ring : RingMeshes) if (Ring) Ring->SetTextureForceResidentFlag(false);
    Super::EndPlay(EndPlayReason);
}

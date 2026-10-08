#include "Solar/SolarFlightGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "InputKeyEventArgs.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UObjectIterator.h"
#include "UnrealClient.h"
#if WITH_EDITOR
#include "AssetCompilingManager.h"
#include "ShaderCompiler.h"
#endif

// Planet quality diagnostics: -SolarSharpProbe=<dir> -SolarPlanetProbe [-SolarPlanetBodies=a,b,...]
// Frame-scripted 4K captures of Earth (close reference-like view, full disc, limb, terminator,
// ocean glint, motion) and optional representative bodies, plus GPU frame time and resident
// texture memory of the whole active system. Moves only the probe's own flight state.
namespace
{
struct FPlanetShot
{
    FString Name;
    FVector3d Camera = FVector3d::ZeroVector, Look = FVector3d(1, 0, 0);
    double Clock = 0;
    int32 System = 0, Body = INDEX_NONE;   // Body != NONE: standard inspection view via ActivateSystem
    bool bShip = false, bMoving = false, bClouds = true;
};

FVector3d SurfaceNormal(const FSolarBodyDefinition& Body, double LatDeg, double LonDeg, double Clock)
{
    // Body space: u = 0.5 - phi/2pi with lon = -phi (Tools/Prepare-PlanetMaterials.py).
    const double Lat = FMath::DegreesToRadians(LatDeg), Phi = FMath::DegreesToRadians(-LonDeg);
    const FVector3d Local(FMath::Cos(Lat) * FMath::Cos(Phi), FMath::Cos(Lat) * FMath::Sin(Phi), FMath::Sin(Lat));
    const double Angle = Body.RotationHours == 0 ? 0 : FMath::Fmod(Clock * 360 / (Body.RotationHours * 3600), 360.);
    const FQuat4d Rotation = FQuat4d(FVector3d::ForwardVector, FMath::DegreesToRadians(Body.Tilt)) * FQuat4d(FVector3d::UpVector, FMath::DegreesToRadians(Angle));
    return Rotation.RotateVector(Local);
}

// Clock at which the site has the requested sun elevation cosine (morning side), searching one day.
double ClockForSun(const FSolarBodyDefinition& Body, const FVector3d& ToSun, double LatDeg, double LonDeg, double WantCos)
{
    const double Day = FMath::Abs(Body.RotationHours) * 3600;
    double Best = 0, BestError = 1e9;
    for (int32 Step = 0; Step < 1440; ++Step)
    {
        const double Clock = Day * Step / 1440;
        const double Cos = FVector3d::DotProduct(SurfaceNormal(Body, LatDeg, LonDeg, Clock), ToSun);
        const double Next = FVector3d::DotProduct(SurfaceNormal(Body, LatDeg, LonDeg, Clock + Day / 1440), ToSun);
        if (Next > Cos && FMath::Abs(Cos - WantCos) < BestError) { BestError = FMath::Abs(Cos - WantCos); Best = Clock; }
    }
    return Best;
}

// Oblique view of a surface site from Distance, tilted towards world up so camera pitch stays moderate.
void Oblique(FPlanetShot& Shot, const FSolarBodyDefinition& Body, const FVector3d& Normal, double Distance)
{
    FVector3d Tangent = FVector3d::UpVector - Normal * FVector3d::DotProduct(Normal, FVector3d::UpVector);
    Tangent = Tangent.IsNearlyZero() ? FVector3d::ForwardVector : Tangent.GetSafeNormal();
    const double Beta = FMath::Clamp(FMath::Asin(FMath::Clamp(Normal.Z, -1., 1.)), FMath::DegreesToRadians(35.), FMath::DegreesToRadians(65.));
    Shot.Look = (-Normal * FMath::Cos(Beta) + Tangent * FMath::Sin(Beta)).GetSafeNormal();
    Shot.Camera = Body.Position + Normal * Body.Radius - Shot.Look * Distance;
}
}

void ASolarFlightGameMode::TickPlanetProbe()
{
    ++ProbeFrame;
    GEngine->bUseFixedFrameRate = true; GEngine->FixedFrameRate = 60;
    auto* PC = GetWorld()->GetFirstPlayerController();
    auto* Pawn = PC ? Cast<ASolarFlightPawn>(PC->GetPawn()) : nullptr;
    static TArray<FPlanetShot> Shots;
    static FString Report;
    static double GpuSum = 0, FrameSum = 0, GpuMemory = 0; static int32 Samples = 0;
    static FString PendingRow;
    constexpr int32 Window = 100, ShotAt = 60;
    auto Check = [&](bool Pass, const FString& Label) {
        bProbePassed &= Pass;
        UE_LOG(LogTemp, Display, TEXT("PLANET_PROBE %s %s"), *Label, Pass ? TEXT("PASS") : TEXT("FAIL"));
    };
    auto SetShipVisible = [&](bool bVisible) {
        if (Ship) Ship->SetVisibility(bVisible);
        if (HullLights) HullLights->SetVisibility(bVisible);
        for (const auto& Door : HangarDoors) if (Door) Door->SetVisibility(bVisible);
        for (const auto& Mesh : ShipDetails) if (Mesh) Mesh->SetVisibility(bVisible);
        for (const auto& Mesh : EngineGlows) if (Mesh) Mesh->SetVisibility(bVisible);
    };
    if (!bReady || Systems.IsEmpty())
    {
        Check(false, TEXT("scene ready"));
        FFileHelper::SaveStringToFile(TEXT("{\"passed\":false,\"error\":\"scene not ready\"}"), *(ProbeDirectory / TEXT("result.json")));
        FPlatformMisc::RequestExitWithStatus(false, 1);
        return;
    }
    if (ProbeFrame == 1)
    {
#if WITH_EDITOR
        // Uncooked editor runs compile textures/shaders asynchronously; finish before any capture.
        FAssetCompilingManager::Get().FinishAllCompilation();
        if (GShaderCompilingManager) GShaderCompilingManager->FinishAllCompilation();
#endif
        Shots.Reset(); Report = TEXT("{\"shots\":[");
        int32 EarthIndex = INDEX_NONE;
        for (int32 I = 0; I < Systems[0].Bodies.Num(); ++I) if (Systems[0].Bodies[I].Id == TEXT("sol.earth")) EarthIndex = I;
        Check(EarthIndex != INDEX_NONE, TEXT("earth present"));
        if (EarthIndex != INDEX_NONE)
        {
            const auto& Home = Systems[0].Bodies[EarthIndex];
            const FVector3d ToSun = (SunPosition - Home.Position).GetSafeNormal();
            // Close reference-like view over northern England/Scotland (0022 REFERENCE: ~0.87 km/px).
            FPlanetShot Near; Near.Name = TEXT("earth-reference"); Near.Clock = ClockForSun(Home, ToSun, 54, -3, .55);
            Oblique(Near, Home, SurfaceNormal(Home, 54, -3, Near.Clock), 3.4e6); Shots.Add(Near);
            FPlanetShot WithShip = Near; WithShip.Name = TEXT("earth-reference-ship"); WithShip.bShip = true; Shots.Add(WithShip);
            // Diagnostic only: same view without the cloud layer to judge coast, relief and water.
            FPlanetShot Surface = Near; Surface.Name = TEXT("earth-reference-surface"); Surface.bClouds = false; Shots.Add(Surface);
            for (int32 I = 0; I < 3; ++I) { FPlanetShot Moving = Near; Moving.Name = FString::Printf(TEXT("earth-moving-%d"), I); Moving.bMoving = I > 0; Shots.Add(Moving); }
            // Whole disc, sun from the side so terminator, relief light and glint are all present.
            FPlanetShot Full; Full.Name = TEXT("earth-full"); Full.Clock = Near.Clock;
            const FVector3d Side = FVector3d::CrossProduct(ToSun, FVector3d::UpVector).GetSafeNormal();
            const FVector3d FromEarth = (ToSun * .55 + Side * .83 + FVector3d::UpVector * .1).GetSafeNormal();
            Full.Camera = Home.Position + FromEarth * Home.Radius * 3.4; Full.Look = -FromEarth; Shots.Add(Full);
            // Limb and atmosphere over the day side, low altitude.
            FPlanetShot Limb; Limb.Name = TEXT("earth-limb"); Limb.Clock = ClockForSun(Home, ToSun, 30, -40, .7);
            const FVector3d Site = SurfaceNormal(Home, 30, -40, Limb.Clock);
            FVector3d Along = FVector3d::CrossProduct(Site, FVector3d::CrossProduct(ToSun, Site)).GetSafeNormal();
            Limb.Camera = Home.Position + Site * (Home.Radius + 7.0e5) - Along * 1.5e6;
            Limb.Look = (Home.Position + Site * Home.Radius + Along * 2.2e6 - Limb.Camera).GetSafeNormal(); Shots.Add(Limb);
            // Dusk over Europe: terminator, city lights and cloud tops in the last light.
            FPlanetShot Dusk; Dusk.Name = TEXT("earth-terminator"); Dusk.Clock = ClockForSun(Home, ToSun, 48, 10, .02);
            Oblique(Dusk, Home, SurfaceNormal(Home, 48, 10, Dusk.Clock), 4.5e6); Shots.Add(Dusk);
            // Ocean glint: camera placed so the specular point lies in the open Atlantic.
            FPlanetShot Glint; Glint.Name = TEXT("earth-glint"); Glint.Clock = ClockForSun(Home, ToSun, 15, -35, .93);
            const FVector3d Sea = SurfaceNormal(Home, 15, -35, Glint.Clock);
            const FVector3d Mirror = (Sea * 2 * FVector3d::DotProduct(Sea, ToSun) - ToSun).GetSafeNormal();
            Glint.Camera = Home.Position + Sea * Home.Radius + Mirror * 6.0e6; Glint.Look = -Mirror; Shots.Add(Glint);
        }
        // Optional representative bodies by ID, using the standard inspection pose.
        FString Extra;
        if (FParse::Value(FCommandLine::Get(), TEXT("SolarPlanetBodies="), Extra, false))
        {
            TArray<FString> Ids; Extra.ParseIntoArray(Ids, TEXT(","));
            for (const FString& Id : Ids)
                for (int32 S = 0; S < Systems.Num(); ++S)
                    for (int32 B = 0; B < Systems[S].Bodies.Num(); ++B)
                        if (Systems[S].Bodies[B].Id == Id) { FPlanetShot Shot; Shot.Name = Id.Replace(TEXT("."), TEXT("-")); Shot.System = S; Shot.Body = B; Shots.Add(Shot); }
        }
        UE_LOG(LogTemp, Display, TEXT("PLANET_PROBE shots=%d"), Shots.Num());
    }
    const int32 Index = (ProbeFrame - 10) / Window, Local = (ProbeFrame - 10) % Window;
    if (ProbeFrame >= 10 && Shots.IsValidIndex(Index))
    {
        const FPlanetShot& Shot = Shots[Index];
        if (Local == 0)
        {
            ApplySharpRenderingSettings();
            SetShipVisible(Shot.bShip);
            for (const auto& Mesh : CloudMeshes) if (Mesh) Mesh->SetVisibility(Shot.bClouds);
            FPlanetShot Pose = Shot;
            if (Shot.Body != INDEX_NONE)
            {
                // Switch system through the game's own path, then a centred side-lit pose (~60 deg
                // phase, three radii) so shading, relief and the terminator are visible.
                Check(ActivateSystem(Shot.System, Shot.Body), TEXT("inspect ") + Shot.Name);
                const auto& Body = Systems[Shot.System].Bodies[Shot.Body];
                FVector3d ToSun = (SunPosition - Body.Position).GetSafeNormal();
                if (ToSun.IsNearlyZero()) ToSun = FVector3d(1, 0, 0);
                FVector3d Side = FVector3d::CrossProduct(ToSun, FVector3d::UpVector).GetSafeNormal();
                if (Side.IsNearlyZero()) Side = FVector3d(0, 1, 0);
                const FVector3d Dir = (ToSun * .5 + Side * .866 + FVector3d::UpVector * .15).GetSafeNormal();
                Pose.Camera = Body.Position + Dir * (Body.Radius * Body.Shape.GetMax() * 3.0 + 3000);
                Pose.Look = -Dir; Pose.Clock = Flight.GetState().SimulationSeconds;
            }
            if (!Shot.bMoving)
            {
                if (Shot.Body == INDEX_NONE && ActiveSystem != 0) ActivateSystem(0, 0);
                Daedalus::FFlightState Start; Start.PositionMetres = Pose.Camera; Start.SimulationSeconds = Pose.Clock;
                const FRotator Look = FVector(Pose.Look).Rotation();
                // The follow camera looks 18.3 degrees below the ship heading (SolarFlightGameMode.cpp).
                Start.YawDegrees = Look.Yaw; Start.PitchDegrees = FMath::Clamp(double(Look.Pitch) + 18.3, -59., 59.);
                FString Error; ProfileIndex = 1;
                Check(Flight.Initialize(Profiles[ProfileIndex].Config, Start, Bodies, Error), TEXT("place ") + Shot.Name + TEXT(" ") + Error);
                if (FMath::Abs(Look.Pitch + 18.3) > 59) UE_LOG(LogTemp, Warning, TEXT("PLANET_PROBE %s pitch clamped from %f"), *Shot.Name, Look.Pitch);
            }
            if (Pawn) { Pawn->CameraDistanceMetres = 1500; Pawn->ResetCamera(); }
            if (Shot.bMoving && Index > 0 && !Shots[Index - 1].bMoving && PC)
            {
                // Ordinary impulse: about 4 km per frame relative to the planet.
                PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R, IE_Pressed, 1));
                PC->InputKey(FInputKeyEventArgs::CreateSimulated(EKeys::R, IE_Released, 1));
            }
            GpuSum = FrameSum = GpuMemory = 0; Samples = 0;
        }
        // GPU time and process GPU memory from the engine stat unit data, measured after the
        // screenshot so the overlay never appears in a capture.
        if (Local == ShotAt + 4 && PC) PC->ConsoleCommand(TEXT("stat unit"));
        if (Local >= ShotAt + 14 && Local < ShotAt + 36 && GEngine->GameViewport)
            if (const FStatUnitData* Unit = GEngine->GameViewport->GetStatUnitData())
            {
                GpuSum += Unit->RawGPUFrameTime[0]; FrameSum += Unit->RawFrameTime; ++Samples;
                // Process GPU memory is only reported by some RHIs; implausible values mean unavailable.
                const double Memory = double(Unit->RawGPUMemoryUsage[0]);
                GpuMemory = Memory > 0 && Memory < 64. * 1073741824. ? FMath::Max(GpuMemory, Memory) : GpuMemory;
            }
        if (Local == ShotAt + 36)
        {
            if (PC) PC->ConsoleCommand(TEXT("stat none"));
            const FString Row = PendingRow.Replace(TEXT("@GPU@"), *FString::Printf(TEXT("\"gpuMs\":%.2f,\"frameMs\":%.2f,\"gpuMemoryMiB\":%.0f"),
                Samples ? GpuSum / Samples : 0., Samples ? FrameSum / Samples : 0., GpuMemory / 1048576.));
            Report += Row;
            UE_LOG(LogTemp, Display, TEXT("PLANET_SHOT %s"), *Row);
        }
        if (Local == ShotAt)
        {
            IFileManager::Get().MakeDirectory(*ProbeDirectory, true);
            FScreenshotRequest::RequestScreenshot(ProbeDirectory / (Shot.Name + TEXT(".png")), false, false);
            // Resident texture memory of everything loaded for the active system (all mips resident).
            double PlanetBytes = 0, AllBytes = 0; int32 PlanetCount = 0;
            for (TObjectIterator<UTexture2D> It; It; ++It)
            {
                const double Bytes = It->CalcTextureMemorySizeEnum(TMC_ResidentMips);
                AllBytes += Bytes;
                const FString Path = It->GetPathName();
                if (Path.StartsWith(TEXT("/Game/PlanetQuality/")) || Path.StartsWith(TEXT("/Game/Solar/Textures/"))) { PlanetBytes += Bytes; ++PlanetCount; }
                if (Index == 0 && Path.StartsWith(TEXT("/Game/PlanetQuality/")))
                    UE_LOG(LogTemp, Display, TEXT("PLANET_TEXTURE %s %dx%d %s mips=%d/%d %.1f MiB"), *Path, It->GetSizeX(), It->GetSizeY(),
                        GetPixelFormatString(It->GetPixelFormat()), It->GetNumResidentMips(), It->GetNumMips(), Bytes / 1048576.);
            }
            int32 Clouds = 0, Fine = 0;
            for (const auto& Mesh : CloudMeshes) Clouds += Mesh != nullptr;
            for (int32 Level : PlanetSphereLevels) Fine += Level > 0;
            const FIntPoint Size = GEngine->GameViewport->Viewport->GetSizeXY();
            PendingRow = FString::Printf(TEXT("%s{\"name\":\"%s\",\"width\":%d,\"height\":%d,@GPU@,\"planetTextureMiB\":%.1f,\"planetTextures\":%d,\"allTextureMiB\":%.1f,\"cloudShells\":%d,\"finerSpheres\":%d,\"system\":\"%s\"}"),
                Index ? TEXT(",") : TEXT(""), *Shot.Name, Size.X, Size.Y,
                PlanetBytes / 1048576., PlanetCount, AllBytes / 1048576., Clouds, Fine, *Systems[ActiveSystem].Id);
        }
    }
    if (ProbeFrame == 10 + Shots.Num() * Window + 30)
    {
        SetShipVisible(true);
        for (const auto& Shot : Shots)
            Check(IFileManager::Get().FileSize(*(ProbeDirectory / (Shot.Name + TEXT(".png")))) > 100, TEXT("render written ") + Shot.Name);
        Check(Flight.GetError().IsEmpty(), TEXT("no domain errors"));
        Report += FString::Printf(TEXT("],\"passed\":%s,\"frames\":%d}"), bProbePassed ? TEXT("true") : TEXT("false"), ProbeFrame);
        FFileHelper::SaveStringToFile(Report, *(ProbeDirectory / TEXT("result.json")));
        FPlatformMisc::RequestExitWithStatus(false, bProbePassed ? 0 : 1);
    }
}

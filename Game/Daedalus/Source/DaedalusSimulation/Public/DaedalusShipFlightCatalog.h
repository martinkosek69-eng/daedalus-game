#pragma once

#include "CoreMinimal.h"
#include "DaedalusFlightModel.h"

namespace Daedalus
{
/** Singleplayer flight-lab definition. Length is the working game scale;
 * flight profiles carry conservative collision radii in metres. */
struct DAEDALUSSIMULATION_API FShipFlightDefinition
{
    FString Id;
    FString Name;
    double LengthMetres = 0;
    TArray<FString> ProfileNames;
    TArray<FFlightConfig> Profiles;
};

/** Data-only catalog. No Actors, assets or persistent world identities. */
struct DAEDALUSSIMULATION_API FShipFlightCatalog
{
    TArray<FShipFlightDefinition> Ships;

    /** Parse and validate JSON v1 completely before replacing Ships.
     * Unrelated presentation/attribution fields are permitted. */
    bool LoadJson(const FString& Json, FString& Error);
};
}

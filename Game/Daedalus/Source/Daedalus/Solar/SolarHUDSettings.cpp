#include "Solar/SolarFlightGameMode.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
const TCHAR* Section = TEXT("Daedalus.SolarHUD");
}

bool ASolarFlightGameMode::LoadHUDSettings()
{
    FString Json;
    TSharedPtr<FJsonObject> Root;
    if (!FFileHelper::LoadFileToString(Json, *(FPaths::ProjectContentDir() / TEXT("Data/Solar/hud.json")))
        || !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid()) return false;
    double Version = 0;
    const TSharedPtr<FJsonObject>* Families = nullptr;
    if (!Root->TryGetNumberField(TEXT("version"), Version) || Version != 1
        || !Root->TryGetObjectField(TEXT("shipFamilies"), Families)) return false;
    TMap<FString, FString> Candidate;
    for (const auto& Pair : (*Families)->Values)
    {
        FString Family;
        if (Pair.Key.IsEmpty() || !Pair.Value->TryGetString(Family)
            || (Family != TEXT("earth") && Family != TEXT("ancient"))) return false;
        Candidate.Add(FString(Pair.Key), Family);
    }
    for (const auto& Definition : ShipCatalog.Ships) if (!Candidate.Contains(Definition.Id)) return false;
    ShipHUDFamilies = MoveTemp(Candidate);
    FString Preference;
    if (GConfig && GConfig->GetString(Section, TEXT("AncientStyle"), Preference, GGameUserSettingsIni))
    {
        if (Preference == TEXT("earth")) bAncientHUDPreferred = false;
        else if (Preference != TEXT("ancient")) UE_LOG(LogTemp, Warning, TEXT("Unknown HUD preference; using Ancient default."));
    }
    return true;
}

bool ASolarFlightGameMode::HasAncientInterface() const
{
    const auto* Family = ShipHUDFamilies.Find(ShipId);
    return Family && *Family == TEXT("ancient");
}

bool ASolarFlightGameMode::UsesAncientHUD() const
{
    return HasAncientInterface() && bAncientHUDPreferred;
}

bool ASolarFlightGameMode::SetAncientHUD(bool bAncient)
{
    if (!bReady || !bPaused || Galaxy.bOpen || !HasAncientInterface() || !GConfig) return false;
    bAncientHUDPreferred = bAncient;
    GConfig->SetString(Section, TEXT("AncientStyle"), bAncient ? TEXT("ancient") : TEXT("earth"), GGameUserSettingsIni);
    GConfig->Flush(false, GGameUserSettingsIni);
    return true;
}

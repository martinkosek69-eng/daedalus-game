#include "DaedalusShipFlightCatalog.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace Daedalus
{
namespace
{
bool ReadObject(const TSharedPtr<FJsonValue>& Value, TSharedPtr<FJsonObject>& Out)
{
    if (!Value.IsValid() || Value->Type != EJson::Object) return false;
    Out = Value->AsObject();
    return Out.IsValid();
}

bool ReadName(const FJsonObject& Object, const TCHAR* Field, FString& Out)
{
    const TSharedPtr<FJsonValue> Value = Object.TryGetField(Field);
    return Value.IsValid() && Value->Type == EJson::String && Value->TryGetString(Out)
        && !Out.TrimStartAndEnd().IsEmpty() && Out.Len() <= 1024;
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

bool ReadNumber(const FJsonObject& Object, const TCHAR* Field, double& Out)
{
    const TSharedPtr<FJsonValue> Value = Object.TryGetField(Field);
    return Value.IsValid() && Value->Type == EJson::Number && Value->TryGetNumber(Out) && FMath::IsFinite(Out);
}

bool ReadSharedConfig(const FJsonObject& Object, FFlightConfig& Out)
{
    return ReadNumber(Object, TEXT("radiusMetres"), Out.ShipRadiusMetres)
        && ReadNumber(Object, TEXT("turnRateDegrees"), Out.TurnRateDegrees)
        && ReadNumber(Object, TEXT("angularAccelerationDegrees"), Out.AngularAccelerationDegrees)
        && ReadNumber(Object, TEXT("pitchLimitDegrees"), Out.PitchLimitDegrees)
        && ReadNumber(Object, TEXT("bankDegrees"), Out.BankDegrees)
        && ReadNumber(Object, TEXT("lowSpeedTurnMultiplier"), Out.LowSpeedTurnMultiplier)
        && ReadNumber(Object, TEXT("highSpeedBankFraction"), Out.HighSpeedBankFraction);
}

bool ReadProfile(const FJsonObject& Object, FFlightConfig& Out)
{
    return ReadNumber(Object, TEXT("speed"), Out.MaxSpeed)
        && ReadNumber(Object, TEXT("acceleration"), Out.Acceleration)
        && ReadNumber(Object, TEXT("braking"), Out.Braking)
        && ReadNumber(Object, TEXT("coastDeceleration"), Out.CoastDeceleration)
        && ReadNumber(Object, TEXT("lateralAcceleration"), Out.LateralAcceleration);
}
}

bool FShipFlightCatalog::LoadJson(const FString& Json, FString& Error)
{
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid())
    {
        Error = TEXT("Ship flight catalog is not a valid JSON object.");
        return false;
    }
    double Version = 0;
    const TArray<TSharedPtr<FJsonValue>>* ShipValues = nullptr;
    if (!ReadNumber(*Root, TEXT("version"), Version) || Version != 1
        || !Root->TryGetArrayField(TEXT("ships"), ShipValues) || ShipValues->IsEmpty())
    {
        Error = TEXT("Ship flight catalog requires version 1 and a nonempty ships array.");
        return false;
    }

    TArray<FShipFlightDefinition> NewShips;
    TSet<FString> ShipIds;
    for (const TSharedPtr<FJsonValue>& Value : *ShipValues)
    {
        TSharedPtr<FJsonObject> Object;
        if (!ReadObject(Value, Object))
        {
            Error = TEXT("Each ship flight definition must be an object.");
            return false;
        }
        FShipFlightDefinition Ship;
        FFlightConfig SharedConfig;
        const TArray<TSharedPtr<FJsonValue>>* ProfileValues = nullptr;
        if (!ReadName(*Object, TEXT("id"), Ship.Id) || !ValidId(Ship.Id) || ShipIds.Contains(Ship.Id)
            || !ReadName(*Object, TEXT("name"), Ship.Name)
            || !ReadNumber(*Object, TEXT("lengthMetres"), Ship.LengthMetres) || Ship.LengthMetres <= 0
            || !ReadSharedConfig(*Object, SharedConfig)
            || SharedConfig.ShipRadiusMetres < Ship.LengthMetres / 2
            || !Object->TryGetArrayField(TEXT("profiles"), ProfileValues) || ProfileValues->Num() != 3)
        {
            Error = TEXT("Ship needs a unique stable ID, nonempty name, finite positive length, radius covering half its length, and three profiles.");
            return false;
        }
        TSet<FString> ProfileNames;
        for (const TSharedPtr<FJsonValue>& ProfileValue : *ProfileValues)
        {
            TSharedPtr<FJsonObject> ProfileObject;
            FString ProfileName;
            FFlightConfig Profile = SharedConfig;
            if (!ReadObject(ProfileValue, ProfileObject)
                || !ReadName(*ProfileObject, TEXT("name"), ProfileName)
                || ProfileNames.Contains(ProfileName) || !ReadProfile(*ProfileObject, Profile))
            {
                Error = FString::Printf(TEXT("Ship '%s' needs three named profiles with numeric flight settings."), *Ship.Id);
                return false;
            }
            FString ValidationError;
            if (!Profile.Validate(ValidationError))
            {
                Error = FString::Printf(TEXT("Ship '%s', profile '%s': %s"), *Ship.Id, *ProfileName, *ValidationError);
                return false;
            }
            ProfileNames.Add(ProfileName);
            Ship.ProfileNames.Add(MoveTemp(ProfileName));
            Ship.Profiles.Add(Profile);
        }
        ShipIds.Add(Ship.Id);
        NewShips.Add(MoveTemp(Ship));
    }

    Ships = MoveTemp(NewShips);
    Error.Reset();
    return true;
}
}

#include "Persistence/DaedalusSaveStore.h"
#include "HAL/FileManager.h"
#include "HAL/CriticalSection.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
struct FEnvelope { int64 Generation = 0; FString Snapshot; bool bValid = false; };
FString Digest(const FString& Text, int64 Generation)
{
    // Corruption detection, not authentication. Generation is part of the checksum.
    const FTCHARToUTF8 Bytes(*(FString::Printf(TEXT("1:%lld:"), Generation) + Text));
    uint8 Hash[FSHA1::DigestSize];
    FSHA1::HashBuffer(Bytes.Get(), Bytes.Length(), Hash);
    return BytesToHex(Hash, FSHA1::DigestSize);
}
FString Slot(const FString& Directory, int32 Index)
{
    return FPaths::Combine(Directory, FString::Printf(TEXT("foundation-%d.sav"), Index));
}
FString LockName(const FString& Directory)
{
    FString Normalized = FPaths::ConvertRelativePathToFull(Directory);
    FPaths::NormalizeDirectoryName(Normalized); FPaths::CollapseRelativeDirectories(Normalized);
    return TEXT("DaedalusSave_") + Digest(Normalized.ToLower(), 0);
}
FEnvelope Read(const FString& Path, const Daedalus::FSimulation& Current, bool* Incompatible = nullptr)
{
    FEnvelope Result;
    const int64 Size = IFileManager::Get().FileSize(*Path);
    if (Size <= 0 || Size > 64 * 1024 * 1024) return Result;
    FString Text;
    if (!FFileHelper::LoadFileToString(Text, *Path) || Text.Len() > 32 * 1024 * 1024) return Result;
    // Native UObject deserialization can assert on corrupt names before validation.
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Root) || !Root) return Result;
    FString Format, GenerationText, Hash; double Version = 0;
    if (!Root->TryGetStringField(TEXT("format"), Format) || Format != TEXT("daedalus-save") ||
        !Root->TryGetNumberField(TEXT("version"), Version)) return Result;
    if (Version != 1) { if (Incompatible) *Incompatible = true; return Result; }
    if (!Root->TryGetStringField(TEXT("generation"), GenerationText) || GenerationText.IsEmpty() || GenerationText.Len() > 19) return Result;
    // Decimal text preserves all int64 bits; JSON numbers do not.
    for (TCHAR C : GenerationText) if (C < TEXT('0') || C > TEXT('9')) return Result;
    if (!LexTryParseString(Result.Generation, *GenerationText) || Result.Generation <= 0 ||
        FString::Printf(TEXT("%lld"), Result.Generation) != GenerationText ||
        !Root->TryGetStringField(TEXT("snapshot"), Result.Snapshot) ||
        !Root->TryGetStringField(TEXT("checksum"), Hash) || Hash != Digest(Result.Snapshot, Result.Generation)) return Result;
    Daedalus::FSimulation Candidate = Current; FString Error;
    Result.bValid = Candidate.Restore(Result.Snapshot, Error);
    if (!Result.bValid && Incompatible) *Incompatible = true;
    return Result;
}
}

bool FDaedalusSaveStore::Save(const FString& Directory, const Daedalus::FSimulation& Simulation, FString& Error)
{
    Error.Reset();
    FSystemWideCriticalSection Lock(LockName(Directory));
    if (!Lock.IsValid()) { Error = TEXT("Another instance is using this save directory"); return false; }
    FString Json;
    if (!Simulation.Serialize(Json, Error)) return false;
    bool Incompatible = false;
    const FEnvelope A = Read(Slot(Directory, 0), Simulation, &Incompatible);
    const FEnvelope B = Read(Slot(Directory, 1), Simulation, &Incompatible);
    if (Incompatible) { Error = TEXT("Existing save needs migration; refusing to overwrite it"); return false; }
    const int64 Generation = FMath::Max(A.bValid ? A.Generation : 0, B.bValid ? B.Generation : 0);
    if (Generation == MAX_int64) { Error = TEXT("Save generation exhausted"); return false; }
    const int32 Target = !A.bValid ? 0 : !B.bValid ? 1 : A.Generation <= B.Generation ? 0 : 1;
    TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetStringField(TEXT("format"), TEXT("daedalus-save")); Root->SetNumberField(TEXT("version"), 1);
    Root->SetStringField(TEXT("generation"), FString::Printf(TEXT("%lld"), Generation + 1));
    Root->SetStringField(TEXT("snapshot"), Json); Root->SetStringField(TEXT("checksum"), Digest(Json, Generation + 1));
    FString Text;
    if (!FJsonSerializer::Serialize(Root, TJsonWriterFactory<>::Create(&Text))) { Error = TEXT("Save serialization failed"); return false; }
    if (!IFileManager::Get().MakeDirectory(*Directory, true)) { Error = TEXT("Cannot create save directory"); return false; }
    const FString Temp = FPaths::Combine(Directory, FGuid::NewGuid().ToString() + TEXT(".tmp"));
    if (!FFileHelper::SaveStringToFile(Text, *Temp, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM) || !Read(Temp, Simulation).bValid)
    {
        IFileManager::Get().Delete(*Temp);
        Error = TEXT("Save temporary write or verification failed"); return false;
    }
    const FString Destination = Slot(Directory, Target);
    if (!IFileManager::Get().Move(*Destination, *Temp, true, false, false, true))
    {
        IFileManager::Get().Delete(*Temp);
        Error = TEXT("Save replacement failed; previous generation retained"); return false;
    }
    return true;
}

bool FDaedalusSaveStore::Load(const FString& Directory, Daedalus::FSimulation& Simulation, FString& Error)
{
    Error.Reset();
    FSystemWideCriticalSection Lock(LockName(Directory));
    if (!Lock.IsValid()) { Error = TEXT("Another instance is using this save directory"); return false; }
    const FEnvelope A = Read(Slot(Directory, 0), Simulation), B = Read(Slot(Directory, 1), Simulation);
    const FEnvelope& Latest = !A.bValid ? B : !B.bValid || A.Generation >= B.Generation ? A : B;
    if (!Latest.bValid) { Error = TEXT("No compatible verified save generation"); return false; }
    return Simulation.Restore(Latest.Snapshot, Error);
}

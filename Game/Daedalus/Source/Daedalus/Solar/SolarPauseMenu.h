#pragma once
#include "CoreMinimal.h"

class ASolarFlightGameMode;
enum class ESolarPausePage : uint8 { Main, Ships, Settings, Saves };
enum class ESolarPauseAction : uint8 { Resume, Ships, Settings, Saves, Quit, Back, ChooseShip, None };
struct FSolarPauseButton
{
    FString Label, Hint;
    ESolarPauseAction Action = ESolarPauseAction::None;
    int32 ShipIndex = INDEX_NONE;
    bool bEnabled = true, bSelected = false;
    FBox2D Pixels;
};
struct FSolarPauseView
{
    float Scale = 1, X = 0, Y = 0, Width = 440, Height = 0;
    FString Title, Description;
    TArray<FSolarPauseButton> Buttons;
};
// Shared display/hit geometry; no state mutation through HUD drawing.
FSolarPauseView BuildSolarPauseView(const ASolarFlightGameMode& Lab, FVector2D Viewport);

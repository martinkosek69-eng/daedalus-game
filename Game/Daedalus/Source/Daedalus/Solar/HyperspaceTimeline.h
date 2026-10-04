#pragma once
#include "CoreMinimal.h"

// Authored presentation seconds/metres. Does not command a flight or travel model.
struct FHyperFrame
{
    double ShipX=0, Radius=0, Strength=0, Engine=0, Transit=0;
    bool bExteriorShip=true;
};
struct FHyperTimeline
{
    double Opening=0, Full=0, Acceleration=0, Nose=0, Tail=0;
    double Collapse=0, Closed=0, Transit=0, End=0;
    double Length=0, WindowX=0, WindowRadius=0;
    bool Load(const FString& Json);
    FHyperFrame Sample(double Seconds) const;
};

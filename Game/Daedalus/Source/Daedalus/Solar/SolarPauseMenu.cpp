#include "Solar/SolarPauseMenu.h"
#include "Solar/SolarFlightGameMode.h"
#include "GameFramework/PlayerController.h"

FSolarPauseView BuildSolarPauseView(const ASolarFlightGameMode& Lab, FVector2D Viewport)
{
    FSolarPauseView V;
    V.Scale = FMath::Max(.35, FMath::Min(Viewport.X / 1920., Viewport.Y / 1080.));
    auto Add = [&](FString Label, FString Hint, ESolarPauseAction Action, bool Enabled = true, int32 ShipIndex = INDEX_NONE, bool Selected = false) {
        FSolarPauseButton B; B.Label = MoveTemp(Label); B.Hint = MoveTemp(Hint); B.Action = Action;
        B.bEnabled = Enabled; B.ShipIndex = ShipIndex; B.bSelected = Selected; V.Buttons.Add(MoveTemp(B));
    };
    switch (Lab.PauseMenuPage)
    {
    case ESolarPausePage::Main:
        V.Title = TEXT("PALUBNÍ NABÍDKA"); V.Description = TEXT("LET JE POZASTAVEN");
        Add(TEXT("Pokračovat ve hře"), TEXT("P"), ESolarPauseAction::Resume);
        Add(TEXT("Přepnout loď"), Lab.ActiveShip == 1 ? TEXT("AURORA") : TEXT("DAEDALUS"), ESolarPauseAction::Ships);
        Add(TEXT("Nastavení"), TEXT("OBRAZ"), ESolarPauseAction::Settings);
        Add(TEXT("Uložit / načíst"), TEXT("POZDĚJI"), ESolarPauseAction::Saves);
        Add(TEXT("Vypnout hru"), TEXT("UKONČIT"), ESolarPauseAction::Quit);
        break;
    case ESolarPausePage::Ships:
        V.Title = TEXT("VÝBĚR LODI"); V.Description = TEXT("ZMĚNA ZACHOVÁ POLOHU A LET");
        for (int32 I = 0; I < Lab.AvailableShips().Num(); ++I)
        {
            const auto& S = Lab.AvailableShips()[I];
            Add(I == 1 ? TEXT("Aurora Class") : S.Name,
                FString::Printf(TEXT("%.0f m · %.0f km/s"), S.LengthMetres, S.Profiles[1].MaxSpeed / 1000),
                ESolarPauseAction::ChooseShip, I != Lab.ActiveShip, I, I == Lab.ActiveShip);
        }
        Add(TEXT("Zpět"), TEXT("NABÍDKA"), ESolarPauseAction::Back);
        break;
    case ESolarPausePage::Settings:
        V.Title = TEXT("NASTAVENÍ OBRAZU"); V.Description = TEXT("KVALITNÍ REŽIM · PLNÁ OSTROST");
        Add(TEXT("Vykreslování"), FString::Printf(TEXT("%.0f × %.0f"),Viewport.X,Viewport.Y), ESolarPauseAction::None, false);
        Add(TEXT("Rozlišení scény"), TEXT("100 % · NATIVNÍ"), ESolarPauseAction::None, false);
        Add(TEXT("Rozmazání pohybem"), TEXT("VYPNUTO"), ESolarPauseAction::None, false);
        Add(TEXT("Další volby"), TEXT("DOPLNÍME POZDĚJI"), ESolarPauseAction::None, false);
        Add(TEXT("Zpět"), TEXT("NABÍDKA"), ESolarPauseAction::Back);
        break;
    case ESolarPausePage::Saves:
        V.Title = TEXT("ULOŽENÍ A NAČTENÍ"); V.Description = TEXT("LETOVÁ ZKUŠEBNA");
        Add(TEXT("Uložit let"), TEXT("ZATÍM NEDOSTUPNÉ"), ESolarPauseAction::None, false);
        Add(TEXT("Načíst let"), TEXT("ZATÍM NEDOSTUPNÉ"), ESolarPauseAction::None, false);
        Add(TEXT("Zpět"), TEXT("NABÍDKA"), ESolarPauseAction::Back);
        break;
    }
    V.Height = 114 + V.Buttons.Num() * 52;
    V.X = (Viewport.X / V.Scale - V.Width) * .5;
    V.Y = (Viewport.Y / V.Scale - V.Height) * .5;
    for (int32 I = 0; I < V.Buttons.Num(); ++I)
    {
        const FVector2D A(V.X + 16, V.Y + 74 + I * 52);
        V.Buttons[I].Pixels = FBox2D(A * V.Scale, (A + FVector2D(V.Width - 32, 44)) * V.Scale);
    }
    return V;
}

void ASolarFlightGameMode::HandlePauseMenuClick(FVector2D Pixel, FVector2D Viewport)
{
    if (!bReady || !bPaused || Galaxy.bOpen) return;
    const auto View = BuildSolarPauseView(*this, Viewport);
    for (const auto& B : View.Buttons)
    {
        if (!B.bEnabled || !B.Pixels.IsInside(Pixel)) continue;
        switch (B.Action)
        {
        case ESolarPauseAction::Resume: TogglePause(); break;
        case ESolarPauseAction::Ships: PauseMenuPage = ESolarPausePage::Ships; break;
        case ESolarPauseAction::Settings: PauseMenuPage = ESolarPausePage::Settings; break;
        case ESolarPauseAction::Saves: PauseMenuPage = ESolarPausePage::Saves; break;
        case ESolarPauseAction::Back: PauseMenuPage = ESolarPausePage::Main; break;
        case ESolarPauseAction::ChooseShip: SelectShip(B.ShipIndex); break;
        case ESolarPauseAction::Quit:
            if (auto* PC = GetWorld()->GetFirstPlayerController()) PC->ConsoleCommand(TEXT("quit"));
            break;
        default: break;
        }
        return;
    }
}

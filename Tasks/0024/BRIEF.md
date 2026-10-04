# 0024 — Nový zelený HUD: nejprve návrh vzhledu

- Vlastník: Codex koordinátor. Základ `91e0e0111de4bc5715b3d847504ae7eabbfae38e`.
- Pracovní/cílová větev: codex/solar-flight, vlastní přidělená kopie na A:.
- Uživatel požádal o paralelní práci na HUD při Claudeově úkolu0023.
- Uživatel vybral **nový HUD**, dodal dvě reference STO a požádal o návrh
  postupu před implementací. Zvolen samostatný náhled nad snímkem hry;
  napojení do hry až po jeho hodnocení. Viz Docs/HUD.md.
- Světle zelený terminál, minimapa vlevo nahoře, počítač vpravo nahoře,
  Daedalus se čtyřmi štíty dole uprostřed, energie vlevo, tah/rychlost vpravo.
  Bez online prvků/chat/misijního seznamu/Endeavors. Štíty/energie připravené.

Přečíst společné pokyny a Docs/VISUAL_QUALITY.md. Pure singleplayer,
prezentace čte stav, nic v letu nemění. Zachovat funkce a klávesy, oddělit
obrazovkové údaje od pravidel letu a planetární prezentace.

## Přidělené soubory

**Aktuální fáze je pouze návrh:** Tasks/0024/Preview/**, Tools/Prepare-HudOutline.py,
Docs/HUD.md a vlastní progress/handoff; koordinační INDEX/CURRENT_STATE.
Níže uvedené herní cesty jsou plánovaná následná fáze, teď je neměnit.

- Game/Daedalus/Source/Daedalus/Solar/SolarFlightGameMode.cpp: výhradně
  existující DrawHUD a nutné include při jeho oddělení; žádná letová logika.
- Nový Game/Daedalus/Source/Daedalus/Solar/SolarHUD.cpp.
- Docs/HUD.md a vlastní Tasks/0024/PROGRESS.md, HANDOFF.md.
- Koordinátorské aktualizace Tasks/INDEX a Docs/CURRENT_STATE.

Neměnit SolarFlightGameMode.h, SolarSystem.cpp, SolarRendering.cpp,
SolarSharpProbe.cpp, žádné planetární/ship binary assety, data a konfiguraci.
Tyto soubory mohou být přidělené Claudeovi. Galaxijní mapu nyní zachovat.
Případnou úpravu mapového rozhraní nejdřív vymezit zvlášť.

## Kontroly a hranice

- Písmo rasterizované v cílových pixelech, čitelné české znaky, žádný
  zvětšený obrázek HUD. Kompaktní rozměry ve4K, bez překryvu panelů.
- Zachovat rychlost, tah/režim, navigaci, vzdálenost/cíl, pauzu, varování
  a nápovědu. Dlouhé názvy omezit podle prostoru, ne dovolit přetéct.
- Jediná implementace DrawHUD; čtení stavu bez jeho mutace.
- Dokud Claude drží editor/build pro0023, žádný Unreal/build/package z Codexu.
  Připravovat pouze vlastní zdroje a běžné statické kontroly. Po uvolnění
  potřebný build a cílený herní/4K test, většinu vizuálního hodnocení uživatel.
- Zdroje označit jako neověřené kompilací, pokud společné nástroje ještě
  nejsou uvolněné. Nepřepsat dosavadní balíček hry.
- Commit/push koordinační větve, výsledek/checks/omezení do HANDOFF.

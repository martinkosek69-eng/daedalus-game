# 0024 — Nový přístrojový HUD podle STO: nejprve návrh vzhledu

- Vlastník: Codex koordinátor. Základ `91e0e0111de4bc5715b3d847504ae7eabbfae38e`.
- Pracovní/cílová větev: codex/solar-flight, vlastní přidělená kopie na A:.
- Uživatel požádal o paralelní práci na HUD při Claudeově úkolu0023.
- Uživatel vybral **nový HUD**, dodal dvě reference STO a požádal o návrh
  postupu před implementací. Zvolen samostatný náhled nad snímkem hry;
  napojení do hry až po jeho hodnocení. Viz Docs/HUD.md.
- Světle zelený terminál, minimapa vlevo nahoře, počítač vpravo nahoře,
  Daedalus se čtyřmi štíty dole uprostřed, energie vlevo, tah/rychlost vpravo.
  Bez online prvků/chat/misijního seznamu/Endeavors. Štíty/energie připravené.

**Aktuální upřesnění:** jednobarevný zelený návrh byl odmítnut. Minimapa má
být úplně bez obdélníku/panelu; světlá zelená platí pro její linky. Další funkce
odlišují tlumené modré/jantarové/měděné a neutrální text. Energii upozadit,
klávesovou nápovědu/ovládání odstranit. Přidat tři skupiny podle skutečných
modelových bodů: dělové věže, raketová sila, paprskové emitory.
Ukázkový výběr zbraně může zvýraznit body, nezapíná střelbu.

**Nejnovější upřesnění:** uživatel odmítl příliš minimalistickou revizi.
Minimapu zachovat beze změny; obrys lodi zelený jako minimapa, čtyři štíty
modré. Ostatní funkční barvy zachovat. Přístroje mají být výrazně komplexnější
a bližší STO: členité rámy, podrobné displeje zbraní, pohon a sekundární energie.
Nejde o přidání online prvků, bojové logiky nebo smyšlených stavových hodnot.

Přečíst společné pokyny a Docs/VISUAL_QUALITY.md. Pure singleplayer,
prezentace čte stav, nic v letu nemění. Zachovat funkce a klávesy, oddělit
obrazovkové údaje od pravidel letu a planetární prezentace.

## Přidělené soubory

**Aktuální fáze: implementace schváleného vzhledu.** Uživatel schválil návrh,
požádal sjednotit velikosti a zasadit HUD do hry. Jediná nově funkční část
je minimapa; zbraňové/energetické/počítačové akce zůstávají nezapojené.
Dosavadní rychlost/tah/režim a varování se zachovají jako čtené údaje.
Tasks/0024/Preview/**, Tools/Prepare-HudOutline.py, Docs/HUD.md a vlastní
progress/handoff; koordinační INDEX/CURRENT_STATE patří koordinátorovi.

- Game/Daedalus/Source/Daedalus/Solar/SolarFlightGameMode.cpp: výhradně
  existující DrawHUD a nutné include při jeho oddělení; žádná letová logika.
- Nové Game/Daedalus/Source/Daedalus/Solar/SolarHUD*.h/.cpp/.inl:
  vlastní vykreslení a read-only radarová projekce; žádná změna letu.
- Game/Daedalus/Source/Daedalus/Tests/SolarHUDTests.cpp: radarové hranice.
- Docs/HUD.md a vlastní Tasks/0024/PROGRESS.md, HANDOFF.md.
- Koordinátorské aktualizace Tasks/INDEX a Docs/CURRENT_STATE.

Neměnit SolarFlightGameMode.h, SolarSystem.cpp, SolarRendering.cpp,
SolarSharpProbe.cpp, žádné planetární/ship binary assety, data a konfiguraci.
Tyto soubory mohou být přidělené Claudeovi. Galaxijní mapu nyní zachovat.
Případnou úpravu mapového rozhraní nejdřív vymezit zvlášť.

## Kontroly a hranice

- Písmo rasterizované v cílových pixelech, čitelné české znaky, žádný
  zvětšený obrázek HUD. Kompaktní rozměry ve4K, bez překryvu panelů.
- Zachovat rychlost, tah/režim, navigaci, vzdálenost/cíl, pauzu a varování;
  klávesovou nápovědu podle pozdějšího upřesnění nezobrazovat.
  Dlouhé názvy omezit podle prostoru, ne dovolit přetéct.
- Jediná implementace DrawHUD; čtení stavu bez jeho mutace.
- Dokud Claude drží editor/build pro0023, žádný Unreal/build/package z Codexu.
  Připravovat pouze vlastní zdroje a běžné statické kontroly. Po uvolnění
  potřebný build a cílený herní/4K test, většinu vizuálního hodnocení uživatel.
- Zdroje označit jako neověřené kompilací, pokud společné nástroje ještě
  nejsou uvolněné. Nepřepsat dosavadní balíček hry.
- Commit/push koordinační větve, výsledek/checks/omezení do HANDOFF.

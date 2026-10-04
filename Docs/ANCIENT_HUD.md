# Antické palubní rozhraní — 0027

## Použití a hranice

Na Auroře otevři P → Nastavení a vyber „HUD: antická technika“ nebo
„HUD: pozemská technika“. P vrátí do letu. Nový profil používá pro antické lodě
antický vzhled; pozemská varianta zachovává předchozí HUD. Volba antické rodiny
se ukládá do sekce Daedalus.SolarHUD v GameUserSettings.ini ve složce uživatelských
dat na A:, odděleně od uložené hry. Přepnutí přes Daedalus ji nemění.

Daedalus má vždy pozemský HUD. Při přidání dalšího ověřeného modelu se jeho
stabilní ID přiřadí rodině earth/ancient v Content/Data/Solar/hud.json. Není nutné
kopírovat celý HUD pro další antickou loď. Samotné přiřazení nevytváří model ani
herní schopnosti nové lodě. Současný modelový adaptér obsahuje dvě známé lodě.

Jde pouze o vzhled a preferenci rozhraní. Letové profily, poloha, rychlost,
navigační cíl, čas a fyzika se nemění. Štíty, výzbroj, energetické kanály a nové
akce počítače zůstávají nezapojené; jejich hodnoty jsou pomlčky. Ornamenty nejsou
naměřená telemetrie. Skutečné rychlosti, tah, režim, radar a navigace čtou stejný
kanonický stav jako původní HUD. Galaxijní mapa zachovává data a ovládání.

## Vizuální reference a rozhodnutí

Prohlédnuté původní seriálové obrazovky v galerii TSACS:

- [Jumper: holografická navigace](https://www.tsacs.com/gallery/puddle_jumper_hud/60/)
  a [detail planetárního displeje](https://www.tsacs.com/uploads/images/Gallery/puddle_jumper_hud/puddle_jumper_hud_planet_display_1.png):
  dvojité modré obrysy, zkosené/notchované tvary, průhledné vrstvy, zelené schéma
  planety, samostatná pole a drobné datové pásy.
- [Orion: konzole antické válečné lodi](https://www.tsacs.com/gallery/orion/60/)
  a [detail pásů/kruhu](https://www.tsacs.com/uploads/images/Gallery/orion/orion_panels_4.png):
  oranžové/jantarové a červené plochy, koncentrické kruhy, lomené propojení panelů
  a blokové geometrické znaky. Orion je relevantnější než pozemské monitory
  expedice pro vzhled palubní konzole Aurory.
- [Antická databáze](https://www.tsacs.com/gallery/ancients_database/60/)
  a [detail](https://www.tsacs.com/uploads/images/Gallery/ancients_database/ancient_database_c.png):
  modrý dvojitý rám a členěná pole s jemným světlým písmem.
- [Jumper: další ovládací obrazovky](https://www.tsacs.com/gallery/puddle_jumper_interface/60/):
  modré diagnostické rámy, světelné řádky, samostatné barevné stavové oblasti.

Doplňkové dohledané reference: [Atlantis OS](https://stargatewiki.de/wiki/Atlantis-Betriebssystem),
[lantijská databáze](https://www.stargate-fusion.com/fiches/474-base-de-donnees-lantienne.html),
[antické konzole](https://www.rdanderson.com/stargate/lexicon/entries/ancientconsole.htm).
Nevydáváme lidské notebooky expedice ani fanouškovský simulátor za jednotný
kanonický antický design. Seriál používá více podob; nový HUD je jejich vlastní
herní interpretace, přizpůsobená čitelnosti a současným údajům hry.

Paleta kombinuje tmavé modré sklo, bledě modré hrany, tyrkysové schémata,
pískově jantarové ukazatele a cihlové/měděné pásy. Tvarově má dvojité zkosené
rámy, krystalové kanály, osmiúhelníkový přístroj lodi, čtyři modré sektory štítu,
šikmé segmenty pohonu a odlišné symboly dronů/pulzů. Počítač a nabídka mají
stejný vizuální jazyk. Mapa má odpovídající paletu, rámy a nadpis databáze.
Radar zůstává bez velkého podkladového obdélníku.

Vše je vlastní vektorová kresba a písmo vykreslené v cílových pixelech, včetně
originálních dekorativních znaků. Ty nejsou překlad antického jazyka; důležité
popisky zůstávají česky. Nebyly převzaty seriálové obrázky, fonty ani programy.
Rozložení využívá společnou stupnici 1920×1080; v nativním 3840×2160 se geometrie
a písmo počítají znovu. Největší spodní sestava má 1876×428 px, se zachovaným
volným středem obrazu. Neobsahuje rozmazané bitmapy ani záře zhoršující ostrost.

## Implementace a ověření

SolarHUDCanvas.h sdílí původní ostré kreslicí primitivy; SolarHUD.cpp ponechává
pozemské přístroje a společný kanonický radar. SolarAncientHUD.cpp kreslí nový
vzhled. SolarHUDSettings.cpp načítá mapování a mění pouze místní preferenci přes
explicitní příkaz z nabídky; DrawHUD nic neukládá ani nemění letový stav.

Invoke-AncientHUDProbe.ps1 používá oddělené uživatelské testovací složky na A:.
Dva procesy ověřují 4K, skutečné klikání na volby, zachování pauzy/letu,
oddělení Daedala, zachování volby při změně lodě a po úplném restartu.
Kontroly a stav dokončení jsou v Tasks/0027/HANDOFF.md a EVIDENCE.json.
Většinu osobního hodnocení stylu a létání provádí uživatel.

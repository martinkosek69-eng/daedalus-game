# První letová scéna

Pracovní větev: `codex/solar-flight`, úkol 0004. Hra zůstává čistě singleplayer.
Tato scéna je první hratelná zkouška skutečného Daedala, Země, Slunce a řízení.
Předchozí mapa Foundation zůstává samostatnou technickou zkouškou.

## Spuštění

Na tomto PC po sestavení dvojklik na `Tools/SPUSTIT_LET_DAEDALA.cmd`.
Nic se neinstaluje: spouštěč otevře místní Windows verzi pod `.local/solar/Build-WebReference`.
Po kliknutí do herního okna:

| Klávesa | Účinek |
| --- | --- |
| W / S | Příď dolů / nahoru (jako ve webové předloze) |
| A / D | Zatáčení vlevo / vpravo |
| E / Q | Zvýšit / snížit tah o 20 procentních bodů; drží se po uvolnění |
| R | Přepnout mezi nulovým a plným tahem |
| Držet mezerník nebo X | Brzdit ve všech směrech až do zastavení |
| Pravé tlačítko + myš | Volná kamera kolem lodě; Home obnoví sledování lodi |
| Kolečko / Home | Přiblížení kamery / návrat za loď |
| 1 / 2 | Ve stoje a s nulovým tahem přepnout přístavní chod / impuls |
| Backspace | Obnovit počáteční polohu u Země |
| P / Esc | Pozastavit / zavřít |

Záporný tah dovoluje pomalé couvání, nejvýše čtvrtinovou rychlostí.
Nulový tah sám postupně zpomaluje; brzda zastavuje rychleji. R nevynucuje
okamžité zastavení. Loď se v zatáčce mírně naklání a po uvolnění dorovná.
Kamera udržuje rovný horizont.

## Co ladíme

`Game/Daedalus/Content/Data/Solar/flight.json` je jediný zdroj parametrů této
scény. Změna dat vyžaduje nový Package, aby ji samostatná hra načetla.
Režimy nyní přebírají rychlosti webové předlohy: přístavní chod 150 m/s,
impuls 250 km/s. Start je v impulsu, takže 20 % tahu odpovídá cílovým 50 km/s.
Zrychlení jsou 25 m/s² a 18 km/s², brzdění 45 m/s² a 45 km/s².
Zatáčení až 9°/s, sklon ±60°, náklon 22°. Pomoc proti bočnímu skluzu
zůstává v existujícím samostatném letovém modelu; není to přesná kopie
všech vnitřních vzorců webu ani uzavřeného STO.
W snižuje příď a S ji zvedá, podle webu. Kamera má FOV 52°, výchozí
odstup 1500 m a citlivost přibližně .344° na jednotku myši. Ruční orbit
reaguje přímo a přestane sledovat zatáčení lodi, dokud nestisknete Home.
Sledování kurzu používá plynulé dorovnání, bez náklonu horizontu.
Motion blur je vypnutý a solar scéna používá FXAA bez historie snímků,
aby rychlé otáčení kamery nerozmazávalo trup. FXAA může mít více jemných
zubatých hran než časové vyhlazení. HUD je kompaktní dole podle webu.

Při zkoušení hodnotíme čitelnost, pohodlí řízení, setrvačnost a měřítka. Jde o
asistovaný herní let bez gravitace. Nejsou zde oběžné dráhy, hyperpohon,
boje, cestování mezi soustavami ani přistávání. Povrchová ochrana zastaví
konzervativní kouli kolem lodě před Zemí nebo Sluncem. Po otočení lze odletět.
Scéna zatím neukládá letovou polohu: další spuštění začíná znovu u Země.
Trvalé uložení původního základu funguje dál ve své původní scéně.

## Zdrojové modely a zobrazení

Aktuálně použitý model je Art/Ships/Daedalus/WebReference/WebDaedalus.glb:
nový převod nezměněného původního webového trupu, včetně původních runtime
detailů. Jemné panely a spáry se znovu počítají při vykreslování povrchu.
Claudeova fotografická varianta zůstává uložená samostatně pro porovnání.
Slunce má mírnější hlavní světlo a chladné jemné pomocné světlo.


- `Art/Ships/Daedalus`: skutečný původní GLB, původový záznam, upravitelný
  Blender soubor a export 600 m, +X vpřed, +Z nahoru. 600 m je projektová volba.
  Claudeův úkol 0008 je převzatý: nové barvy, samostatná světla, šest motorových
  efektů a změřené body zbraní/hangárů. Zachovává původní trup a proporce;
  odstranil dodatečné tmavé čtverce a lišty základního převodu. Barvy zůstávají
  prvním výtvarným přiblížením předloze, které můžeme dál ladit.
- `Art/Space`: vlastní hladká koule, původní textury Slunce/Země s původovým
  záznamem; hvězdy používají stejný HYG podvýběr jako webová předloha.
- `Content/Ships/Daedalus`, `Content/Solar`: importované Unreal assety přes LFS.
  `Tools/Prepare-SolarContent.py` zkontroluje rozměry a přiřadí materiály.
  Základní import nyní podporuje vertex color a konstantní PBR materiály.
  Motorová záře mění jas podle tahu. Zdrojové body jsou v Blenderu +Y vlevo;
  import je převádí na Unreal -Y. Viz CONTENT_WORKFLOW. Pohyblivé věže a skutečná
  střelba dosud nejsou implementované. Dodá-li někdo UV textury či pohyblivé díly,
  nejprve rozšířit
  a ověřit import; neslučovat je slepě do trupu.

Vzdálená Země/Slunce se vykreslují blíž ke kameře se stejně zmenšeným poloměrem,
takže jejich úhlová velikost zůstává zachována. Fyzika a vzdálenosti přitom
používají skutečné metry (Země 6371 km, Slunce přibližně 1 AU od Země).
Tato projekce řeší kreslení obrovských vzdáleností; není zmenšením fyzikálního
světa. Loď zůstává ve středu místní scény. Hvězdy se pohybují s kamerou,
řídké blízké částečky pomáhají poznat pohyb. Jemné pomocné světlo udržuje
trup čitelný. Povrchy mají denní/noční mapu, oblaka a jednoduchý atmosférický lem;
nejsou simulací počasí nebo budoucím planetárním terénem.

## Technické oddělení a reprodukce

`DaedalusSimulation/DaedalusFlightModel` vlastní letový stav jako obyčejné
hodnoty, bez Actorů. Používá double metry a krok 1/120 s. Limit 600 kroků
na Advance zachová zbytek času. Pauza nepřidává čas. Neplatné příkazy/stav
jsou odmítnuty. `Daedalus/Solar` posílá vstupní příkazy, čte stav a staví obraz.
HUD ani model neurčují fyzikální polohu. Tento samostatný ladicí model ještě
nenahrazuje uložený stav `UDaedalusWorldSubsystem`; jeho převod do společného
herního letu/persistence bude explicitní další změna, ne druhá trvalá pravda.

Z kořene pracovní kopie s lokálním toolchain nastavením:

```powershell
./Tools/Invoke-SolarFlight.ps1 Build
./Tools/Invoke-SolarFlight.ps1 Assets
./Tools/Invoke-SolarFlight.ps1 Test
./Tools/Invoke-SolarFlight.ps1 Package
./Tools/Invoke-SolarFlight.ps1 Smoke
./Tools/Invoke-SolarFlight.ps1 Visual
./Tools/Invoke-SolarFlight.ps1 Play
```

Jeden editor/build současně. Assets nepouští Blender a nezmění existující mapu.
Nezměněná receptura a zdroje mají SHA256 cache. Výstupy, sestavení a uživatelská
data jsou lokální pod `.local/solar` na A:, ne veřejná součást GitHubu.
Visual spouští vlastní izolovaný proces, posílá klávesy přes PlayerController,
kontroluje odezvu a uloží skutečné screenshoty. Jeho řízený čas není měření FPS.
Výsledky aktuální kontroly jsou v [předání 0004](../Tasks/0004/HANDOFF.md).
Zdrojové předání lze nezávisle zkontrolovat přes
`node Tools/Validate-DaedalusDelivery.mjs`; nevyžaduje Blender ani Unreal.

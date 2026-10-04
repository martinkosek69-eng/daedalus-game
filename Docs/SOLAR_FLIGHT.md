# Letová zkouška Sluneční soustavy

Singleplayer, větev codex/solar-flight, úkol0021 (navazuje na0020). Spustit dvojklikem
Tools/SPUSTIT_LET_DAEDALA.cmd po sestavení. Nic se neinstaluje.
Nový balíček .local/solar/Build-SolarSystem používá nativní rozlišení monitoru
v okně přes celou obrazovku. Alt+Enter/F11 mění zobrazení. Data jsou na A:.

| Ovládání | Účinek |
| --- | --- |
| W/S, A/D | Příď dolů/nahoru, zatáčení |
| E/Q | Tah ±20 %, drží nastavenou hodnotu |
| R | Nulový/plný tah; rychlost se mění postupně |
| Shift+R | Zapnout/vypnout plný podsvětelný impuls250000km/s; zapnutí nastaví plný tah |
| Mezerník/X | Brzda až do zastavení |
| Pravé tlačítko + myš | Volný orbit; Home obnoví sledování lodi |
| Kolečko | Odstup kamery650–6000m |
| 1/2 | Přístavní150m/s / impuls250km/s; přepínání i za letu |
| PgUp/PgDn | Vybrat těleso soustavy |
| F | Výslovně testovací přesun k vybranému tělesu |
| Backspace | Začátek u Země |
| M | 3D galaxie, databáze těles a výpočty navigace |
| P/Esc | Pauza/konec; Esc v mapě vrací do letu |

Všechny tři letové profily dosáhnou maxima za2,5s. Pohyb se vždy srovnává s přídí,
včetně couvání a brzdění: žádný boční drift ani umělá ztráta rychlosti v zatáčce.
Reakce na řízení je zvýšena20% (úhlové zrychlení28,8°/s²).
Zatáčení se řídí skutečnou rychlostí: při nízké rychlosti až15,66°/s a náklon36°,
při plné rychlosti10,8°/s a náklon21,6°. Přechod se plynule mění; naklonění je
vzhledový projev zatáčení. Hranice sklonu±60° dosedá postupně. Asistovaný herní let nemá
Newtonovu gravitaci. Kamera FOV52°, přímý volný orbit a plynulé sledování kurzu.
Plný impuls odpovídá rychlosti webové předlohy (asi0,834rychlosti světla);
není hyperpohon. Přepnutí dolů zachová rychlost a polohu a plynule zpomaluje,
se zachovaným silnějším brzděním do dosažení nižšího limitu. R/Mezerník/X ruší
plný impuls. Ve stojící lodi R spouští právě vybraný běžný režim.

Windows high-DPI je zapnuté ještě před vytvořením okna, takže zvětšení rozhraní
Windows nesnižuje fyzické rozlišení obrazu. Motion blur, časové vyhlazování,
FXAA, hloubka ostrosti a barevné rozmazání jsou vypnuté.100% skutečné pixely,
16×anisotropie a písmo vykreslené ve skutečných pixelech pro ostrý český HUD.
Výchozí priorita je plná kvalita: načítání zmenšených textur je vypnuté, používá
se nejpodrobnější geometrie. Dřívější selektivní politika zůstává dostupná při
výslovném zapnutí streamování později. Vyšší spotřebu paměti a výkon posoudíme
podle uživatelského testu. Velmi tenké hrany mohou bez plošného vyhlazení kmitat;
skutečné detaily menší než jeden pixel nemůže rozlišení monitoru zobrazit.
Výkon4K je třeba osobně ladit;
automatický obrazový test nepředstavuje měření běžné snímkové frekvence.

Canonical flight.json obsahuje řízení, system.json oddělený svět v double metrech.
Rozšířený Sol obsahuje506 katalogových položek včetně459 planetárních měsíců,
29 vybraných planetek/komet a9 měsíců malých těles. Z nich104 má známý rozměr
i polohu pro fyzické zobrazení; neznámé rozměry/polohy se nevymýšlejí.
Dalších pět fiktivních soustav přidává63 těles. Prstence mají všichni čtyři
obří plynní/ledoví obři. Řídké vlastní vzorky asteroidů a vzdálených populací
nevyjadřují skutečnou hustotu. Místní vzdálenosti a rozměry jsou v reálných metrech;
rozložení je pevné reprezentativní období, nikoliv aktuální astronomické efemeridy.
Povrchy rotují podle délky dne. Zdroje a schematické výjimky uvádí
SOLAR_REALISM.md; ovládání mapy a nejistoty GALAXY_NAVIGATION.md.

Vzdálené objekty se vizuálně promítají blíž při zachování úhlové velikosti;
reálná poloha a konzervativní ochrana před povrchem zůstávají ve výpočtu letu.
Žádný vizuál nepřepisuje stav letu. F je dočasná pomůcka pro testování, ne hyperpohon.
Lokální modré částice dávají čitelnou informaci o pohybu; jejich zobrazovaná
rychlost je omezená, takže nepředstavují fyzické objekty ani měření prachu.
Obloha je černá a obsahuje samostatně generované bodové hvězdy z8920HYG záznamů,
se stejnou křivkou jasnosti jako webová předloha. Žádná fotografická vrstva,
fotografické hvězdy, mlhoviny ani bloomové záře se ve scéně nepoužívají.
Zdrojová fotografie zůstává zachovaná pouze jako dřívější reference.

Aktivní je nová upravená Claudeova loď z0008 (3effaca): původní600m trup,
UV plátování BaseColor/ORM/Normal, barvy vrcholů,38věží,62montážních bodů,
světla a6motorových efektů. Věže jsou nyní vizuální, palbu teprve přidáme.
WebReference zůstává zachovaná jako výtvarná předloha. Zdrojový původ/licence
v Art/Ships/Daedalus; fan model CC BY-NC4.0 není komerční povolení.

Reprodukovat: Invoke-SolarFlight.ps1 Build,Assets,Test,Package,Visual,Sharp,SharpNative,Smoke.
Sharp porovnává blízký/vzdálený model a rychlý orbit ve4K a zkouší klávesy pohonu.
SharpNative používá stejnou cestu nativního spuštění jako Play a kopii současného
uživatelského nastavení, bez přepisování původních hráčových dat.
Visual skutečně vykresluje3840×2160 a zkouší vstupy, let, všechny37návštěvy,
prstence, motorová světla a vypnuté rozmazání. Podrobnosti výstupů v úkolu0010.
Foundation zůstává oddělený ověřený základ; solární laboratoř zatím neukládá
letovou pozici. Hyperprostor zůstává samostatným budoucím úkolem.


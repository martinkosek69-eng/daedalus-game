# Galaxie a lodní databáze

Singleplayer letová laboratoř. M otevře nebo zavře mapu a během prohlížení
pozastaví let. Esc se z mapy vrátí do letu. Pravé tlačítko myši otáčí pohled,
prostřední posouvá, kolečko přibližuje. Shora / Z boku nastaví pohled na disk;
Home vrátí celou galaxii. Kolečko nad databází posouvá seznam.

Vyber soustavu vlevo; „Přiblížit vybranou soustavu“ ukáže její rozložení.
Vyhledávání přijímá jméno bez diakritiky; filtry oddělují planety a měsíce.
Vyber těleso a „Detail vybraného tělesa“ přiblíží jeho povrch. Vzdálenosti
v mapě používají skutečné katalogové metry, takže malé měsíce nemusí být vidět
vedle celého systému. Viditelné značky pomáhají výběru, nemění fyzické velikosti.

Navigační cíl pouze uloží adresu tělesa. Loď se výběrem nepřemístí. Výpočty
uvádějí vzdálenost mezi středy, dobu při současné rychlosti a plném impulsu,
a potřebnou rychlost pro jednu hodinu, den nebo týden. Při nulové rychlosti
je současná doba nedostupná. Jde o geometrický odhad při konstantní rychlosti;
nezapočítává zrychlení, trasu kolem těles ani budoucí hyperpohon.

„TEST: přesunout loď k cíli“ je výslovně testovací příkaz. Používá ověřený
stav letu a bezpečné místo před tělesem. Cestování hyperprostorem ani jeho
vzhled v této etapě nejsou implementované. Energie a boj rovněž nejsou součástí.

## Údaje a nejistoty

Sol má 506 katalogových záznamů: Slunce, osm planet, 459 planetárních měsíců,
29 vybraných malých těles a dalších devět měsíců malých těles. Nejde o každý
jednotlivý známý asteroid či kometu. Zobrazené malé populace jsou řídké vlastní
vzorky hlavního pásu, Trojanů, Kuiperova pásu a odhadované Oortovy oblasti.
Nejsou naměřenou hustotou ani neprůhlednou stěnou. Zdroje a epochy jsou v
[SOLAR_REALISM.md](SOLAR_REALISM.md) a Art/Space/SolarCatalog/SOURCE.md.

104 těles má použitelný rozměr i polohu a lze zobrazit jejich fyzickou geometrii.
Neznámý poloměr znamená značku bez fyzického povrchu či kolize. Devět položek
s neznámou polohou zůstává v databázi, ale nelze k nim navigovat nebo přesunout
loď. Povrchy bez dostupných snímků jsou označené jako schematické. Pluto a
Charon mají skutečné částečné mapy; černé nepozorované části nejsou geologie.
Čtyři prstencové atlasy uchovávají31 zdrojových částí, zobrazují30; neznámý
rozměr Uranova prstence Zeta není vymyšlený.

Galaxie je vlastní deterministická prostorová ilustrace s34100 vzorky,
diskem, rameny, příčkou, výdutí, prachem a řídkým halem. Vychází z popisu
[ESA/Gaia](https://www.esa.int/Science_Exploration/Space_Science/Gaia/Guide_to_our_galaxy).
Není přesnou mapou jednotlivých pozorovaných hvězd. Morfologie je autorská;
ESA obrázek ani dataset se nekopírují. Pět dalších soustav je výslovně fiktivních.
Dokud nejsou převzaté zdroje, zobrazují „Čeká na podklady“ a blokují přesun.

## Reprodukce a hranice

Tools/Fetch-SolarMinorCatalog.py --offline obnoví katalog ze sdílených veřejných
snapshotů. Tools/Prepare-SolarDetails.py --offline obnoví prstence a mapy;
Tools/Prepare-SolarUniverse.py sestaví jediná běhová data v Content/Data.
Starý37tělesový layout je uchován v Art/Space/SolarSystem/system-reference.json.
Nesmí přepisovat rozšířená běhová data.

Galaktické souřadnice jsou světelné roky, místní pozice jsou double metry.
Mapa drží odděleně galaktický kotvící bod a místní posun, takže při přiblížení
neztrácí přesnost malých těles. Scéna pouze čte tato data a stav letu.
Vzdálené promítání zachová úhlovou velikost a vyhýbá se astronomickým engine
souřadnicím. Při změně soustavy se odstraní pouze vlastněná prezentace.
Laboratoř je stále samostatný experiment bez nové perzistence či skutečných
aktuálních efemerid. Budoucí orbitální dynamika a hyperpohon jsou další etapa.

Ověření: Tools/Invoke-SolarFlight.ps1 Build / Assets / Test / Package / Visual /
Smoke. Visual je skutečný samostatný3840×2160 render s ovládáním mapy a letu;
testované aplikace mají vlastní procesy a ukládají do ignorované .local.
Dokončené výsledky se zapisují do CURRENT_STATE a Tasks/0016/HANDOFF až po testu.

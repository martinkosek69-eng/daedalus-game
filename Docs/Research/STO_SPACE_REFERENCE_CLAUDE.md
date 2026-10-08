# Star Trek Online — podrobná rešerše vesmírné části jako předlohy pro Daedalus

- Autor: Claude Code (rešerše pro Codex a uživatele)
- Datum: 3. 10. 2026
- Větev: `research/claude-sto-space-reference`, založená z `codex/solar-flight` @ `d492d94`
- Zadání: uživatel chce maximálně podrobný průzkum vesmírné části Star Trek Online
  (dále STO). Pozemní akce jsou mimo rozsah. Ve STO hledáme předlohu a inspiraci,
  protože i naše hra bude z velké části ze světa Star Treku.
- Navazuje na úkol 0005 (letový model). Ten je na větvi
  `task/0005-sto-flight-reference` v `Docs/Research/STO_FLIGHT_CLAUDE.md`.
  Jeho závěry jsou zde shrnuté a doplněné.
- Stav: hotová rešerše k posouzení. Není to schválený design hry. Rozhodnutí o tom,
  co převezmeme, dělá uživatel s Codexem.

## Jak tento dokument číst

Každé tvrzení má značku původu:

| Značka | Význam |
| --- | --- |
| **[OFICIÁLNÍ]** | Oficiální text Cryptic/Arc: vývojářský blog, článek se statistikami lodí, oficiální Q&A. Uvádím ID článku a datum. |
| **[VÝVOJÁŘ]** | Jmenovaný vývojář STO v rozhovoru pro jiný web. |
| **[KOMUNITA]** | Hráčská wiki, návod nebo fórum. Mechaniky jsou často změřené hráči, ne potvrzené vývojáři. U archivních kopií uvádím datum snímku. |
| **[ODVOZENO]** | Můj závěr nebo návrh pro naši hru. |
| **[NEOVĚŘENO]** | Údaj, který jsem nemohl potvrdit, nebo jen nepřímý zdroj. |

Důležité výhrady:

- STO se vyvíjí od roku 2010. Čísla se mezi sezónami měnila, například v Season 13
  přišlo velké vyvážení zbraní. U údajů proto uvádím rok. Starší vývojářské
  odpovědi (2008–2009) popisují záměr před vydáním, ne nutně dnešní stav.
- Komunitní vzorce nejsou kód Cryptic. Používám je jako pozorovaný model chování,
  ne jako „pravdu“ hry.
- STO jsem nehrál a nevyhodnocoval jsem videa. Vizuální popisy vycházejí
  z oficiálních textů a hráčských popisů, ne z mého pozorování.
- Autorská práva: STO, jeho vlastní lodě (například Odyssey, Jupiter, Pathfinder),
  texty i modely patří Cryptic/Arc a CBS/Paramount. Z rešerše přebíráme principy
  a mechaniky, nikdy assety, texty ani konkrétní vlastní návrhy STO. Do repozitáře
  jsem nic z hry nekopíroval. Citace jsou jen krátké a vše ostatní je převyprávěné.

---

## 1. Shrnutí: 20 nejdůležitějších poučení ze STO pro naši hru

1. **Velká loď se má chovat jako válečná loď nebo ponorka, ne jako stíhačka.**
   Vývojáři to tak popsali ještě před vydáním **[VÝVOJÁŘ]**. Zatáčení, rozjezd,
   brzdění i obratnost záviset na velikosti lodi **[OFICIÁLNÍ]**. Velké lodě STO
   se otáčejí 4–6 °/s, malé obratné lodě 15–20 °/s **[OFICIÁLNÍ]**.
2. **Vesmír má tři úrovně mapy.** Mapa sektoru je přehledová mapa s cestováním
   warpem. Mapa soustavy je 3D prostor, kde se létá a bojuje. Pozemní mapy jsou
   přístupné transportérem **[OFICIÁLNÍ]**. Tahle architektura dobře sedí na náš
   plán s mnoha soustavami.
3. **Hráč loď ovládá přímo, ne klikáním na cíl.** Boj je taktický: manévrujete
   tak, abyste si chránili slabou stranu a stříleli do nepřítelovy **[OFICIÁLNÍ]**.
4. **Čtyři štítové sektory (přední, zadní, levý, pravý) jsou srdcem taktiky.**
   Zatáčíte tak, aby se poškození rozložilo. Energii mezi sektory přelijete
   a torpéda posíláte na sektor, který už je dole **[KOMUNITA]**.
5. **Napájení je rozdělování energie, ne „mana“.** Energie z jádra se dělí mezi
   zbraně, štíty, motory a pomocné systémy. Nikdy nedojde úplně, jen vždy něco
   obětujete **[OFICIÁLNÍ]**.
6. **Zbraně mají palebné úhly.** Paprsky 250°, dvojitá děla 45°, torpéda 90°.
   Z toho vzniká styl lodí: křižník bojuje bokem, eskorta přídí **[KOMUNITA]**.
7. **Energetické zbraně ubírají štíty i trup stejně.** Kinetické zbraně (torpéda)
   jsou proti štítům slabé, štíty jim odolávají ze 75 %, ale plně zasáhnou trup
   **[KOMUNITA]**. Z toho plyne rytmus boje: paprsky shodí štít, torpéda dorazí trup.
8. **Pohyb je obrana.** Loď v pohybu má vyšší obranu a stojící loď je snadný cíl
   **[KOMUNITA, OFICIÁLNÍ záměr]**.
9. **Pevný cyklus zbraní (5 s) a sdílené prodlevy torpéd (2 s)** dělají boj
   čitelným a dobře se vyvažují **[KOMUNITA]**.
10. **Důstojníci na můstku jsou hlavní zdroj schopností.** Počet a hodnost
    křesel určuje loď. Tak se z lodí stávají odlišné „třídy“ bez složitých stromů
    **[OFICIÁLNÍ]**.
11. **Typy lodí mají jasné role.**
    - Křižník: odolný, bojuje bokem, podporuje spojence.
    - Eskorta: obratná, má děla, je křehká.
    - Vědecká loď: silné štíty a kontrola bojiště.
    - Nosič: pomalý, bojuje stíhačkami.
    - Lodě se stejným typem sdílejí základní pravidla **[OFICIÁLNÍ, KOMUNITA]**.
12. **Každá frakce má čitelnou barvu a chování zbraní.** Phaser je oranžový,
    disruptor zelený, plazma zelenomodrá, polaron fialový **[KOMUNITA]**.
13. **Zničení lodi je událost.** Jádro exploduje asi 5 s po ztrátě trupu a poškodí
    lodě v okruhu několika km. Singularitní jádro vytvoří krátkou černou díru
    **[KOMUNITA]**.
14. **Nepřátelé mají žebříček.** Stíhačka < fregata < křižník < bitevní loď <
    dreadnought. Každý stupeň musí být znatelně nebezpečnější. AI dělá nálety
    (strafing runs), drží si preferovanou vzdálenost, léčí se a povolává
    posily **[OFICIÁLNÍ]**.
15. **Výroba lodí:** koncept → 3D blokování → model → sdílené dlaždicové
    materiály → přes 80 uzlů pro efekty → stats a schopnosti → QA „pocitu“ lodi
    **[OFICIÁLNÍ]**.
16. **Modulární lodě.** Lodě se skládají z dílů (talíř, trup, gondoly, pylony),
    které jdou kombinovat v rámci rodiny lodí. Materiály, okna, barvy a registrační
    číslo si hráč nastaví **[OFICIÁLNÍ, KOMUNITA]**.
17. **Efekty musí být čitelné.** Každá schopnost má rozpoznatelný vizuální jazyk.
    Staré prvky ze seriálu se modernizují, ale musí zůstat poznatelné
    **[OFICIÁLNÍ]**.
18. **Zásady vyvažování od vývojářů:**
    - hra má být hlavně zábavná,
    - investice hráče si udrží hodnotu,
    - žádná volba nesmí být vždy správná ani vždy špatná,
    - po vypršení omezujícího efektu krátce platí odolnost proti dalšímu
    **[OFICIÁLNÍ]**.
19. **Ovládání na ovladači:**
    - radiální menu,
    - zámek cíle, kdy kamera drží v záběru obě lodě,
    - volitelná automatizace schopností.

    Vývojáři zjistili, že hráči „hráli spíš UI než hru“ **[OFICIÁLNÍ]**.
20. **Čemu se vyhnout.** STO je MMO, takže má spoustu monetizace, inflaci síly
    (power creep), stovky položek a desítky schopností najednou. Převzít máme jádro
    taktiky, ne objem. Pro hru jednoho hráče chceme menší a hlubší sadu
    **[ODVOZENO]**.

---

## 2. Zdroje

### 2.1 Oficiální zdroje (Cryptic / Arc Games)

Oficiální články STO jsou dostupné přes veřejné rozhraní Arc Games. Z něj jsem
stáhl rejstřík 6 026 článků z let 2008–2026 a přečetl ty relevantní:

- seznam: `https://api.arcgames.com/v1.0/games/sto/news?limit=100&offset=N`
- článek: `https://api.arcgames.com/v1.0/games/news/<ID>?field[]=content&field[]=author&field[]=updated`
- ve webovém prohlížeči: `https://www.playstartrekonline.com/en/news/article/<ID>`

| ID | Datum | Název | Použito pro |
| --- | --- | --- | --- |
| 1059830 | 2008-09-04 | Ask Cryptic | vesmír má působit obrovský, měřítko lodí |
| 1059810 | 2008-10-27 | Ask Cryptic | přímé ovládání lodi, taktické manévrování |
| 1059800 | 2008-11-26 | Ask Cryptic | napájení bez „many“, přerozdělování energie |
| 1059790 | 2009-01-07 | Ask Cryptic | fyzika podle velikosti lodi, umělecký styl, tiery |
| 1059740 | 2009-04-30 | Ask Cryptic | tři úrovně map, kolize, beranidlo, tažný paprsek |
| 1059700 | 2009-09-03 | Ask Cryptic: Combat Roundup | odběr energie zbraněmi, útočné vzorce, křesla důstojníků, sdílené prodlevy |
| 1055330 | 2009-11-23 | Visual Effects Dev Diary on IGN | zásady efektů: modernizovat, ale zachovat poznatelnost |
| 1058730 | 2012-05-01 | Season 5 Dev Blog #44: Carrier Command | povely nosičů a stíhaček |
| 1012660 | 2013-04-25 | LoR Dev Blog #10: Warbird Singularity Powers | singularitní jádro |
| 1011770 | 2013-05-13 | LoR Dev Blog #17: Warp Cores | warp jádra a přepočet warpu |
| 1002780 | 2013-10-18 | Season 8 Dev Blog #7 | návrh boje s Voth |
| 3029673 | 2014-01-17 | Season 8 Dev Blog #37 | přepracování Undine a AI |
| 3030983 | 2014-01-24 | Dev Blog: Ships – From Start to Finish | celá výroba lodi |
| 3031073 | 2014-01-30 | Season 8 Dev Blog #44 | sekundární deflektory |
| 3039113 | 2014-03-26 | Season 9 Dev Blog #6 | trvalá vesmírná bojová zóna |
| 9078783 | 2015-02-27 | Sector Space Revamp | mapa sektorů, kánonové pozice soustav |
| 9226533 | 2015-05-06 | Fed Pilot Ship Stats | statistiky pilotních eskort |
| 9667053 | 2015-11-23 | Building Jupiter | nosič, statistiky a stíhačky |
| 9782283 | 2016-02-10 | Federation Flagships (Lead Systems Designer) | statistiky Star Cruiser |
| 9792703 | 2016-02-16 | Art of Federation Ships | design variant a modulární díly |
| 9797693 | 2016-02-18 | Skill System Revamp | zásady progrese |
| 9888133 | 2016-03-29 | New Visual Slots | vzhled vybavení oddělený od statistik |
| 10010203 | 2016-06-10 | Console Space UI | ovladač, radiální menu, zámek cíle, automatizace |
| 10029803 | 2016-06-20 | Navigating Console UI | kontextové menu |
| 10426883 | 2017-03-14 | Details on Space Balance Changes | zásady vyvažování |
| 11579886 | 2026-02-17 | Command the Aetherian Harmony | statistiky nosiče |

### 2.2 Vývojáři STO v rozhovorech pro jiné weby

| Zdroj | Datum | Kdo | Použito pro |
| --- | --- | --- | --- |
| [RPG Site: Q&A with Cryptic](https://www.rpgsite.net/interview/2916-star-trek-online-qa-with-cryptic) | 2010-02-17 | Cryptic | lodě jako „tall ships or submarines“ |
| [Ten Ton Hammer: Space Combat Q&A](https://www.tentonhammer.com/articles/star-trek-online-space-combat-q-a) | 2009-11-02 | Craig Zinkievich, výkonný producent | rychlosti, plný impuls, nehybnost škodí |
| [Destructoid](https://www.destructoid.com/we-asked-star-trek-onlines-lead-starship-artist-for-picard-spoilers/) | 2020-01-26 | Thomas Marrone, vedoucí výtvarník lodí, efektů a UI | čas výroby lodi, nárůst detailu |
| [Bleeding Cool](https://bleedingcool.com/games/interview-star-trek-onlines-starship-artist-donny-versiga/) | 2021-08-07 | Donny Versiga, výtvarník lodí | pracovní postup, rozpočty paměti a polygonů |
| [TrekCentral](https://trekcentral.net/thomas-marrone-talks-star-trek-online-starship-design-interview/) | 2025-02-11 | Thomas Marrone, art director | remastery, kánonové předlohy |
| [MMORPG.com](https://www.mmorpg.com/interviews/interview-chatting-star-trek-online-ship-design-philosophy-and-star-trek-canon-with-ep-thomas-marrone-2000137319) | 2026-02-17 | Thomas Marrone, výkonný producent | proces návrhu lodi, měřítko v rámci rodiny lodí |

### 2.3 Komunitní zdroje

Hlavní komunitní wiki byla dlouho na sto.fandom.com, dnes je na stowiki.net.
Živé stránky mi blokovaly ochrany proti robotům a ty jsem neobcházel. Použil jsem
proto archivní kopie z Wayback Machine (web.archive.org) s datem snímku.

| Stránka | Snímek | Použito pro |
| --- | --- | --- |
| Starship (Power and Subsystems) | 2022-09-30 | napájení, plný impuls, energetický rozvod (EPS) |
| Space combat | 2022-07-11 | HUD, taktiky, dostřel |
| Damage type (space) | 2023-01-30 | typy poškození, barvy, vedlejší efekty zbraní, torpéda |
| Ship weapon | 2022-08-27 | palebné úhly, boční palba, cykly, základní poškození |
| Beam Array, Dual Beam Bank, Cannon, Dual Cannons, DHC, Turret, Torpedo/Mine Launcher | 2022 | jednotlivé zbraně |
| Accuracy, Critical Hit, Damage resistance | 2022 | výpočty zásahu, kritických zásahů a odolnosti |
| Ship Shields, Skill: Shield Regeneration | 2022 | typy štítů a regenerace |
| Playable starship | 2023-01-03 | statistiky, sloty, tiery, typy a úpravy lodí |
| Bridge officer, Bridge officer and kit abilities | 2023-01-30 | křesla důstojníků a schopnosti |
| Career path | 2025-11-15 | schopnosti kapitánů |
| Cruiser/Carrier commands, Hangar pet, Singularity Core abilities, Starship Separation, Cloak, Subsystem Targeting, Full Impulse, Evasive Maneuvers | 2021–2023 | vestavěné mechaniky |
| Warp speed, Sector space, Patrol, Difficulty, Non-playable starship | 2022–2023 | warp, výbuch jádra, mapy, nepřátelé |
| Key binds, List of console commands, Inertia | 2021–2022 | ovládání (převzato z úkolu 0005) |
| [Starship Mechanics Guide](https://www.leveling-guides.com/star-trek-online-starship-mechanics-guide/) | 2010, aktualizace 2017 | hráčské vzorce pohybu |
| Diskuse na Steamu ke STO | 2015–2017 | pozorování letu |

Archivní adresa má tvar `https://web.archive.org/web/<datum>/https://sto.fandom.com/wiki/<Stránka>`.

### 2.4 Co se nepodařilo

- stowiki.net a oficiální fórum forum.arcgames.com blokovaly ochrany proti robotům.
  Neobcházel jsem je.
- Původní IGN deník o vizuálních efektech (2009) jsem nenašel. Znám jen citaci
  v oficiálním odkazu 1055330.
- Neexistuje veřejný technický údaj o měřítku lodí ve hře, například kolik metrů
  měří Galaxy ve STO. Měřítko proto popisuji jen podle záměru vývojářů.
- Videa ani hru jsem neměřil. Úhly naklonění, časové konstanty a podobně zůstávají
  jako otevřené otázky (kapitola 21).

---

## 3. Co je STO a jaká je jeho designová filozofie

- STO vydal Cryptic Studios 2. 2. 2010 pro Windows. Od 17. 1. 2012 je zdarma
  (free-to-play). Na PS4 a Xbox One vyšel v září 2016. V letech 2024–2025 hru
  vyvíjelo DECA Games za podpory Cryptic a vydává ji Arc Games. **[KOMUNITA:
  Wikipedia]**
- Recenze byly smíšené, ale vesmírný boj byl chválen. GameSpot ho ocenil a zbytek
  hry označil za plochý. **[KOMUNITA: Wikipedia]**

### 3.1 Záměry vývojářů před vydáním (2008–2010)

- **Přímé ovládání [OFICIÁLNÍ 1059810].** Hráč má mít nad lodí plnou kontrolu,
  ne klikání na cíl jako v EVE. Boj je taktický: postavíte loď tak, abyste si
  chránili životně důležité systémy a útočili na nejslabší stranu soupeře.
- **Lodě jako válečné lodě a ponorky [VÝVOJÁŘ, RPG Site 2010].** Lodě se hýbou
  ve třech rozměrech, ale tempem velkého torpédoborce, ne stíhačky. Tím se
  zachová pocit měřítka. Vývojáři dali přednost „pocitu“ a taktickému tempu před
  plnou newtonovskou fyzikou, která by hráče zahltila.
- **Fyzika podle velikosti [OFICIÁLNÍ 1059790].** Poloměr zatáčení, zrychlení,
  zpomalení i obratnost závisí na velikosti lodi:
  - Malé stíhačky můžou „smykovat“ a rychle manévrovat.
  - Velké lodě zatáčejí plynule, pomalu zrychlují i zastavují.
- **Vesmír má působit obrovský [OFICIÁLNÍ 1059830].** Zároveň má zůstat zábavný.
  Vývojáři hledali rovnováhu mezi pocitem nesmírného prostoru a svižnou hrou.
- **Měřítko lodí [OFICIÁLNÍ 1059830].** Vývojáři přiznali kompromis:
  - Velké lodě (od eskort po bitevní křižníky a Borg kostky) mají přibližně
    věrné měřítko.
  - Raketoplány by vedle lodi třídy Galaxy byly ve skutečném měřítku skoro
    neviditelné, a to není zábavné. Proto jsou zvětšené.
- **Napájení [OFICIÁLNÍ 1059800].** Žádná klasická „mana“ z MMO:
  - Warp jádro dodává energii a hráč ji rozděluje.
  - Energie nedojde tak, že byste čekali na výstřel, to by nebylo zábavné.
  - Kdo posílí jeden systém, oslabí jiný. Nelze mít naplno štíty i motory.
  - Energie se v boji přelévá: štíty před ranou, potom motory na manévr.
- **Malé lodě mají šanci [OFICIÁLNÍ 1059800].** Skupina malých lodí s dobrou
  taktikou může porazit velkou loď.
- **Umělecký směr [OFICIÁLNÍ 1059790].** Fantastický, ne kreslený:
  - realistický, ale o stupeň „zázračnější“,
  - s živými barvami a pocitem úžasu, jaký divák od Star Treku čeká.
- **Konflikt a ideály [OFICIÁLNÍ 1059830].** Federace je idealistická, ale
  drama potřebuje konflikt. Kde to jde, hra nabídne i jiné řešení než boj,
  pokud je zábavné.

---

## 4. Struktura vesmíru: mapy, cestování a obsah

### 4.1 Tři úrovně map [OFICIÁLNÍ 1059740]

1. **Mapa sektoru (sector space).** Přehledová mapa, kde loď letí warpem mezi
   soustavami.
   - Není úplně plochá ani plně 3D.
   - Obsahuje mřížky sektorů, planety, soustavy, stanice, mlhoviny a anomálie.
   - Potkáte tu jiné hráče.
   - Odehrávají se tu náhodné události, například „Kapitáne, nouzové volání!“.
2. **Mapa soustavy (system space).** Plné 3D prostředí, kde se bojuje.
   - Často je vytvořená zvlášť pro hráče (instance).
   - Některé jsou trvalé, s neustále probíhajícím bojem, kam se může kdokoli přidat.
3. **Pozemní mapa.** Povrch planety nebo interiér stanice, přístupný
   transportérem ze soustavy. Pozemní akce jsou mimo rozsah této rešerše.

**Mapa sektoru dnes [KOMUNITA Sector space 2023; OFICIÁLNÍ 9078783]:**

- Jedna velká mapa na kvadrant (Alfa, Beta, Delta). V roce 2015 zmizely
  neviditelné zdi mezi sektory a mapy se rozšířily do obdélníku.
- Cíl lze vybrat ze seznamu, kliknout na mapu, nebo k němu ručně doletět.
- Volba „Astrometrics“ zobrazí oběžné dráhy a mřížku roviny galaxie.
- Kvadranty spojují brány a červí díra, například Bajorská červí díra.
- **Kánonové pozice soustav [OFICIÁLNÍ 9078783].** Tým zmapoval všechny mapy
  soustav ve hře. Kánon dohledával v Memory Alpha a v atlasu *Star Trek: Star
  Charts* (Geoffrey Mandel), který vzal jako hlavní předlohu. Vznikla tabulka
  přes 350 soustav, včetně údajů o skutečných hvězdách. Kánonové soustavy
  přesunuli na správná místa a mezery doplnili vlastními soustavami.
  → **[ODVOZENO]** Stejný přístup se hodí i pro nás: tabulka soustav se zdrojem
  pozice (kánon, reálná hvězda, vlastní) jako jeden zdroj dat.

### 4.2 Rychlosti a režimy cestování

**Impuls (podsvětelná rychlost):**

- Normální let na mapě soustavy.
- Rychlost určuje statistika lodi, motor a energie do motorů. Při 50 energie
  letí loď základní rychlostí a každý bod navíc přidá 2 % **[KOMUNITA]**.

**Plný impuls (Full Impulse) [KOMUNITA Full Impulse 2021; Power 2022]:**

- Cestovní režim na mapě soustavy (klávesa Shift+R), zhruba 2–3× rychlejší
  než běžný impuls.
- Energie motorů jde na 100 a ostatní systémy spadnou na 5.
- Při vstupu do boje se režim vypne a energie se vrací plynule podle rychlosti
  energetického rozvodu (EPS).
- Kdo vletí do boje plným impulsem, má zbraně na zlomku síly a téměř žádnou
  regeneraci štítů. Proto se zastavuje 3–5 km před dostřelem.
- Vývojář před vydáním: při plném impulsu jde veškerá energie do motorů, takže
  na štíty zbude málo a na zbraně nic **[VÝVOJÁŘ, Zinkievich 2009]**.

**Warp [KOMUNITA Warp speed 2023; OFICIÁLNÍ 1011770]:**

- Pouze na mapě sektoru. Stupnice je lineární, warp 8 je dvakrát rychlejší
  než warp 4.
- V roce 2013 vývojáři změnili stupnici: zobrazení omezili na kánonový warp 10,
  ale skutečnou rychlost zvýšili. Rozdíl mezi startovní a pozdní lodí totiž byl
  skoro nepostřehnutelný a hráči necítili pokrok **[OFICIÁLNÍ]**.
- Dočasné zrychlení: kvantový slipstream (alespoň warp 20 na 30–90 s,
  prodleva 2 min).
- Transwarp: okamžitý přesun na pevně dané cíle.

**Animace vstupu do warpu [KOMUNITA]:**

- Efekt se liší podle frakce (lodě z éry Discovery mají vlastní styl).
- Některé lodě mají zvláštní animaci, například Intrepid zvedá gondoly.
- Některé mají vlastní animaci i pro transwarp (Crossfield).

### 4.3 Formy obsahu ve vesmíru

**Hlídky (patrols) [KOMUNITA 2023].** Malé mise v soustavách, opakovatelné
po 30 minutách. Většinou jde o boj, ale jsou i diplomatické, vyjednávací
a závodní.

**Trvalá bojová zóna (battlezone) [OFICIÁLNÍ 3039113, 2014].** Na příkladu
Undine:

- Mapa má 9 strategických bodů tří typů. Hráč u jednoho bodu stráví 5–10 minut.
- Obtížnost bodu se přizpůsobuje počtu hráčů v okolí. Zvládne ho jeden hráč
  i celá flotila.
- Dobytý bod hájí spojenci. Nepřítel se ho pokusí získat zpět, ale jeden hráč
  to dokáže odvrátit.
- Po dobytí všech bodů se otevře časově omezený útok na tři „zabijáky planet“.

**Náhodná setkání v hlubokém vesmíru** přímo na mapě sektoru **[KOMUNITA]**.

**Úrovně obtížnosti [KOMUNITA Difficulty 2022]:**

- Normal: výchozí, bez zranění posádky.
- Advanced: lepší odměny, možná zranění.
- Elite: nejtěžší, vyžaduje silnou výbavu.

**[ODVOZENO] pro nás:**

- Hlídky jsou krátká opakovatelná obsahová jednotka.
- Bojová zóna je model „živé“ fronty: obtížnost se škáluje podle síly hráče,
  spojenci body brání a protivník se je snaží získat zpět. Pro hru jednoho
  hráče stačí škálovat obtížnost podle síly lodi hráče a jeho spojenců.

---

## 5. Let a fyzika

Podrobnosti jsou v dokumentu úkolu 0005. Tady je shrnutí doplněné o nové
oficiální zdroje.

### 5.1 Co je potvrzené

**Oficiální statistiky obratnosti [OFICIÁLNÍ]:**

| Loď | Otáčení (°/s) | Impulse Modifier | Inertia | Zdroj |
| --- | --- | --- | --- | --- |
| Nosič Aetherian Harmony | 4 | 0,15 | 20 | 11579886 (2026) |
| Nosič Jupiter | 6 | – | – | 9667053 (2015) |
| Star Cruiser (rodina Odyssey) | 6 | 0,15 | 30 | 9782283 (2016) |
| Pilotní eskorta | 20 | 0,24 | 75 | 9226533 (2015) |
| Další eskorty (jen ve výsledcích vyhledávání, stránky neotevřeny) | 14–17 | 0,18–0,21 | 60–70 | [NEOVĚŘENO] |

- **Inertia je oficiální statistika** lodi. V oficiálních textech jsem nenašel
  její vzorec ani převod na sekundy.
- **Komunitní výklad [KOMUNITA]:**
  - Vyšší číslo znamená, že loď rychleji mění rychlost a méně klouže.
  - Nízká inertia spolu s vysokou obratností umožní „power slide“: příď míří
    jinam, než loď letí.
  - Klouzání snižuje obranu.
- **Velikost lodi ovlivňuje zatáčení, zrychlení i brzdění** **[OFICIÁLNÍ
  1059790]**.
- **Srážky [OFICIÁLNÍ 1059740, záměr 2009]:**
  - Lodě do sebe narážejí, ale náraz nezpůsobí poškození.
  - Poškození dělá jen speciální útok beranidlem (Ramming Speed).
  - Tažný paprsek drží cíl, aby se nemohl otočit ani uniknout.
  - Odpuzovací paprsek může odtlačit loď, které hrozí výbuch jádra.

### 5.2 Co je komunitní pozorování

- **Plyn [KOMUNITA]:**
  - Nastavení plynu trvá, dokud ho nezměníte.
  - Kláves E/Q mění plyn o jeden dílek (20 %), R přepíná mezi 0 a 100 %.
  - Couvání jede asi čtvrtinovou rychlostí.
  - Couvání déle než 10 s postupně ubírá všechny energie, až o 25 bodů
    (Reverse Power Drain). Jde o úmyslný trest za dlouhé couvání.
- **Zatáčení podle plynu:**
  - Plnou rychlost otáčení má loď od asi 25 % plynu výš.
  - Při nulovém plynu zatáčí jen asi 3–4 °/s.
  - Vyšší plyn znamená větší poloměr otáčení.
  - Při couvání má loď plnou rychlost otáčení.
- **Náklon a kopání:**
  - Kopání (pitch) je omezené na zhruba 75° vůči rovině ekliptiky
    **[NEOVĚŘENO, terciární zdroj]**. Smyčka nejde udělat a na změnu výšky se
    letí ve spirále.
  - Náklon do boku (roll) hráč neovládá. Loď se při zatáčení nakloní sama.
  - Rychlost kopání se zdá navázaná na rychlost zatáčení.
- **Pohyb dává obranu:** bonus k obraně je nejvyšší zhruba při ¾ impulsu.
  Stojící loď je snadný cíl **[KOMUNITA]**. Shoduje se s vývojářem z roku 2009,
  že nehybnost škodí **[VÝVOJÁŘ]**.
- **Úhybné manévry (Evasive Maneuvers):**
  - schopnost všech kapitánů,
  - krátce výrazně zvýší rychlost, obratnost a obranu,
  - aktivace trvá 0,5 s,
  - sdílí 15s prodlevu s Impulse Burst,
  - s plným impulsem zvýší jen obratnost, ne rychlost.
- **Pilotní manévry:** zvláštní mechanika pilotních lodí. Dvojitým poklepáním
  na směr loď uskočí, dostane krátkou nezranitelnost a spotřebuje „palivo
  trysek“ **[OFICIÁLNÍ 9226533, KOMUNITA]**.

### 5.3 Doporučení pro náš letový model [ODVOZENO]

Doporučení z úkolu 0005 platí dál:

- výchozí rychlost otáčení 6 °/s pro 600 m loď,
- reakce řízení kolem 0,5 s,
- mírné klouzání,
- vizuální náklon do boku, který se po manévru sám vrátí,
- kopání omezené na 75–80°,
- kamera s vodorovným horizontem.

Nové podněty:

1. Rychlost otáčení snížit při velmi nízkém plynu a plnou ji dát od zhruba 25 %.
   Hráč tak cítí, že se loď nejlépe točí v pohybu.
2. Pohyb by měl přidávat obranu, až bude boj. Jednoduchá a čitelná odměna za
   manévrování.
3. Srážky bez poškození jsou rozumný základ. Případné beranění by mělo být
   samostatná vědomá akce.
4. Plný impuls jako cestovní režim s cenou: slabé zbraně a štíty a pomalý
   návrat energie. Řeší rychlé přesuny v soustavě bez ztráty taktiky v boji.
5. Až přijde energie, napojit rychlost na energii motorů: 50 = základ,
   +2 % za každý bod navíc.

---

## 6. Ovládání, kamera a HUD

### 6.1 Klávesnice a myš na PC

Podle hráčské wiki, archivní snímek 2021. Oficiální přehled ovládání jsem nenašel.

| Akce | Klávesa |
| --- | --- |
| Nos nahoru / dolů | W / S |
| Zatočit vlevo / vpravo | A / D |
| Plyn +1 dílek (20 %) / −1 dílek | E / Q |
| Plyn 0 ↔ 100 % | R |
| Plný impuls | Shift+R |
| Kamera zpět za loď | Home |
| Rozhlížení myší | tah pravým tlačítkem |
| Otáčení kamery | tah levým tlačítkem |
| Kamera ke kurzoru | levé + pravé tlačítko současně |
| Kamera na cíl | X |
| Přiblížení a oddálení | kolečko, Page Down (kroky 15/30/50 jednotek) |

- **Konzolové příkazy [KOMUNITA]:** `ThrottleAdjust` (posun plynu),
  `ThrottleSet` (záporná hodnota = couvání, 0 = stop), `ThrottleToggle`,
  `camdist` (vzdálenost kamery), `camCycleDist`.
- **Řízení myší** není hlavní způsob ovládání. Hráči používají volnou kameru
  a drží obě tlačítka **[KOMUNITA]**.

### 6.2 HUD ve vesmíru [KOMUNITA Space combat 2022]

- **Posuvník impulsu (plynu).**
- **Schéma štítů a trupu:** loď se čtyřmi sektory štítů a stavem trupu.
  - Klik na střed rozdělí štíty rovnoměrně.
  - Klik na šipku přelije část energie z ostatních tří sektorů do vybraného.
- **Úrovně energie:** zbraně, štíty, motory a pomocné systémy, s předvolbami.
- **Ovládání zbraní:** vystřelit všechny paprsky, všechna torpéda, nebo vše.
  Pravým klikem na zbraň zapnete automatickou palbu a pořadí zapnutí určuje
  prioritu.
- **Lišty schopností:** řádky 1–8, Ctrl+1–8 a Alt+1–8, plus schopnosti
  důstojníků na můstku.
- **Palebný úhel:** po najetí myší na ikonu zbraně se zobrazí její úhel.

### 6.3 Poučení z převodu na ovladač [OFICIÁLNÍ 10010203, 2016]

- Hra na PC používá skoro 72 kláves, ovladač má 12 tlačítek. Tým proto před
  výrobou rychle zkoušel prototypy ovládání.
- **Radiální menu podle kategorií** (kapitán, věda, strojovna, taktika, konzole):
  - krátký stisk = hned spustí předvolenou schopnost,
  - podržení = otevře kruhové menu,
  - v otevřeném menu jde spustit několik schopností za sebou (rychlý úder,
    týmové léčení).
- **Zámek cíle** (stisk pravé páčky): kamera drží vlastní loď i nepřítele
  v záběru vhodném pro boj, takže hráč nemusí řídit kameru. Pohybem páčky
  přepne na další bližší nebo vzdálenější cíl.
- **Automatizace schopností.** Při testech „hráči hráli spíš UI než hru“.
  - Hráč si vybere, které schopnosti se spouštějí samy a za jakých podmínek,
    například oprava trupu při poškození nebo negativním efektu.
  - Prodlevy dál platí a některé schopnosti automatizovat nejdou.
- **Zjednodušení:**
  - štíty se vyrovnávají jedním stiskem, bez posílení jednoho sektoru,
  - energie se přepíná předvolbami „vše do jednoho systému“.

**[ODVOZENO] pro nás:**

- Hlavní řízení lodi ať je na klávesách nebo páčce, myš ovládá kameru.
  Stejně to máme v plánu i v prototypu.
- Pro souboj pomůže zámek cíle s kamerou, která drží obě lodě v záběru.
- Pro začátečníka (náš uživatel) se hodí volitelná automatizace obrany
  a jednoduché vyrovnání štítů jedním tlačítkem.
- Ovládání navrhnout od začátku i pro ovladač. Je to levnější než převod
  později.

---

## 7. Napájení lodi (energie a podsystémy)

### 7.1 Původní záměr [OFICIÁLNÍ 1059800, 1059700]

- Warp jádro je zdroj energie. Hráč ji přerozděluje mezi zbraně, štíty, motory
  a pomocné systémy. Nikdy nedojde úplně, vždy jde jen o kompromis.
- Před vydáním (2009) vývojáři popsali 100 jednotek energie mezi čtyřmi systémy.
  Dnes je to bazén 200 (viz 7.2). Princip zůstal stejný, čísla se změnila.
- **Odběr zbraněmi.** Bez ztráty jdou vystřelit dvě energetické zbraně. Každá
  další ubírá energii zbraní a tím i poškození každé z nich. Plná boční salva
  osmi phaserů je tak silný, ale vyčerpávající úder.

### 7.2 Dnešní pravidla [KOMUNITA Starship (Power and Subsystems), 2022]

| Prvek | Pravidlo |
| --- | --- |
| Systémy | Zbraně, Štíty, Motory, Pomocné (Auxiliary) |
| Bazén | 200 bodů. Zvýšení jednoho systému sníží jiný. |
| Ruční rozsah | 15–100 na systém. Strop je obvykle 125 a předměty ho můžou zvýšit. |
| Předvolby | Uložené rozdělení, přepínatelné i v boji. Na ovladači „100 do jednoho“. |
| Zbraně | Násobitel energetického poškození = (energie + 100) / 200. Při 50 → 75 %, 100 → 100 %, 125 → 112,5 %. Na torpéda a miny nemá vliv. |
| Odběr zbraní | Střelba hráče ubírá energii zbraní. NPC si energii při střelbě neubírají (podle vývojáře z roku 2019). |
| Štíty | Každý bod = 2 % regenerace z hodnoty štítu (50 = 100 %) a 0,2 % pohlcení poškození (50 = 10 %). Kapacitu sektorů nemění. |
| Motory | 50 = základní rychlost. Každý bod nad 50 přidá 2 % (100 = +100 %), každý pod 50 ubere 2 %. Rychlost zvyšuje obranu. Na warp nemá vliv. |
| Pomocné | Síla vědeckých schopností (+2 % za bod nad 50), maskování a jeho odhalování, rychlejší doplňování stíhaček. |
| Energetický rozvod (EPS) | Rychlost přelévání energie mezi systémy. Zrychlují ho konzole a schopnost inženýra. |
| Plný impuls | Motory 100, ostatní 5. Po vypnutí se energie vrací se zpožděním. |
| Couvání | Couvání déle než 10 s ubírá každou sekundu 1 bod ze všech systémů, až −25. |
| Účinnost | Dovednosti a vlastnosti přidají nejvíc systémům, které běží na nízké úrovni. |

### 7.3 Warp jádra [OFICIÁLNÍ 1011770, 2013]

- Jádro mění pravidla energie a dává konkrétní sestavě charakter.
- Jádro hmota/antihmota zvedne strop jednoho systému na 130. Když tento systém
  běží pod 75, dává navíc bonus k účinnosti.
- Singularitní jádro Romulanů dává bonus energie do jednoho systému podle
  nabití singularity (viz kapitola 14).

### 7.4 [ODVOZENO] Pro nás

Napájení je ve STO „volant“ taktiky. Pro hru jednoho hráče doporučuji:

- začít se 3–4 systémy a jednoduchými předvolbami (útok, obrana, rychlost,
  rovnováha),
- zachovat cenu přepnutí: energie se přelévá postupně, ne okamžitě,
- odběr energie zbraněmi ponechat jako přirozenou brzdu „všech zbraní najednou“,
- přesná čísla (200, 15–100, 125) jsou pomůcka pro první ladění, ne závazek.

---

## 8. Štíty

### 8.1 Mechanika [KOMUNITA Playable starship 2023, Space combat 2022, Ship Shields 2022]

- **Čtyři sektory:** přední, zadní, levý a pravý (fore, aft, port, starboard).
  Zásah ubere sektor, ze kterého střela přiletěla. Když sektor padne,
  poškození z té strany jde rovnou do trupu.
- **Prosakování (bleedthrough):** standardně 10 % poškození jde přes štít
  rovnou na trup. Například 100 poškození phaserem znamená 90 na štít a 10 na
  trup. Odolné štíty (Resilient) mají jen 5 %.
- **Kinetické zbraně:** štíty mají vrozenou 75% odolnost proti torpédům a minám.
  Některá torpéda štíty částečně obcházejí (transphasic). **[KOMUNITA Ship weapon]**
- **Regenerace:** štíty se doplňují pravidelně, po 6 sekundách. Rychlost
  ovlivňuje typ štítu, energie štítů a dovednost (0,1 % kapacity za 6 s na bod).
  **[KOMUNITA Skill: Shield Regeneration]**
- **Tvrdost:** energie štítů pohlcuje část poškození (0,2 % za bod).
- **Kapacitu sektoru** určuje vybavení (štítový generátor) krát modifikátor
  štítů dané lodi. Odyssey má modifikátor 1,15 **[OFICIÁLNÍ 9782283]**, vědecké
  lodě mají „velmi vysoký“ **[OFICIÁLNÍ 3030983]**.
- **Přerozdělení:**
  - Ruční: přelít energii do ohroženého sektoru, nebo vyrovnat všechny.
  - Automatické na 10 s schopností Tactical Team.
  - Na ovladači jen vyrovnání všech sektorů jedním stiskem.

**Typy štítových generátorů:**

| Typ | Kapacita | Regenerace | Zvláštnost |
| --- | --- | --- | --- |
| Standardní | vyvážená | vyvážená | – |
| Covariant | +10 % | −25 % | – |
| Regenerative | −10 % | +25 % | – |
| Resilient | −5 % | −5 % | pohlcení 5 %, prosakování 5 % místo 10 % |

**Úprava štítů proti typu energie:** −20 % poškození od phaserů, disruptorů
a podobně.

### 8.2 Taktika, kterou štíty vytvářejí [KOMUNITA Space combat 2022; OFICIÁLNÍ 1059810]

- Natáčet loď tak, aby se poškození rozložilo do různých sektorů.
- Slabý sektor držet od nepřítele.
- Útočník se soustředí na jeden sektor a „prorazí díru“.
- Do prázdného sektoru posílá torpéda, která na trup působí plnou silou.
- Obránce čte, kterým sektorem přiletí torpédo, a reaguje (Brace for Impact).
- Výpadek celého štítu je silný efekt: cílení na podsystémy může štíty vypnout
  a tím shodit všechny sektory naráz (viz 14.6).

### 8.3 Vzhled štítů [OFICIÁLNÍ 9888133, 11568363; KOMUNITA]

- Vzhled štítu je ve STO oddělený od statistik. Od roku 2016 existují vizuální
  sloty, které přepíšou vzhled štítu, deflektoru a impulsních motorů beze
  změny statistik.
- Prodávají se i čistě kosmetické „vanity“ štíty (Borg, Discovery, Section 31…).
- **[NEOVĚŘENO]** Přesný tvar a chování zásahového efektu štítu (bublina kolem
  lodi, rozvlnění v místě zásahu, barva podle typu) jsem v textových zdrojích
  nenašel. Je potřeba ho změřit nebo prohlédnout přímo ve hře či na videu.

### 8.4 [ODVOZENO] Pro nás

Čtyři sektory s prosakováním a s rozdílnou odolností proti energii a kinetice
dávají hodně taktiky za malou složitost. Doporučuji je jako jádro budoucího
boje. Vizuál štítu stavět jako samostatný efekt, ne jako statistiku. Umožní to
pozdější úpravy vzhledu a čitelnost.

---

## 9. Trup, odolnosti a typy poškození

### 9.1 Trup [KOMUNITA Playable starship 2023]

- Trup je „život“ lodi. Když dojde, loď je zničena (viz 16).
- Kapacitu určuje typ a tier lodi. Například Star Cruiser má 57 000 na úrovni 60
  **[OFICIÁLNÍ 9782283]**.
- Trup se opravuje schopnostmi (Engineering Team, Auxiliary to Structural,
  Hazard Emitters) a pasivní regenerací.
- **Posádka (odstraněno 2015):** dříve počet posádky ovlivňoval opravy
  a výsadky a posádku šlo zranit. STO mechaniku v roce 2015 zrušilo.
  **[ODVOZENO]** Nejspíš proto, že přidávala složitost bez dost zábavy. Pro
  hru jednoho hráče může posádka zase dávat smysl jako příběhová a opravárenská
  vrstva, ale jen pokud bude čitelná.

### 9.2 Typy poškození [KOMUNITA Damage type (space) 2023; Ship weapon 2022]

Tři hlavní kategorie:

- **Energie:** paprsky a děla. Na štít i trup působí stejně.
- **Kinetika:** torpéda, miny a některé schopnosti. Proti štítům má jen 25 %
  účinnost, trup zasáhne plně.
- **Exotické:** schopnosti, které nejsou zbraně (důstojníci, konzole). Sílu
  určuje dovednost Exotic Particle Generator.

**Druhy energie** (barvu i vedlejší efekt má většina zbraní daného druhu,
efekt se spouští s šancí 2,5 %, pokud není uvedeno jinak):

| Druh | Barva | Vedlejší efekt | Typický uživatel |
| --- | --- | --- | --- |
| Phaser | oranžová / světle žlutá (TOS a andorijské modré) | vyřadí 1 podsystém na 5 s | Federace |
| Disruptor | zelená | −10 % odolnosti cíle | Klingoni, Cardassiané, Romulanské impérium, Orioni |
| Plazma | zelená / tyrkysová | trvalé poškození ohněm (DoT) | Romulané |
| Tetryon | modrá | poškození navíc všem sektorům štítu | Tholiané, Hirogeni |
| Polaron | růžová / fialová | −25 energie všem systémům | Dominion, Breen |
| Antiproton | karmínová s černým okrajem | +20 % síla kritického zásahu (trvale) | Borg, Undine, Voth, Iconiané |
| Proton | bělomodrá | při kritickém zásahu 50 %: poškození přes štíty | vzácné |

**Odolnosti:**

- úpravy štítů (−20 % proti jednomu druhu),
- pancéřové konzole (plating: 2 druhy silně, armor: 4 druhy středně, alloy:
  všechny slabě; zvláštní konzole proti kinetice),
- dovednosti a vlastnosti kapitána,
- frakce používají určité druhy, takže výběr odolnosti je taktické rozhodnutí
  před misí.

### 9.3 Výpočet odolnosti [KOMUNITA Damage resistance 2022]

- Hra počítá „hodnotu odolnosti“ (součet bonusů a postihů) a převádí ji na
  procento se **snižujícím se přínosem**. Křivka začíná lineárně (1 bod = 1 %)
  a blíží se 75 %.
- Body z tabulky: 10 → 9,1 %, 40 → 28,3 %, 50 → 32,8 %.
- 75 % lze překonat jen zvláštními dočasnými schopnostmi.
- **[ODVOZENO]** Pro nás je klíčový princip, ne přesný vzorec: odolnost
  nesmí dosáhnout 100 % a každý další bonus musí přinést méně. Vzorec
  navrhneme vlastní a otestujeme.

---

## 10. Zbraně

### 10.1 Typy zbraní a palebné úhly [KOMUNITA Ship weapon 2022 a stránky zbraní]

| Zbraň | Úhel | Montáž | Charakter |
| --- | --- | --- | --- |
| Paprskové pole (Beam Array) | 250° | příď i záď | široký úhel, menší poškození, základ všech lodí |
| Dvojitá paprsková banka (DBB) | 90° | jen příď | víc poškození |
| Dělo | 180° | jen příď | o něco víc než paprsek |
| Dvojitá děla (DC) | 45° | jen příď | velké poškození, jen některé lodě |
| Dvojitá těžká děla (DHC) | 45° | jen příď | pomalejší, větší salvy, +10 % síla kritického zásahu |
| Věž (Turret) | 360° | příď i záď | nejmenší poškození, dobrá na stíhačky |
| Všesměrový paprsek (Omni) | 360° | příď i záď; počet na loď je omezen [NEOVĚŘENO] | paprsek všemi směry |
| Torpédomet | 90° | příď i záď | těžké kinetické poškození, prodleva |
| Minomet | bez úhlu | záď | mina čeká, pak pronásleduje cíl |
| Experimentální zbraň | podle typu | zvláštní slot | jen některé lodě |

- Úhel zadních zbraní se měří od zádi.
- Kanóny, DBB a DC nejdou montovat dozadu.
- Dostřel je standardně **10 km**, některé zbraně mají 12 km.
- Paprsky mají plné poškození do **2 km** a se vzdáleností slábnou až do 10 km.
  Postih zmenšuje dovednost Long-Range Targeting Sensors.
- Obecně: blíž (≤ 5 km) znamená víc poškození.

### 10.2 Boční salva [KOMUNITA Ship weapon 2022]

- Přední a zadní paprsková pole (250°) se na obou bocích překrývají asi v pásmu
  70°.
- Loď s paprsky vpředu i vzadu proto nejvíc poškodí cíl, který má „po boku“,
  a často kolem něj krouží.
- Pomalé křižníky s až 8 zbraněmi jsou na boční salvu stavěné. Plná salva osmi
  paprsků je zničující, ale vyčerpá energii.

### 10.3 Palebné cykly [KOMUNITA Ship weapon 2022]

Všechny energetické zbraně mají 5sekundový cyklus:

| Zbraň | Střelba v cyklu | Dobití | Účinnost (DPS / základ) |
| --- | --- | --- | --- |
| Paprsek | 4 výstřely, 1 za sekundu | 1 s | 80 % |
| Paprsek s Fire at Will | 5 výstřelů | – | – |
| Paprsek s Overload | 1 velký výstřel | potom normální cyklus | – |
| Lehké dělo | 3 salvy po 2 výstřelech za sekundu | 2 s | 120 % |
| Těžké dělo | 2 salvy po 2 výstřelech | 3 s | 80 % |

- Rapid Fire zvýší počet výstřelů děl.
- **Základní poškození** (nezávislé na úrovni předmětu):

  | Zbraň | Poškození | DPS |
  | --- | --- | --- |
  | Paprsek | 200 | 160 |
  | DBB | 260 | 208 |
  | Věž | 100 | 120 |
  | Dělo | 160 | 192 |
  | DC | 192 | 230,4 |
  | DHC | 288 | 230,4 |

- **Torpéda:** každý typ má vlastní nabití (photon 6 s, quantum 8 s, plasma 8 s,
  transphasic 10 s, chroniton 10 s, tricobalt 30 s). Navíc mají všechny
  torpédomety sdílenou prodlevu 2 s, takže nevypálí naráz, ale postupně.
- **Historie:** v Season 13 vývojáři změnili cyklus děl (dřív 3 s) a za základ
  vzali 100 energie zbraní. Proto starší texty uvádějí, že při 50 má zbraň
  plné poškození.
- **Prodlevy podle vývojářů [OFICIÁLNÍ 1059700, 2009]:** paprsky a děla nemají
  prakticky žádnou prodlevu. Skutečnou brzdou je energie. Prodlevu mají torpéda.

### 10.4 Torpéda a miny [KOMUNITA Damage type (space) 2023; Mine Launcher 2022]

- Torpédo je samonaváděcí výbušnina s kinetickým poškozením a vedlejším efektem
  podle typu:

  | Typ | Vlastnost |
  | --- | --- |
  | Photon | nejčastější palba |
  | Quantum | víc poškození na salvu |
  | Plasma | poškození ohněm; těžká plazmová torpéda jdou sestřelit |
  | Transphasic | částečně obchází štíty |
  | Chroniton | zpomalí cíl |
  | Tricobalt | vyřadí cíl; vždy jde sestřelit |

- **Sestřelitelná torpéda:** od roku 2017 letí rychleji a gravitační jámy na
  ně nepůsobí. Neexplodují tak hned po výstřelu, ale pořád se dají sestřelit.
  **[OFICIÁLNÍ 10426883]**
- **Miny:** stojí, dokud se nepřítel nepřiblíží na 2 km (zlepšeně až 4 km),
  pak ho pronásledují a vybuchnou. Minomety sdílejí prodlevu, takže nejde položit
  „koberec“ min.
- Některé zbraně dělají kulový plošný zásah (až 10 km průměr).

### 10.5 Režimy palby (schopnosti taktických důstojníků) [KOMUNITA; OFICIÁLNÍ 10426883]

| Schopnost | Účinek |
| --- | --- |
| Beams: Fire at Will | paprsky střílí na více cílů. Od 2017 lehce snižuje poškození a přesnost. |
| Beams: Overload | jeden silný výstřel. Od 2017 není vždy kritický, ale posílí následující paprsky. |
| Cannon: Rapid Fire | víc výstřelů |
| Cannon: Scatter Volley | plošně, úhel 45° se rozšíří na 90° |
| Torpedo: High Yield | silné torpédo (může být sestřelitelné) |
| Torpedo: Spread | vějíř torpéd |

### 10.6 Automatická palba [KOMUNITA Ship weapon 2022]

- Každá zbraň může střílet automaticky.
- Pořadí, v jakém hráč automatiku zapne, určuje prioritu při sdílených
  prodlevách, například u torpéd.

### 10.7 [ODVOZENO] Pro nás

- Palebné úhly a montáž vpředu či vzadu dávají lodím charakter bez dalších
  pravidel. Kandidát na jádro boje.
- 5s cyklus s pevným rytmem střel je čitelný a dobře se vyvažuje. Rytmus
  výstřelů navíc sedí na zvukové a vizuální efekty.
- Slábnutí poškození se vzdáleností vede hráče k manévrování a přibližování.
- Barva podle druhu energie je okamžitá čitelnost frakce. Barvy navrhneme
  vlastní, ale princip „barva = druh zbraně“ převezmeme.

---

## 11. Zásah, obrana a kritické zásahy [KOMUNITA Accuracy 2022, Critical Hit 2022]

- **Šance zásahu:**
  - Když je přesnost útočníka ≥ obrana cíle, šance je 100 %.
  - Jinak = 100 / (100 + obrana − přesnost).
  - Nikdy neklesne pod 25 %.
  - Přebytečná přesnost se mění v malý bonus ke kritickým zásahům.
- **Obranu** zvyšuje hlavně pohyb (rychlost) a schopnosti jako Evasive Maneuvers.
  Maskování ji zvyšuje taky.
- **Kritický zásah:** základní šance 2,5 %, síla +50 % (tedy 150 % poškození).
  Zvyšují ho dovednosti, úpravy zbraní a antiproton.
- **[ODVOZENO]** Pro hru jednoho hráče stačí jednodušší model: obrana roste
  s pohybem lodi, šance zásahu má spodní hranici a kritické zásahy jsou vzácné
  a čitelné. Vzorce STO jsou dobrá výchozí reference pro ladění.
---

## 12. Schopnosti: důstojníci na můstku, kapitáni a progrese

### 12.1 Důstojníci na můstku (bridge officers) [OFICIÁLNÍ 1059700; KOMUNITA Bridge officer 2023]

- **Každá loď má „zasedací pořádek“:** daný počet křesel podle profese
  (taktika, strojovna, věda, univerzální) a hodnosti (praporčík až komandér).
  - Eskorty mají víc taktických křesel, vědecké lodě víc vědeckých a křižníky
    víc inženýrských.
- **Hodnost křesla omezuje schopnosti.** Důstojník použije jen tolik schopností,
  kolik dovolí hodnost křesla (praporčík 1, poručík 2, nadporučík 3,
  komandér 4). Ani komandér v poručickém křesle nepoužije víc.
  Vývojáři to v roce 2009 vysvětlili tak, že loď je postavená pro určitý účel:
  Prometheus je stavěný na palbu, ne na vědecký výzkum.
- **Tier 6 přidává specializace křesel**, například inženýr/zpravodajec. Takový
  důstojník smí použít i schopnosti specializace.
- **Duplicitní schopnosti [OFICIÁLNÍ 1059700]:**
  - Každá schopnost má dlouhou vlastní prodlevu a kratší sdílenou prodlevu
    se stejnými schopnostmi.
  - Druhý důstojník se stejnou schopností jde ve vývojářském příkladu ze
    2009 do 20s sdílené prodlevy.
  - Víc důstojníků se stejnou schopností ji tak umožní použít častěji, ale
    s klesajícím přínosem.
- **Úrovně schopností:** I–III. Získávají se z výcvikových příruček a kapitán
  je může vyrábět podle svých dovedností.

**Přehled vesmírných schopností** (jen názvy a účel; podrobnosti jsou na
komunitní wiki, snímek 2023):

| Profese | Útok | Obrana a opravy | Kontrola a oslabení | Energie a ostatní |
| --- | --- | --- | --- | --- |
| Taktika | Fire at Will, Overload, Rapid Fire, Scatter Volley, High Yield, Spread, Attack Pattern Beta/Omega, Focused Assault, Kemocite | Tactical Team (automatické přelévání štítů) | Cílení podsystémů ×4, Attack Pattern Delta (ochrana spojence) | rozsévání min (Dispersal Pattern) |
| Strojovna | Directed Energy Modulation (průnik štítem), Aceton Beam | Engineering Team, Auxiliary to Structural, Reverse Shield Polarity, Extend Shields | Eject Warp Plasma (oblak za lodí), Boarding Party | Emergency Power to W/S/E/A, Auxiliary to Battery, Auxiliary to Dampeners |
| Věda | Subspace Vortex, Photonic Shockwave, Feedback Pulse | Science Team, Hazard Emitters, Transfer Shield Strength, Polarize Hull | Tractor Beam, Repulsors, Gravity Well, Tyken's Rift, Jam Targeting Sensors, Scramble Sensors, Viral Matrix, Tachyon Beam, Energy Siphon, Charged Particle Burst (odhalí maskování) | Mask Energy Signature, Photonic Officer |
| Velení (Command) | Concentrate Firepower, Call Emergency Artillery | Rally Point, Needs of the Many, Subspace Interception | Suppression Barrage | Reroute Power, Ambush Point, Phalanx |
| Miracle Worker | Mixed Armaments Synergy, Exceed Rated Limits | Align Shield Frequencies, Reroute Shields to Hull | Null Pointer Flood, Overwhelm Power Regulators, Gravitic Induction Platform | Narrow Sensor Bands |

Další specializace (zpravodajská, pilotní, temporální) mají vlastní sady.

**Příklady parametrů** [KOMUNITA, stránky schopností, snímky 2023]:

- **Tractor Beam:**
  - zpomalí a drží cíl, který se pak nemůže maskovat,
  - dosah 5 km, aktivace 0,5 s,
  - síla roste s pomocnou energií,
  - sdílí 15s prodlevu s Repulsors.
- **Gravity Well:**
  - anomálie, která stahuje nepřátele do středu a drtí je kinetickým
    poškozením,
  - čím blíž středu, tím silnější tah,
  - dosah 10 km, úhel 135°.
- **Brace for Impact** (všichni kapitáni):
  - krátce výrazně zvýší odolnost proti kinetice a torpédům a postupně léčí,
  - doporučená chvíle: těsně před dopadem torpéda nebo výbuchem jádra
    blízké lodi.

**Ochrana proti řetězení kontroly [OFICIÁLNÍ 10426883, 2017]:** po skončení
efektu „držení“ nebo „vyřazení“ dostane cíl krátkou odolnost proti dalšímu.
Opakované zvedání zničeného podsystému (Hot Restart) je omezeno na jednou
za 60 s.

### 12.2 Kapitánské kariéry [KOMUNITA Career path 2025]

- Tři kariéry: Taktik, Inženýr a Vědec. Kapitán může řídit jakoukoli loď.
- Každá kariéra má 5 vesmírných schopností:

  | Kariéra | Role | Typické schopnosti |
  | --- | --- | --- |
  | Taktik | poškození | Attack Pattern Alpha, Fire on my Mark, Tactical Initiative, Go Down Fighting |
  | Inženýr | odolnost | Rotate Shield Frequency, Miraculous Repairs, EPS Power Transfer, Nadion Inversion |
  | Vědec | podpora a oslabení | Sensor Scan, Subnucleonic Beam / Deflector Overcharge, Scattering Field, Photonic Fleet |

- Společné schopnosti všech kapitánů: Evasive Maneuvers, Brace for Impact,
  Abandon Ship (sebedestrukce s výbuchem) a další.
- Vyvažování 2017 posílilo inženýry a vědce **[OFICIÁLNÍ 10426883]**:
  - kariéry nemají být nerovné,
  - schopnost vědce Deflector Overcharge posílí léčení, exotické poškození
    a kontrolu.

### 12.3 Dovednosti a progrese [OFICIÁLNÍ 1059220, 2011; 9797693, 2016]

**Revize 2011** ukazuje tři chyby, kterým se vyhnout:

- Dovednosti vázané na konkrétní loď odrazovaly od střídání lodí.
- Dovednosti pro konkrétní typ zbraně odrazovaly od rozmanitosti.
- Hráči nechápali, co dovednosti dělají.
- Řešení: obecné dovednosti platné na všechny lodě (body trupu, body štítu,
  rychlost a otáčení…).

**Přestavba 2016** měla tři zásady:

1. zjednodušit a objasnit (lepší popisky),
2. odstranit volby, které jsou vždy horší, a zvednout minimální úroveň,
3. hráč nesmí nic ztratit.

Výsledek přestavby:

- 405 stupňů dovedností se zmenšilo na 110 a každý stojí 1 bod.
- Vesmírné a pozemní dovednosti jsou oddělené.
- Pasivní odměny se odemykají podle investice do kategorie.
- Za 24 bodů v jedné profesi se odemkne „ultimátní“ schopnost.

**Mistrovství lodi (Starship Mastery) [KOMUNITA]:**

- Loď získává úrovně používáním v boji.
- Úrovně 1–4 dávají pasivní bonus podle typu lodi.
- Úroveň 5 u lodí Tier 6 odemkne vlastnost (trait), kterou lze použít na
  jakékoli lodi.

**[ODVOZENO] Pro nás:**

- Zasedací pořádek důstojníků je elegantní způsob, jak dát lodím odlišné
  schopnosti a hráči volbu, koho kam posadit. Hodí se i pro příběh hry jednoho
  hráče se jmenovanou posádkou.
- Ponaučení z progrese STO: žádné „pasti“ ve volbách, žádné dovednosti
  vázané na jednu loď a srozumitelné popisky od začátku.

---

## 13. Typy lodí, tiery a statistiky

### 13.1 Statistický blok lodi [OFICIÁLNÍ 9782283, 9226533, 11579886; KOMUNITA Playable starship 2023]

Oficiální stránky lodí uvádějí:

- tier, frakci a potřebnou hodnost,
- **trup** (např. 49 162 na úrovni 50 a 57 000 na úrovni 60 u Star Cruiseru),
- **modifikátor štítů** (např. 1,15),
- posádku (dnes jen údaj bez herního účinku),
- počet zbraní vpředu a vzadu (např. 4/4),
- počet slotů pro zařízení (devices),
- křesla důstojníků s hodností a specializací,
- konzole (taktické, inženýrské, vědecké, univerzální),
- **základní otáčení v °/s, Impulse Modifier a Inertia**,
- bonus k energii (např. +5 všem systémům),
- vestavěné schopnosti (například velení křižníku),
- unikátní konzoli a vlastnost lodi (starship trait).

### 13.2 Sloty vybavení [KOMUNITA Playable starship 2023]

- přední a zadní zbraně, experimentální zbraň (jen některé lodě),
- deflektor, sekundární deflektor (vědecké lodě),
- štít, warp nebo singularitní jádro, impulsní motor,
- zařízení (baterie a jednorázové pomůcky),
- konzole tří profesí a univerzální,
- hangáry (nosiče).
- Sady vybavení dávají bonusy za kombinaci.

### 13.3 Tiery [OFICIÁLNÍ 1059790; KOMUNITA Playable starship 2023]

- Tier 1–4 jsou lodě pro postup úrovněmi a hráč je dostává zdarma na úrovních
  10, 20 a 30.
- Tier 5 je první loď pro konec hry. Upgrady T5-U a T5-X ji udrží
  konkurenceschopnou.
- Tier 6 se škáluje od úrovně 1. Oproti T5-U má jednu schopnost důstojníka
  navíc, specializovaná křesla a vlastnost lodi.
- **Záměr 2009:** vylepšená loď nižšího tieru se přiblíží středu vyššího,
  ale nejlepší loď vyššího tieru je lepší, protože má víc „prostoru pro růst“.
  Staré lodě si hráč nechá a může se k nim vrátit, například s menší obratnou
  lodí na proražení blokády.

### 13.4 Typy lodí a jejich role [KOMUNITA Playable starship 2023; OFICIÁLNÍ 3030983]

| Typ | Velitelské křeslo | Zbraně | Povaha | Zvláštnost |
| --- | --- | --- | --- | --- |
| Křižník | komandér strojovny | 8 (4/4) | odolný, pomalý, boční salva | velení křižníku (aury pro tým) |
| Eskorta / raptor | komandér taktiky | 7 (víc vpředu) + experimentální | nejobratnější, křehká | dvojitá děla |
| Vědecká loď | komandér vědy | 6 (3/3) | průměrný trup, velmi silné štíty, průměrné otáčení | sekundární deflektor, Sensor Analysis, cílení podsystémů |
| Nosič | podle varianty | různé | velký, pomalý | 2 hangáry, povely stíhačkám |
| Dreadnought | podle varianty | různé | velký | hangár, často kopí (spinal lance) |
| Warbird (Romulané) | různé | různé | maskování | singularitní jádro, dvojitá děla |
| Bitevní křižník | jako křižník | 8 | křižník s dvojitými děly | ztrácí Attract Fire |
| Torpédoborec / warship / juggernaut | – | 7–8 | odolnější eskorta s horší obratností | extra zařízení nebo osmá zbraň |
| Raider / Bird-of-Prey | – | – | útoky z boku, maskování | Raider Flanking |
| Pilotní lodě | – | – | uhýbací manévry dvojitým poklepáním | pilotní manévry |
| Velitelské, zpravodajské, temporální a miracle-worker lodě | – | – | specializace | vlastní vestavěné schopnosti |

**Princip vývojářů [OFICIÁLNÍ 3030983]:** Nový typ lodi se navrhuje
z „klíčových slov“. Například „transformující se vědecký torpédoborec“ začíná
od statistik vědecké lodi a přidává mód, který prohodí taktická a vědecká
křesla a zpřístupní zvláštní zbraň. Systémoví designéři sledují, které typy
vznikly nedávno, aby jich nebylo moc stejných.

### 13.5 Úprava lodi v loděnici (Ship Tailor) [KOMUNITA Playable starship 2023; OFICIÁLNÍ 9792703]

- **Díly:** talíř, trup, gondoly, pylony, sekundární pole a misijní modul
  (u některých lodí).
- **Kombinace:** díly z vlastněných variant téže rodiny jdou míchat. Kvůli
  tomu vývojáři u Odyssey rozdělili model tak, aby šly pylony gondol vyměnit.
- **Vzhled:**
  - materiál pláště (sdílené dlaždicové materiály, víc než 60),
  - okna, vzor nátěru a barvy,
  - logo flotily,
  - jméno a registrační číslo s předponou a příponou,
  - interiér můstku.
- **Vizuální sloty** přepíšou vzhled štítů, motorů a deflektoru beze změny
  statistik **[OFICIÁLNÍ 9888133]**.

---

## 14. Zvláštní mechaniky lodí

### 14.1 Velení křižníku (Cruiser Commands) [KOMUNITA 2022]

- Týmové aury: Weapon System Efficiency, Shield Frequency Modulation, Strategic
  Maneuvering a Attract Fire (přitáhne pozornost nepřátel na křižník).
- Bitevní křižníky a dreadnoughty mají jen část.
- **[ODVOZENO]** Pro nás: křižník jako „vlajková loď skupiny“, která posiluje
  své spojence.

### 14.2 Nosiče a hangáry [OFICIÁLNÍ 1058730, 10426883, 9667053; KOMUNITA Hangar pet 2022]

- Hangár se na lodi chová jako slot. Schopnost „vypustit“ pošle křídlo malých
  plavidel.
- Každý hangár drží až 2 křídla, nosič má 2 hangáry.
- Malá plavidla mohou být stíhačky, raketoplány, runabouty i fregaty.
- **Povely** jsou přepínací a nová plavidla je převezmou:
  - Attack (útok na cíl),
  - Escort (doprovod spojence),
  - Intercept (sestřelovat torpéda, miny a malá plavidla),
  - Recall (návrat a oprava).
- **Dostřel stíhaček:**
  - v roce 2012 až 12 km od nosiče, takže nosič může zůstat mimo boj
    **[OFICIÁLNÍ]**,
  - komunita v roce 2022 uvádí asi 20 km.
- Mimo boj se stíhačky řadí před loď a letí s ní.
- **Hodnosti stíhaček:** stíhačky přežívající v boji získávají hodnosti 1–5
  (maximum za 5 minut boje). Za každou hodnost dostanou opravu trupu, víc
  životů a víc poškození.
- **Vyvážení 2017:**
  - lepší AI,
  - imunita vůči jednomu torpédu za 30 s,
  - výbuchy jader na ně nepůsobí,
  - víc životů a poškození.
- Slabina stíhaček: plošné útoky.

### 14.3 Singularitní jádro (Romulané) [OFICIÁLNÍ 1012660, 2013]

- Měřič s 5 dílky se plní v boji.
  - Neplní se při maskování a mimo boj pomalu klesá.
- Síla schopnosti roste s počtem dílků, které spotřebuje. Použití vyprázdní celý
  měřič a na chvíli zablokuje plnění.
- Počet schopností odpovídá tieru lodi:
  - Plasma Shockwave,
  - Quantum Absorption,
  - Warp Shadows (návnady a maskování),
  - Singularity Jump (teleport a malá singularita),
  - Singularity Overcharge.
- **[ODVOZENO]** Dobrý vzor frakční odlišnosti: místo jiných čísel má frakce
  jinou zdrojovou mechaniku.

### 14.4 Maskování (cloak) [KOMUNITA Cloak 2023]

| Typ | Použití | Účinek |
| --- | --- | --- |
| Standardní | jen mimo bojovou pohotovost (Red Alert) | vypne zbraně a štíty, zvýší obranu, utajení roste s pomocnou energií; po odmaskování krátký bonus k poškození (přepad) |
| Bitevní | i v boji | rychlejší a obratnější loď, ale při maskování pod palbou spadnou štíty |
| Vylepšené bitevní | i v boji | střelba torpéd a min ze skrytu (loď je chvíli zaměřitelná), bonus k poškození celou dobu |

- Odhalení maskované lodi: schopnosti jako Charged Particle Burst, tažný paprsek
  maskování znemožní.
- Komunitní wiki má video maskování (přechod do neviditelnosti a zpět).

### 14.5 Rozdělení lodi (Starship Separation) [KOMUNITA 2022]

- **Galaxy:** oddělí talíř. Pohonná sekce dostane bonus ke zbraním a poškození,
  ale ztratí trup a štíty. Talíř bojuje jako spojenec.
- **Další příklady:**
  - Intrepid vypouští aeroraketoplán,
  - Odyssey odděluje „chevron“ a vypouští eskortu Aquarius,
  - Prometheus se dělí na tři části (multi-vector assault),
  - podobné mechaniky mají i lodě dalších frakcí.
- **[ODVOZENO]** Pro naši hru je to atraktivní kánonový moment. Technicky
  potřebuje model rozdělený na díly s vlastními body a AI (viz kapitola 17).

### 14.6 Cílení podsystémů [KOMUNITA Subsystem Targeting 2023; OFICIÁLNÍ 10426883]

- Na 10 s pak zásahy energetickými zbraněmi postupně vysávají energii
  vybraného systému. Volí se pomocné systémy, motory, štíty nebo zbraně.
- Mají šanci systém úplně vyřadit. Vyřazené štíty shodí všechny sektory.
- Vědecké lodě to mají vestavěné, taktici jako schopnost.
- **[ODVOZENO]** Kánonově silný moment („Zaměřte jejich motory!“) s jasným
  taktickým smyslem.

### 14.7 Další vestavěné mechaniky (jen výčet; názvy KOMUNITA, popisy účinků [NEOVĚŘENO])

- Raider Flanking: bonus při útoku z boku.
- Sensor Analysis: vědecká loď postupně zvyšuje poškození proti cíli.
- Wingmen: doprovodné lodě.
- Spinal lance: kopí přes celou délku lodi, například phaser na Galaxy
  dreadnought.
- Tactical a Siege mode: přepnutí role lodi.
- Transwarp.

### 14.8 Zbraně v čase a energie podle frakcí [ODVOZENO]

STO odlišuje frakce tím, čím střílejí (druh energie, torpéda), jak se maskují
a jaké mají zdroje energie (singularita). Pro nás je to levnější a čitelnější
než odlišovat frakce jen čísly.
---

## 15. Nepřátelé, AI a návrh střetnutí

### 15.1 Žebříček nepřátel [KOMUNITA Non-playable starship 2022; OFICIÁLNÍ 3029673]

| Třída NPC | Charakter |
| --- | --- |
| Stíhačka | nejslabší; vypouštějí ji nosiče, NPC i hráčské |
| Fregata | malá a obratná, 3–4 zbraně, málo schopností, obvykle ve skupině po třech |
| Vědecká loď | malý trup, silné štíty |
| Eskorta | malý trup, slabé štíty, rychlá a velmi obratná |
| Křižník | velký trup, střední štíty, pomalý |
| Bitevní loď | velmi silný trup a štíty, 5–6 zbraní, mnoho schopností, velmi pomalá; někdy nosič |
| Dreadnought | ještě silnější trup a štíty; na jednu loď obvykle příliš silný; někdy nosič |
| Capital / zvláštní | bossové (například Borg, zabijáci planet) |

**Zásada vývojářů [OFICIÁLNÍ 3029673, 2014]:**

- Každá vyšší třída musí být znatelně nebezpečnější. Když bitevní loď Undine
  dávala menší poškození než křižník, přidali jí zbraně a přepsali AI.
- Frakce má ve hře působit tak hrozivě jako v seriálu. Undine byli slabší
  než Borg, a proto je upravili.

### 15.2 Vzorce chování AI [OFICIÁLNÍ 3029673, 1002780, 1059700]

- **Nálety (strafing runs).** Fregaty a křižníky Undine nalétávají na cíl
  vysokou rychlostí, na pár sekund zapojí přední i zadní zbraně a pak odletí.
  Fregaty krouží na okraji boje.
- **Řetězení schopností.** Lepší AI používá schopnosti častěji a v kombinacích.
  Pomalá AI působí neschopně.
- **Povolávání posil.** Dreadnought Undine má menší vlastní palbu, ale
  povolává fregaty.
- **Léčení ve vesmíru (Voth):**
  - Umírající loď může zavolat opravnou loď.
  - Ta se odmaskuje, přiletí, obnoví trup a pak léčí štíty spojenců.
  - Hráč ji proto musí rozpoznat jako prioritní cíl.
- **Směrová imunita (Voth).** Pole kolem lodi ji chrání před poškozením
  z jednoho směru, pokročilá verze energii odráží. Hráč musí manévrovat.
- **Odběr energie** dronami nebo zbraněmi, dočasná nezranitelnost vůči energii
  a podobně.
- **Preferovaná vzdálenost.** Pravidlo z pozemního boje (2009) se hodí
  i do vesmíru: útočníci zblízka se přibližují, střelci na dálku si drží odstup.
  Různá AI preferuje různé schopnosti (léčitelé, přivolávači).
- **Kánon upravený pro zábavu [OFICIÁLNÍ 1002780].** Voth v seriálu umějí
  vyřadit loď „kdykoli“. Ve hře by to nebylo zábavné, takže to vývojáři pojali
  jako „kontrolu nad energií a časoprostorem“.

### 15.3 Další pravidla [OFICIÁLNÍ 10426883; KOMUNITA]

- Po skončení efektu držení nebo vyřazení dostane cíl krátkou odolnost proti
  dalšímu, aby kontrola nešla řetězit donekonečna.
- NPC si při střelbě neubírají energii zbraní. Zjednodušení pro AI.
- Obtížnosti Normal, Advanced a Elite mění sílu nepřátel a odměny (viz 4.3).
- Obtížnost v bojové zóně se přizpůsobuje počtu hráčů (viz 4.3).

### 15.4 [ODVOZENO] Pro nás

- Pro první bojový prototyp stačí dva nepřátelé:
  - fregata s nálety ve skupině,
  - křižník s boční palbou a jednou schopností.
- Nepřítel s jasnou slabinou nutí hráče manévrovat a volit priority: léčitel,
  směrový štít, povolávač.
- Žebříček tříd s výrazným rozdílem hrozby usnadní tvorbu misí z dat
  (třída → statistiky → chování).

---

## 16. Zničení lodí [KOMUNITA Warp speed 2023; OFICIÁLNÍ 1059740, 10426883]

- **Výbuch warp jádra:**
  - nastane asi 5 s po ztrátě celého trupu,
  - poškodí lodě v okruhu několika kilometrů,
  - velikost a síla výbuchu závisí na lodi.
- **Jádro hmota/antihmota:** po smrtelném zásahu přijdou vnitřní výbuchy
  a pak jasný záblesk.
- **Singularitní jádro:**
  - loď se zhroutí do dočasné černé díry,
  - ta přitahuje okolní lodě,
  - po pár sekundách exploduje s odhozením.
  - Poškození je stejné jako u výbuchu antihmoty.
- **Ochrana před výbuchem:** Brace for Impact, nebo odtlačení umírající lodi
  odpuzovacím paprskem (záměr 2009).
- **Sebedestrukce (Abandon Ship).** Silnější výbuch, podle výsledku vyhledávání
  asi 5 000 kinetického poškození v okruhu 1 km plus výbuch jádra
  **[NEOVĚŘENO, souhrn z vyhledávání]**.
- Stíhačky nosičů výbuchy jader od roku 2017 nepoškozují **[OFICIÁLNÍ]**.
- Rada hráčům: vybuchující loď poškozuje okolí, takže je třeba odletět
  **[KOMUNITA Space combat]**.
- **[ODVOZENO]** Pro nás je výbuch jádra skvělý dramatický i taktický moment:
  pětisekundové varování, viditelná řetězová exploze a plošné poškození.
  V prototypu Three.js už existuje rozpad lodí na úlomky a pravděpodobnostní
  výbuch reaktoru. Na to lze navázat.

---

## 17. Modely lodí a jejich výroba

### 17.1 Postup výroby lodi [OFICIÁLNÍ 3030983, 10434133, 9075673; VÝVOJÁŘ Versiga 2021, Marrone 2020/2026]

1. **Rozhodnutí.** Vedení určí, že loď je potřeba, i 6 a víc měsíců předem.
   Ohlíží se, jaké typy lodí vznikly nedávno.
2. **Parametry.** Kdo loď staví, k čemu slouží, jaké má schopnosti, velikost
   a rodinu, do které patří.
3. **Koncept:**
   - stylové prvky frakce (u Federace talíř a gondoly),
   - schopnosti lodi se musí promítnout do tvaru: eskorta má útočný „postoj“
     vpřed, vědecká loď výrazný deflektor,
   - jako vzor stylu, velikosti a „postoje“ slouží kánonová loď,
   - mnoho rychlých náčrtků siluet, ze kterých se vyberou a zkombinují nejlepší.
4. **3D blokování (koncept).** Hrubý 3D model v 3ds Max pro proporce a „povahu“
   lodi. Přes něj se kreslí ve Photoshopu, což šetří ortografické výkresy
   a umožní rychlé iterace. Výsledkem je barevná „předloha“ pro výtvarníka lodí.
5. **Model pro hru:**
   - počítá s variantami, animacemi, zvláštními zbraněmi a schopnostmi,
   - drží se rozpočtu paměti a polygonů pro PC i konzole,
   - tvary musí na sebe navazovat, aby loď působila věrohodně ze všech úhlů.
6. **Materiály a textury** (viz 17.2).
7. **Data ve hře:**
   - textové soubory určují díly, efekty a jejich názvy,
   - **přes 80 uzlů pro efekty na loď**: místa zbraní, motorů, světel, warpu
     a podobně,
   - určí se, které díly jdou upravovat.
8. **Statistiky a schopnosti** dodá systémový designér, efekty efektový
   výtvarník (FX), animace animátor a ikony UI.
9. **QA:**
   - standardní testovací plán lodi,
   - průzkumné testy nových schopností brzy, ještě s provizorními efekty,
   - ověřuje se, jak loď působí při ovládání a jestli plní svou roli,
   - testují i vybraní hráči.
10. **Čas:** od 1 měsíce po několik měsíců podle rozsahu. Nejdéle trvá nový
    design s animacemi a novým materiálem **[VÝVOJÁŘ Marrone 2020]**.

### 17.2 Materiály: technika STO [OFICIÁLNÍ 3030983 (2014), 10434133 (2017)]

- **Dlaždicové (opakovatelné) textury sdílené napříč loděmi.**
  - Jedna sekce textury se na modelu opakuje zrcadlená, zvětšená nebo otočená,
    takže nepůsobí jako opakování.
  - Výsledek: vysoký detail při nízkých nárocích na hardware.
  - Téměř každá loď může použít kterýkoli z víc než 60 materiálů, a z toho
    vzniká úprava vzhledu v loděnici.
- **Hlavní šablona materiálu.** Všechny lodě dodržují stejná pravidla
  rozložení, aby šly materiály zaměnit.
- **Nástroje (2017):** 3ds Max (modely a panely), ZBrush (organické pláty,
  například šupinatý pancíř Tzenkethi), Substance Designer (skládání materiálu).
- **Zdrojové mapy:** okolní stínění (AO), ID materiálů, normálová mapa
  a pomocné mapy (cavity, height).
- **Výstupní textury (4):**
  - difuzní (v alfa kanálu odrazivost),
  - normálová,
  - svítivost (okna a světla),
  - barevná specularita (v alfa kanálu lesk).
  Rozdělení usnadňuje pozdější přebarvení a úpravy.
- **Okna, nápisy a světla** jsou na samostatném listu, který se mapuje na
  „plovoucí geometrii“ těsně nad povrchem lodi. Proto jdou registrace,
  okna i barvy nezávisle měnit.
- **Rozdělení na díly.** Variantám se dělají vyměnitelné díly. Starší model se
  kvůli tomu přestavěl (u Odyssey se oddělily pylony). Lodě bez vyměnitelných
  dílů se staví jako jeden kus, což je jednodušší (lodě 26. století).

### 17.3 Tvarový jazyk a design [OFICIÁLNÍ 9245593, 7005503, 9075673, 10389143, 10434133, 10038053, 9792703]

- **Funkce určuje tvar.** Inspiruje se přírodou, vozidly, letadly i loděmi
  **[VÝVOJÁŘ Marrone 2026]**:
  - pilotní lodě: letadla (SR-71, šípová křídla dopředu),
  - zpravodajské lodě: stealth stíhačky a nízké závodní vozy,
  - Tzenkethi: želvy a krokodýli.
- **Zpravodajské lodě** mají jednotný „vizuální jazyk“ pro všechny frakce
  a velikosti. Ze schopností vyvodili vlastnosti:
  - kradmé: ostré hrany, tmavé materiály se vzorem,
  - obratné: šípovité zužující se linie,
  - agresivní: čepelovité tvary, málo oken, viditelné zbraně.
- **Velitelské lodě** dostaly pravidla napříč frakcemi:
  - zřetelný rotující disk,
  - 4 gondoly, které působí jako 2 rozdělené,
  - u Federace druhý trup mezi krkem a talířem.
- **Variace rodiny.** Varianty zachovají siluetu rodiny a mění klíčové prvky
  (pylony, gondoly, impulsní motory), aby ukázaly vývoj technologie. Proporce
  si můžou vypůjčit z jiných tříd (Sojourner od Galaxy). Design odráží
  schopnosti: dok pro eskortu, motory pro oddělení.
- **Měřítko a díly v rodině** musí sedět (Marrone 2026). Vedle sebe musí lodě
  působit konzistentně.
- **Kánonová věrohodnost.** Lodě mají vypadat přirozeně vedle fyzických modelů
  ILM z 90. let. Vychází se i z nepoužitých kánonových návrhů (Sternbach
  pro Pathfinder).
- **Siluetu frakce nesmí ztratit.** Klingonská pilotní loď v jednom náčrtku
  vypadala „moc romulansky“, a tak ji zahodili. Federační lodě potřebovaly
  nejvíc iterací, aby zůstaly rozpoznatelně federační.
- **Mimozemské frakce.** Tvar lodí navazuje na postavy druhu. Tzenkethi mají
  uzavřené, tupé tvary jako skořápky a grafické dekorace navíc.

### 17.4 Kvalita v čase [VÝVOJÁŘ Marrone 2020, 2025; Versiga 2021]

- Nové modely jsou 4–5× detailnější než před deseti lety.
- Staré lodě se remasterují. Při remasteru lodí z dřívějších her se zachovává
  i to, jak se chovaly.
- Enterprise-F se kvůli Picardovi přestavovala od nuly (3 týdny intenzivní
  práce).
- Zdokonalení starého modelu má být nenápadné a nesmí změnit charakteristické
  rysy (Odyssey 2016).

### 17.5 Stanice a velké objekty [OFICIÁLNÍ 3041603, 10878734, 11572869]

- **Whitebox.** Prostor se nejdřív postaví z jednoduchých bloků bez textur
  s pomocnou mřížkou pro měřítko. Dokud nemá rekvizity, velikosti se špatně
  odhadují.
- **Kánonová rekonstrukce DS9:**
  - plány a rozměry ateliéru Stage 17,
  - víc než 2 600 snímků obrazovky ze seriálu,
  - chybějící části se dotvořily ve stylu seriálu.
- **Earth Spacedock:**
  - Při vydání měl vlastní nekánonový design.
  - Hráči v roce 2011 prosadili kánonový vzhled (Star Trek III / TNG).
  - V roce 2025 přišla nová podoba podle Picarda.
  - Poučení: kánonová věrnost ikonických míst je pro fanoušky důležitá.

### 17.6 [ODVOZENO] Pro náš postup v Blenderu a Unrealu

| Princip STO | Naše obdoba |
| --- | --- |
| Hlavní šablona materiálu, sdílené dlaždicové materiály | Sada sdílených materiálů v Unrealu (master material + instance), v Blenderu jednotné pojmenování slotů materiálu |
| Okna, nápisy a světla na samostatné vrstvě | Samostatná mesh vrstva nebo decaly. Registrace a jméno lodi jako data, ne textura. |
| Přes 80 uzlů pro efekty | Pojmenované sockety nebo empties v Blenderu (zbraně, motory, deflektor, světla, warp, výbuchy), exportované do Unrealu a evidované v datech lodi |
| Vyměnitelné díly | Rodiny lodí stavět z dílů s pevnými připojovacími body a společným měřítkem |
| Rozpočty paměti a polygonů | Definovat rozpočet a LOD pro 600 m loď. Ověřit výkonem na RTX 3070 / 16 GiB. |
| Měřítko v rodině | Tabulka délek lodí v metrech (kánon nebo vlastní) jako jediný zdroj pravdy |
| Funkce určuje tvar, vizuální jazyk frakce | Krátký stylový list pro každou frakci (tvary, barvy, světla, okna) |

Vlastní lodě STO (Odyssey, Jupiter…) jsou návrhy STO. Nepřebíráme je.

---

## 18. Vizuální efekty a animace

### 18.1 Zásady [OFICIÁLNÍ 1055330 (2009), 3030983 (2014)]

- **Modernizovat, ale zachovat poznatelnost.** Efekty z TOS měly malý rozpočet.
  Hra je musí udělat lépe, ale tak, aby je fanoušek okamžitě poznal (vedoucí
  efektového týmu, 2009).
- **Čitelný vizuální jazyk.** Každý mód a každá schopnost musí mít svůj
  rozpoznatelný vzhled, aby hráč mohl reagovat. Inspirace je v seriálech.
  Cílem je, aby se po stisku tlačítka stalo „něco skvělého“ (efektový
  výtvarník, 2014).

### 18.2 Přehled efektů a animací ze zdrojů

| Prvek | Co víme | Zdroj |
| --- | --- | --- |
| Paprsky a střely | barva podle druhu energie (kapitola 9) | KOMUNITA |
| Torpéda | samonaváděcí střely; některá jdou sestřelit | KOMUNITA |
| Štíty | vzhled je ze štítového vybavení nebo z vizuálního slotu; kosmetické štíty | OFICIÁLNÍ |
| Motory a deflektor | vzhled je z vybavení nebo z vizuálního slotu | OFICIÁLNÍ |
| Warp | efekt vstupu a výstupu podle frakce; zvláštní animace lodí (Intrepid zvedá gondoly), vlastní animace transwarpu | KOMUNITA |
| Maskování | animace přechodu do a z neviditelnosti (video na wiki) | KOMUNITA |
| Oddělení lodi | animace oddělení talíře, MVAM a dalších (video na wiki) | KOMUNITA |
| Výbuch jádra | vnitřní exploze a záblesk; singularitní kolaps a černá díra | KOMUNITA |
| Transformace lodi | módy lodí potřebují vlastní animace, efekty a schopnosti | OFICIÁLNÍ |
| Náklon při zatáčení | automatický, hráč ho neovládá | KOMUNITA |
| Světla a okna | svítivostní mapa a list oken a světel | OFICIÁLNÍ |
| Kamera | zámek cíle drží obě lodě v záběru (ovladač) | OFICIÁLNÍ |

### 18.3 Nepodařilo se doložit [NEOVĚŘENO]

- Jak přesně vypadá zásah do štítu (bublina, rozvlnění, barva podle sektoru).
- Vizuální stavy poškození trupu (ohně, úniky plazmy, spáleniny).
- Animace startu stíhaček z hangáru.
- Prostředí soustav: hvězdná obloha, planety v pozadí, velikost a hranice map.

Tyhle věci je nutné zjistit přímým pozorováním (kapitola 21). Prototyp Three.js
už má vlastní řešení některých z nich (štít podle vrcholů modelu, úlomky
a výbuchy).

### 18.4 [ODVOZENO] Pro nás

- Každému druhu zbraně dát jasnou barvu a rytmus, sladěný se zvukem.
- Štít zobrazovat hlavně při zásahu a podle sektoru, aby hráč viděl, kde ho
  zasáhli.
- Události (warp, maskování, oddělení, výbuch jádra) jsou „momenty“, které
  stojí za vlastní animace. Plánovat je už v modelu (sockety, díly).
- Efekty oddělit od statistik, aby šel vzhled měnit bez zásahu do pravidel.
  STO to zavedlo až dodatečně (2016).

---

## 19. Principy designu a vyvažování podle vývojářů STO

| Princip | Odkud | Co to znamená pro nás [ODVOZENO] |
| --- | --- | --- |
| Hra má být hlavně zábavná; hráčova „zábava“ nesmí být „špatně“ | OFICIÁLNÍ 10426883 (2017) | Netrestat styly hry |
| Investice hráče si udrží hodnotu | 10426883 | Změny nesmí zničit uložené lodě a postupy |
| Žádná volba nesmí být vždy správná ani vždy špatná | 10426883, 9797693 | Testovat volby, odstraňovat pasti |
| Zjednodušit a vysvětlit; lepší popisky | 9797693 (2016) | Začátečník musí rozumět |
| Dovednosti nevázat na jednu loď ani typ zbraně | 1059220 (2011) | Volnost střídat lodě |
| Kánonovou schopnost upravit, pokud není zábavná | 1002780 (2013) | Kánon je předloha, ne zákon |
| Hráč musí cítit postup (přepočet warpu) | 1011770 (2013) | Pokrok musí být viditelný |
| Po vypršení omezujícího efektu krátká odolnost proti dalšímu | 10426883 | Žádná nekonečná kontrola |
| Nový typ lodi z klíčových slov a s ohledem na existující typy | 3030983 | Plánovat portfolio lodí |
| Rychle prototypovat ovládání, testovat a automatizovat | 10010203 (2016) | Hráč má hrát hru, ne UI |
| Testovat „pocit“ lodi brzy, s provizorními efekty | 3030983 | Prototyp letu a boje před grafikou |
| Kánonová věrnost ikonických míst | 11572869, 10878734, 9078783 | Sol, Země, Spacedock a DS9 věrně podle kánonu |
| Mechanika, která nepřidává zábavu, může odejít (posádka 2015) | KOMUNITA | Odvaha škrtat |
| Rozpočty výkonu od začátku | Versiga 2021 | Měřit výkon průběžně |

---

## 20. Co převzít pro Daedalus

Doporučení podle fáze. Jsou to podněty pro rozhodnutí uživatele a Codexu,
ne hotové rozhodnutí. **[ODVOZENO]**

### 20.1 Teď: sluneční letová laboratoř (úkoly 0004 a 0006)

- Let jako u velké lodi: 6 °/s, reakce kolem 0,5 s, mírné klouzání,
  automatický náklon, kopání omezené na 75–80°. Podrobně v úkolu 0005.
- Nižší rychlost otáčení při velmi malém plynu, plná od 25 %.
- Kamera: vodorovný horizont, Home vrací kameru za loď, myš ovládá kameru,
  ne loď.
- HUD: plyn včetně couvání, rychlost, kurz a náklon.
- Volitelně plný impuls jako cestovní režim s cenou.

### 20.2 Další krok: první bojový prototyp

1. Štíty se 4 sektory, 10% prosakováním, regenerací v taktech a přeléváním.
2. Energie a kinetika: energie působí na štít i trup stejně, kinetika má proti
   štítu jen 25 %.
3. Tři zbraně s úhly: paprsek (250°, vpředu i vzadu), dělo (45–90°, vpředu)
   a torpédo (90°, s prodlevou). Pevný 5s cyklus. Poškození klesá se
   vzdáleností.
4. Energie ve 3–4 systémech s předvolbami a postupným přeléváním. Střelba
   ubírá energii zbraní.
5. Obrana z pohybu a jednoduchá šance zásahu s dolní hranicí.
6. Dva nepřátelé: fregaty s nálety a křižník s boční palbou.
7. Výbuch jádra s 5s varováním a plošným poškozením.
8. Měřítko boje: STO bojuje na 2–10 km s loďmi o délce zhruba stovek metrů až
   kilometru, tedy na desítky délek lodi. Pro 600 m Daedalus odpovídá dostřel
   asi 6–15 km. Hodnoty jsou k vyladění v testech.

### 20.3 Později

- Důstojníci na můstku se zasedacím pořádkem lodi a sdílenými prodlevami.
- Kapitánská kariéra nebo role.
- Typy lodí s rolemi (křižník, eskorta, vědecká loď, nosič) a vestavěnými
  mechanikami.
- Nosiče a stíhačky s povely.
- Maskování pro frakce Klingonů a Romulanů.
- Oddělení talíře jako kánonový moment.
- Cílení podsystémů.
- Mapa sektoru s warpem, náhodnými setkáními, hlídkami a „živou frontou“.
- Úprava lodi (díly, materiály, registrace) a vizuální sloty.
- Ovládání na ovladači (radiální menu, zámek cíle, volitelná automatizace).

### 20.4 Výroba assetů

- Sdílené materiály a stylové listy frakcí.
- Díly lodí se socket body pro efekty.
- Tabulka měřítek lodí v metrech.
- Rozpočty výkonu a LOD.
- Whitebox pro velké stanice.
- Kánonové podklady pro ikonická místa (Země, Spacedock).

### 20.5 Co nepřebírat

- Monetizaci MMO: lockboxy, prémiové lodě, pay-to-win.
- Inflaci síly a desítky vzácností a úrovní předmětů (Mk I–XV, Common–Epic).
  Pro hru jednoho hráče stačí jednodušší vybavení.
- Desítky aktivních schopností naráz. STO používá skoro 72 kláves. My chceme
  menší a hlubší sadu, srozumitelnou pro začátečníka.
- Honbu za poškozením za sekundu jako měřítko obtížnosti (Elite 30–50 k DPS).
- Vlastní lodě, texty a assety STO. Kánonové lodě Star Treku zůstávají fikční
  značkou CBS/Paramount. Platí dosavadní opatrnost s licencemi
  (README a ENVIRONMENT).

---

## 21. Otevřené otázky a co ověřit přímým pozorováním

Tyhle údaje veřejné texty nedávají. Nejlépe je zjistit nahrávkou hry STO
(60 FPS) s poznamenanými časy, nebo měřením na kvalitním videu.

| Otázka | Proč je důležitá |
| --- | --- |
| Úhel a rychlost automatického náklonu při zatáčení, rychlost vyrovnání | vizuální pocit letu |
| Vyrovnává se kopání samo po puštění W/S? Jaký je skutečný limit kopání? | ovládání a kamera |
| Časy zrychlení a zastavení u velké lodi, skutečné klouzání | ladění inertie |
| Rychlost plného impulsu a warpu ve viditelných jednotkách | měřítko vesmíru |
| Velikost map soustav, hranice, vzdálenosti planet | návrh soustavy Slunce–Země |
| Vzhled zásahu štítu (tvar, rozvlnění, barva, sektor) | efekt štítu |
| Vizuální stavy poškození trupu | čitelnost stavu lodi |
| Vzdálenosti a úhly kamery, chování kamery v zámku cíle | kamera |
| Zvuky zbraní a lodí | atmosféra (ve zdrojích chybí) |

Dále je vhodné:

- přečíst živou wiki stowiki.net, pokud k ní uživatel získá přístup
  (nové stránky po roce 2023),
- projít oficiální patch notes Season 13 (velké vyvážení),
- přečíst vývojářské blogy, které jsem jen indexoval. Rejstřík 6 026 článků
  lze znovu stáhnout přes API (kapitola 2.1).

---

## 22. Poznámky k rozsahu a postupu

- Přečtené zdroje:
  - přes 30 oficiálních článků (2008–2026) z rejstříku 6 026 článků,
  - 6 rozhovorů s vývojáři,
  - asi 35 archivních stránek komunitní wiki,
  - několik hráčských diskusí.
- Unreal, Blender, kompilaci ani hru jsem nespouštěl. Codex mezitím na stejném
  počítači pracuje na letové scéně.
- Dokument je samostatná rešerše na vlastní větvi. Neměnil jsem INDEX, zadání
  ani společné soubory. Pokud ji Codex chce vést jako číslovaný úkol, může jí
  přidělit číslo.
- Žádný text, obrázek ani model STO jsem do repozitáře nekopíroval. Citace jsou
  krátké a ostatní je převyprávěné.

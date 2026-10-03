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


> ROZPRACOVÁNO: kapitoly 7–22 budou doplněny v dalším checkpointu.

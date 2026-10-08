# HUD — přístroje podle STO a lokální minimapa

## Aktuální implementace0024

Uživatel schválil vzhled a autorizoval zasazení do hry, sjednocení velikostí
spodních přístrojů a zprovoznění minimapy. C++ implementace je nyní v
Solar/SolarHUD.cpp; původní DrawHUD byl vyjmut ze SolarFlightGameMode.cpp.
Jde o jednu implementaci. Spodní panely mají společnou výšku208 jednotek
v referenčním1920×1080 layoutu, všechny zarovnané na společný spodní okraj.
Celá sestava má šířku868 jednotek (1736px při3840×2160); písmo je rasterizované
v cílovém rozlišení, typicky24–32px při4K. Obrys lodi se nedeformuje; zachovává
měřený poměr stran. Žádné nové HUD obrázky/importované assety nejsou potřeba.

Minimapa je průhledný zelený půdorys aktivní soustavy, centrovaný na
kanonickou polohu lodi v metrech, přídí nahoru podle skutečného směru.
Pohyb kamery, sklon a náklon lodi její měřítko/otočení neovlivňují. Tři kruhy
mají1/3,2/3 a plný aktuální dosah; stupnice1/2/5 se automaticky přizpůsobuje
nejbližšímu fyzickému tělesu. Měřítko se vztahuje k roviněXY; značka+ / −
označuje výraznou výšku vůči lodi, údaj o vzdálenosti je skutečný ve3D.
Polohy se čtou z katalogu; jde o jeho současný statický epochový layout,
nikoli nově implementované astronomické oběžné dráhy. Neznámé polohy vynechává.
Vybrané těleso má jantarovou značku a může být ukázáno směrovou značkou na
okraji, pokud je mimo dosah. Ostatní body jsou skutečně uvnitř dosahu.
Počet drobných bodů je omezen na32 nejbližších, vybraný cíl se zachová.

Rychlost, tah (včetně záporného), aktuální letový režim, již existující
navigační údaje, pauza, ochrana povrchu a chyby se dál čtou ze skutečného stavu.
Energie, síla štítů a zbraně mají prázdné hodnoty; výzbroj ukazuje modelové
zóny. Počítačové sekce nejsou nové funkční akce. Letová pravidla, klávesy,
galaxijní mapa a kanonická data zůstaly beze změny. HUD nic z toho nemutuje.

Radarová projekce je oddělená v SolarHUDRadar.h/.cpp. Test
Daedalus.HUD.RadarCoordinates pokrývá velké lokální souřadnice, směr/otočku,
skutečnou vzdálenost oproti oříznutí na okraj, výšku a neplatné vstupy.
Tools/Prepare-HudOutline.py teď generuje i SolarHUDShipData.inl ze stejného
zdrojového modelu a bodů zbraní jako návrh. Dosavadní geometrie/asset se nemění.

**Stav ověření:** dokončeno v integrační verzi0025 po uvolnění aplikací Claudem.
C++ build i Windows balíček prošly;20/20 testů včetně radarové projekce.
Skutečný render3840×2160 s HUD byl zkontrolován u Země i nad černým vesmírem.
Půdorys lodi má tmavou plochu pro čitelnost nad světlou planetou; minimapa
zůstává úplně průhledná. Popisky byly upraveny, aby se zbytečně nezkracovaly.
Běžný launcher nyní míří na Build-Ancient27. PlayerData a starší balíčky zachovány.
Nový počítač/zbraně/energie jsou stále nezapojené; minimapa a dosavadní letové
údaje jsou funkční. Další hodnocení vzhledu a ovládání provede uživatel.

Rozšíření 0026 přidává skutečný obrys a název Aurory, její vlastní letové údaje
a odpovídající nezapojené zbraňové popisky. P otevírá klikací nabídku s výběrem
lodí, návratem do hry, ukončením, přehledem obrazu a nedostupným uložením/
načtením. Nabídka i její hitboxy vycházejí ze stejného rozložení pro aktuální
rozlišení. Změny stavu provádí explicitní příkazy; vykreslování HUD pouze čte.
Přepínání a nabídka prošly kontrolou myší ve skutečném rozlišení 3840×2160.

Úkol 0027 přidává alternativní antický vzhled pro Auroru. Volí se přes
P → Nastavení → HUD: pozemská technika / HUD: antická technika. Volba se uchová
po změně lodi i restartu. Daedalus si vždy ponechá pozemský HUD. Nové přístroje
sdílejí ostré kreslení i kanonický radar; hodnoty letu se nemění. Antický vzhled
se vztahuje i na nabídku a mapu. Referenční obrazovky, použité motivy a hranice
ornamentů popisuje [ANCIENT_HUD.md](ANCIENT_HUD.md).

## Předchozí schválený návrh vzhledu

Úkol0024 nyní připravuje nový HUD podle uživatelových dvou referencí STO.
Z původního zůstává inspirace základním rozmístěním, žádné převzaté STO assety.
Bez chatu, misijního seznamu, Endeavors a online panelů. Uživatel odmítl
jednobarevné obdélníkové panely první verze a následně i příliš minimalistickou
revizi. Současný návrh má komplexnější přístrojové provedení: členité rámy,
zapuštěné displeje, propojenou spodní sestavu a podrobné skupiny výzbroje.
Kompaktní text míří na32palcový4K monitor. Minimalismus již není cílem.

- Vlevo nahoře: poloha, soustava a zelená minimapa bez jakéhokoli panelu,
  rámečku či výplně; jen tři kruhy, kříž, šipka a několik bodů.
- Dole uprostřed: zelený půdorys Daedala v kruhovém přístroji a čtyři
  zachované modré sektory štítů, názvy stran a jmenovka lodi.
- Vlevo od půdorysu: sekundární kompaktní přístroj čtyř energetických kanálů,
  segmentované dosud neaktivní stupnice a prázdné hodnoty.
- Vpravo od půdorysu: vertikální tah motoru0–100%, číselná rychlost a režim.
- Vpravo nahoře: rámovaný palubní počítač s čtyřmi sekcemi a displejem.
- Vedle pohonu: tři skupiny výzbroje, věže/railguny (měděná), rakety
  (jantarová), asgardské paprsky (modrá). Štíty mají tlumenou modrou,
  obrys trupu zelený jako minimapa a primární text neutrální šedobílou.
  Zbraňové displeje zobrazují skutečné umístění bodů na projektovaném modelu;
  raketová sila mají zvětšený pohled na příď. Žádné klávesové nápovědy/ovládací
  slider v HUD; motorový ukazatel je jen zobrazení.

Náhled `Tasks/0024/Preview/hud-concept.html` je lokální samostatný design.
Otevři v prohlížeči po GitLFS pull; žádný server ani Unreal není potřeba.
Přepínání počítače a zvýraznění zbraňových zón jsou **pouze ukázka**,
nepřipojená ke hře. Zvolená zbraň zvýrazní skutečné body podle
Art/Ships/Daedalus/WEAPON_MOUNTS.json:38věží,16sil a4paprskové emitory.
Nejde o schválené počty herních zbraní, munici, střelbu či vybalancování.
Minimapa je schematická. Energie a štíty nemají smyšlené hodnoty; ukazují
nezapojení. Budoucí herní hodnoty budou čteny z autoritativního stavu.

Půdorys je odvozený z Art/Ships/Daedalus/Daedalus.glb (Astrofossil, stejné
CC BY-NC4.0 podmínky jako zdrojový model, viz jeho manifest). Obnovit lze
pomocí Python3/Pillow: `python Tools/Prepare-HudOutline.py`.
Poté `node Tasks/0024/Preview/build-preview.cjs` znovu sestaví náhled.
Background.png je uživatelův referenční snímek vlastní hry, ne nová vesmírná
textura ani součást hry. Záběr pozadí vynechává původní spodní HUD; scénu
nezakrývá nový spodní pruh. Vektorové prvky se vykreslují nad scénou.

Prohlížečový render ověřen v3840×2160: základní text24.96px, žádné horizontální
přetékání ani oříznuté popisky pohonu/zbraní, nulový rámeček/průhledná minimapa
a funkční ukázkové interakce
všech tří zbraňových skupin s jejich skutečnými modelovými body.
Podklad není důkaz aktuální ostrosti
scény ve4K; tato kontrola ověřuje nový HUD. Menší390px náhled se přeskupí.
Tento historický prohlížečový návrh byl následně schválen a implementován
v herní verzi0025; aktuální stav a herní ověření jsou popsány nahoře.

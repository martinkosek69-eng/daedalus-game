# HUD — minimalistický návrh, ne implementovaná herní funkce

Úkol0024 nyní připravuje nový HUD podle uživatelových dvou referencí STO.
Z původního zůstává inspirace základním rozmístěním, žádné převzaté STO assety.
Bez chatu, misijního seznamu, Endeavors a online panelů. Uživatel odmítl
jednobarevné obdélníkové panely první verze. Současný návrh má střídmé
funkční barvy, otevřené prvky nad scénou a kompaktní text pro32palcový4K monitor.

- Vlevo nahoře: poloha, soustava a zelená minimapa bez jakéhokoli panelu,
  rámečku či výplně; jen tři kruhy, kříž, šipka a několik bodů.
- Dole uprostřed: půdorys Daedala a čtyři samostatné sektory štítů.
- Pod půdorysem: drobné sekundární údaje čtyř energetických kanálů.
- Vpravo od půdorysu: vertikální tah motoru0–100%, číselná rychlost a režim.
- Vpravo nahoře: navigace, skenování, informace o lodi, hyperpohon.
- Vedle pohonu: tři skupiny výzbroje, věže/railguny (měděná), rakety
  (jantarová), asgardské paprsky (modrá). Štíty mají tlumenou modrou,
  trup a primární text neutrální šedobílou. Žádné klávesové nápovědy/ovládací
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
přetékání, nulový rámeček/průhledná minimapa a funkční ukázkové interakce
všech tří zbraňových skupin s jejich skutečnými modelovými body.
Podklad není důkaz aktuální ostrosti
scény ve4K; tato kontrola ověřuje nový HUD. Menší390px náhled se přeskupí.
Herní C++/asset/import/build/package zůstaly beze změny; finální herní
napojení čeká na uživatelovo hodnocení návrhu a koordinaci s Claude0023.

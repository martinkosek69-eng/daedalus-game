# HUD — návrh vzhledu, ne implementovaná herní funkce

Úkol0024 nyní připravuje nový HUD podle uživatelových dvou referencí STO.
Z původního zůstává inspirace základním rozmístěním, žádné převzaté STO assety.
Bez chatu, misijního seznamu, Endeavors a online panelů. Světle zelený
terminálový styl; kompaktní text a linie pro32palcový4K monitor.

- Vlevo nahoře: poloha, soustava a kulatá holografická minimapa.
- Dole uprostřed: půdorys Daedala a čtyři samostatné sektory štítů.
- Vlevo od půdorysu: čtyři připravené energetické kanály.
- Vpravo od půdorysu: vertikální tah motoru0–100%, číselná rychlost a režim.
- Vpravo nahoře: navigace, skenování, informace o lodi, hyperpohon.

Náhled `Tasks/0024/Preview/hud-concept.html` je lokální samostatný design.
Otevři v prohlížeči po GitLFS pull; žádný server ani Unreal není potřeba.
Přepínání počítače a změna tahu jsou **pouze ukázka**, nepřipojená ke hře.
Minimapa je schematická. Energie a štíty nemají smyšlené hodnoty; ukazují
nezapojení. Budoucí herní hodnoty budou čteny z autoritativního stavu.

Půdorys je odvozený z Art/Ships/Daedalus/Daedalus.glb (Astrofossil, stejné
CC BY-NC4.0 podmínky jako zdrojový model, viz jeho manifest). Obnovit lze
pomocí Python3/Pillow: `python Tools/Prepare-HudOutline.py`.
Poté `node Tasks/0024/Preview/build-preview.cjs` znovu sestaví náhled.
Background.png je uživatelův referenční snímek vlastní hry, ne nová vesmírná
textura ani součást hry. Původní spodní HUD je v náhledu vynechaný výřezem.

Prohlížečový render ověřen v3840×2160: text25.6px, žádné horizontální
přetékání a funkční ukázkové interakce. Podklad není důkaz aktuální ostrosti
scény ve4K; tato kontrola ověřuje nový HUD. Menší390px náhled se přeskupí.
Herní C++/asset/import/build/package zůstaly beze změny; finální herní
napojení čeká na uživatelovo hodnocení návrhu a koordinaci s Claude0023.

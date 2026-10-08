# 0022 — Porovnání a revize Codexu

Datum: 2026-10-04. Revidovaný výstup Claude: `c2ccd5d`, checkpoint `c930517`,
větev `codex/claude-0022-planet-quality`, herní základ `906567c`.

**ACCEPTED jako nezávislá analýza a návrh, s níže uvedenými výhradami.**
Není to přijetí opravené hry ani povolení implementace. Výběr uživatele čeká.
Claude dodržel přidělené cesty, nic ve hře neměnil a uvolnil editor/build.
Výstupy jsou převzaté do koordinační větve; pracovní větev zůstala nedotčená.

## Co bylo skutečně ověřeno

- Fetch obou commitů, diff pouze povolených výstupů; přečten PROPOSAL,
  SUMMARY_CZ, PROGRESS, HANDOFF a měření. Oba obrázky evidence staženy přes
  LFS a vizuálně prohlédnuty. Složený podklad skutečně obsahuje podobný
  pruhovaný vzor mraků jako uživatelův snímek.
- V aktuálním Tools/Prepare-SolarSystemMaterials.py ověřeno: maximum importu
  8192, směšování Cloud.r*.7 přímo do povrchu Země, žádná samostatná vrstva
  zemských mraků ani výšková/odlesková mapa v tomto materiálu.
- V Tools/Prepare-SolarContent.py ověřen unlit materiál a běžný TextureSample;
  v SolarRendering.cpp nativních100%, vypnuté časové AA/rozmazání, aniso16.
- Lokální engine BaseDeviceProfiles.ini potvrzuje skupinu World s
  MipFilter=point a MaxLODSize16384. Skutečný vliv na konkrétní záběr je ještě
  potřeba izolovat v běžící hře.
- Předchozí cílený běh0020 prokazuje plnou rezidenci8K Země v tehdejších
  záběrech. Neprokazuje přesné vzorkování v nové fotografii uživatele.
- Nebyl spuštěn editor, build ani nový herní test. Je to revize návrhu.

## Porovnání dvou pohledů

| Oblast | Původní pohled Codexu | Co přinesl Claude | Hodnocení |
| --- | --- | --- | --- |
| Hlavní limit | Globální8K mapa nestačí pro detail malého kusu planety při blízkém pohledu. | Přibližný výpočet5–6obrazových pixelů na texel v referenci. | Shoda; číslo je orientační, přesná kamera není známá. |
| Mraky | Oddělit mraky od povrchu, zlepšit zdroj a materiál. | Doložil pruhovaný vzor přímo ve zdroji a jeho složení do povrchu. | Silný užitečný důkaz, konkrétnější než původní hypotéza. |
| Nastavení | Prověřit zdroje, sampling/import a skutečný runtime; ne plošné zvětšování. | Přidal kompresi, mip filtr, osvětlení a geometrii. | Dobré kandidáty pro izolované ověření, nejsou všechny prokázané příčiny. |
| Řešení | Lepší skutečné mapy/detail po oblastech a samostatné mraky. | B: více vrstev, procedurální detail, mraky, odlesky a reliéf; A/C podle potřeby. | B je vhodný společný základ, skutečné rozlišení pobřeží vyžaduje lepší data. |

## Doporučení

**B jako společný základ, doplněné cíleně kvalitnějšími skutečnými mapami.**
Nejdřív Země: oddělené mraky a jejich stín, vhodné kódování a filtrování,
odlesk oceánu, věrohodný reliéf, dostatečně jemný obrys. Jemné generované
detaily jsou vzhledový doplněk; skutečná geografie zůstane podle dat.

Na Zemi současně prověřit dostupný kvalitnější podklad. Pokud v nativním4K
zůstane pobřeží měkké, otestovat skutečný16K podklad, nikoli zvětšené8K.
Pořadí B a potom porovnání B+16K pomůže zjistit, co která změna přinesla.
Dlaždicové řešení C ponechat pro případ, že uživatel bude chtít ještě bližší
skutečnou geografii. Nemá smysl ho nyní stavět pro všechna tělesa.

Rozšíření na ostatní tělesa až po uživatelově přijetí Země, včetně lepších
zdrojů pro malé současné mapy měsíců/obrů. Fiktivní světy mohou využívat
generovaný detail výrazněji. Nevracet schválenou loď ani hvězdné body zpět.

## Výhrady a zpřesnění pro případné realizační zadání

1. B zvyšuje vnímaný detail, **nevytvoří skutečné ostré pobřeží z chybějících
   dat**. Nelze slíbit, že B samo splní každý blízký pohled. „4K nepomůže
   vůbec“ je příliš absolutní u těles, která dnes mají mapy menší.
2. Vzorek mraků ze stejného UV nemusí mít stejné rozlišení jako povrch.
   Současný problém je konkrétní8K zdroj a způsob směšování. Samostatná vrstva
   ani přidaný šum samy nezaručují odstranění pruhů. Vybrat/zlepšit zdroj a
   porovnat přirozený tvar; neudělat z mraků tvrdě ořezané bílé skvrny.
3. Barevný posun, podíl komprese a mip přechodů jsou runtime hypotézy.
   Offline BC1 simulace není důkaz skutečného cooked formátu. PixelNormalWS
   není jen plochý vrcholový normál. Bez normal map však detail reliéfu chybí.
4. Test zmenšení/zvětšení zdroje prokazuje vysokofrekvenční obsah, ne jeho
   skutečný původ. Nelze z něj dokázat, že soubor nikdy nebyl zvětšen.
5. Masky musí mít vhodný lineární barevný prostor; přechod z dnešní sRGB
   mapy mraků na BC4 ověřit podle výsledné coverage. BC7/BC4/BC5 musí být
   potvrzené po cooku, importní volba sama není hotové řešení.
6. Paměť je odhad:8K BC7/BC5 celý řetězec asi42.7MiB na mapu, BC4/BC1
   asi21.3MiB. B má asi128MiB na uvedené mapy; výměna pouze denní mapy za16K
   zvedne součet zhruba na256MiB. Další masky/vrstvy/ostatní tělesa jsou navíc.
   RTX3070 je známá z předchozího běhu, uživatel ji nemusí znovu zjišťovat.
   GPU přirážka1ms je návrh cíle, nikoli změřený nebo zaručený výsledek.
7. Vyšší rozlišení výškových dat se při upečení malé mapy opět ztratí.
   Zvolit skutečnou hustotu podle kamery. Reliéf neznamená automaticky fyzicky
   vystouplé hory; běžná normal mapa mění osvětlení, ne siluetu.
8. Cube-sphere sama neopraví póly staré válcové textury; nutná reprojekce
   nebo jiné vzorkování. C se dá použít i na fiktivní data, není technicky
   omezené na skutečné planety. Náročnost musí ospravedlnit viditelný přínos.
9. Akceptace podle Docs/VISUAL_QUALITY.md: skutečný render3840×2160,
   100%výřezy a stejná kamera/slunce ve srovnání.1440p může být pomocný běh.
   Kontrola blízko/daleko/horizont/pohyb/mapa, bez plošného dlouhého testování.
10. Nové zdroje musí mít doložený původ; doplnit dependency hash importní
    recipe, validaci skutečně zapojených map a packaged dostupnost. Nové
    master materiály nemění kanonická data/stav singleplayer simulace.

## Primární podklady zkontrolované při revizi

- [NASA BMNG](https://svs.gsfc.nasa.gov/3523/) potvrzuje původní cloud-free
  data přibližně500m; snímky na této konkrétní stránce mají jen4000×2000.
  Nestahovat je jako domnělý16K podklad; zvolit skutečná zdrojová data.
- [NOAA ETOPO2022](https://www.ncei.noaa.gov/products/etopo-global-relief-model)
  potvrzuje výšková data15/30/60arcsec a požadovanou citaci. Dostupnost,
  konkrétní soubory a prostor pro zpracování ověřit před stažením.

## Další krok

Uživatel vybere postup. Doporučená volba: B s ověřením lepšího skutečného
podkladu Země, nejdřív pouze Země ve4K. Potom koordinátor připraví konkrétní
realizační úkol pro Claude s cestami, vlastnictvím editoru a kontrolami.
Implementace zatím nezačala a tento dokument ji sám nespouští.

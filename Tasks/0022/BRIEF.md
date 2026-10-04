# 0022 — Nezávislá diagnóza a návrh kvality planet

- Vlastník: **Claude Code**. Koordinátor/reviewer: Codex.
- Repo: https://github.com/martinkosek69-eng/daedalus-game
- Herní základ: `906567c5023cffb30825f2c24b37833f4561f7ca`.
- Zakládací commit zadání je publikovaný ve vyhrazené větvi
  **codex/claude-0022-planet-quality**. Začni z jejího zakládacího commitu;
  obsahuje výše uvedenou hru a toto zadání. Zjisti jeho SHA přes Git.
- Vlastní nová pracovní kopie na A:, nepřepínej cizí checkout.
- Cílová větev návrhu změn: `codex/solar-flight`, nikdy main.
- **Nyní je povolena pouze fáze1: analýza a návrh. Implementace čeká na
  výběr uživatele po porovnání s pohledem Codexu.**

## Proč dostáváš úkol

Uživatel odmítá měkké/rozpixelované povrchy planet a mraků, především v
blízkém orbitálním pohledu. Referenční obrázek je `REFERENCE.png`: planeta
zabírá téměř celý záběr, loď a HUD jsou čitelnější než její povrch a oblaka.
Podobné výhrady uvádí i k dalším tělesům. Ostrost Daedala už schválil; obloha
má být samostatné generované hvězdné body, nikoliv fotografické pozadí.
Hra je čistě singleplayer, nyní šest soustav a letový testovací svět v Unrealu.

**Chceme dva nezávislé odborné pohledy.** Codex má svůj předběžný návrh;
není ti zde předkládán jako správná diagnóza ani řešení. Samostatně prohlédni
skutečné soubory hry, zdroje a nastavení a urči příčinu. Nehledej potvrzení
cizího závěru. Existující dokumentaci o předchozích kontrolách ber jako záznam
konkrétních minulých běhů, ne jako důkaz pro tento záběr. Pochybnosti označ.
Pokud důkazy povedou ke stejnému závěru, je to v pořádku; rozdíl nevymýšlej.

Uživatel navrhl možnost přegenerovat objekty do4K, ale nepředepisuje technologii
ani rozlišení. Tvůj návrh musí vysvětlit, co konkrétně zlepší obraz a proč.
Zkontroluj zdroj, geometrii, materiály, import/cook a skutečné vykreslení podle
potřeby. Nekonči pouhým opisem dokumentace nebo obecnou radou pro engine.

## Podklady a nezávislý postup

Nejdřív AGENTS.md, README.md, Docs/ENVIRONMENT.md, FOUNDATION.md,
ARCHITECTURE.md, CODING_RULES.md, CURRENT_STATE.md, Tasks/README.md.
Pro orientaci Docs/SOLAR_FLIGHT.md, SOLAR_REALISM.md, CONTENT_WORKFLOW.md.
Najdi odpovídající zdroje a runtime napojení sám v Art, Tools,
Game/Daedalus/Content a Game/Daedalus/Source. Porovnej stejné podmínky záběru,
ne vzdálenou planetu v jedné verzi s velkým přiblížením v druhé.

1. Sepiš symptomy, varianty možných příčin a konkrétní zjištění s cestami/
   řádky/názvy assetů, rozměry, nastavením a podle potřeby snímky.
2. Rozliš potvrzený problém, pracovní hypotézu a co bez dalšího testu nevíš.
   Rozlišení souboru samo o sobě není měřítko skutečného detailu ve hře.
3. Navrhni alespoň dvě rozumné varianty, vyber svou doporučenou a zdůvodni ji.
   Porovnej kvalitu dálka/blízko, náročnost implementace, paměť/výkon, velikost
   zdrojů, údržbu a použitelnost pro skalnaté/ledové/plynné světy a měsíce.
4. Navrhni konkrétní první ověřovací krok na Zemi a následný rollout na nynější
   soustavy; nemáme dělat slepé hromadné změny před ověřením základní volby.
5. Popiš potřebná data/nástroje, dohledatelný původ/licence, omezení a stručné
   vizuální podmínky přijetí. U neznámých povrchů nenabízej smyšlenou přesnost.

Priorita je vysoká kvalita a čitelnost. Uživatel chce šetřit usage a většinu
osobního hraní/testování udělá sám. Dělej cílenou diagnostiku; žádné opakování
celé testovací sady nebo všech569položek bez důvodu. Nic nestahuj/generuj hromadně
a ještě nepřidávej nové4K/8K assety. Je-li potřebná rešerše, používej primární
zdroje a zapiš, jaké tvrzení podporují; nepředkládej reklamu za důkaz.

## Přesný výstup fáze1

- `Tasks/0022/PROPOSAL.md`: vlastní diagnóza a doporučené řešení včetně variant,
  důkazů, nejistot, etap a podmínek přijetí. Technické detaily patří sem.
- `Tasks/0022/SUMMARY_CZ.md`: maximálně jedna krátká stránka pro neprogramátora:
  co je problém, co navrhuješ, jaký rozdíl uvidí, nevýhody a potřebné rozhodnutí.
- `Tasks/0022/PROGRESS.md` a `HANDOFF.md`: skutečně provedená práce a předání.
- Volitelně `Tasks/0022/EVIDENCE/**`: stručná vlastní evidence/snímky bez
  soukromých/strojových údajů. Velké/raw diagnostické logy pouze ignored.local.

Reference je uživatelův snímek této hry; související zdrojové modely/data mají
licence a původ v příslušných Art manifestech. Nedávej do veřejného Gitu
konverzace, credentials, osobní fotografie nebo celé strojové výpisy.

## Povolené změny a aplikace

**Měnit a commitovat smíš pouze PROPOSAL.md, SUMMARY_CZ.md, PROGRESS.md,
HANDOFF.md a EVIDENCE/** ve svém úkolu. BRIEF, START, REFERENCE, INDEX a
společné docs vlastní koordinátor. Celou hru/art/tools můžeš číst.
Soukromé pomocné skripty/výstupy pro analýzu patří do ignored.local tvé kopie.
Neměň herní zdroj, materiály, modely, data, uložené pozice ani existující balíček.
Žádný Assets/reimport, SaveAll nebo přegenerování obsahu v této fázi.

Pro cílené read-only prohlížení je ti přidělen jeden Unreal editor a případný
nezbytný build nezměněného základu; viz Docs/APP_OWNERSHIP.md. Codex do editoru
paralelně nevstupuje. Otevírej jen .uproject své kopie, před MCP ověř cestu.
Vygenerované build/cache/save soubory musí být ignored a na A:. Před plným C++
buildem zavři jen svůj editor. Nezavírej žádné uživatelovy existující aplikace.
Blender případně jen background/read-only nad svými kopiemi; netvoř nové assety.
Instalační cesty/TEMP/DDC jsou v ignored.local toolchain nebo DAEDALUS_* env.

## Publikace, porovnání a další fáze

Ukládej smysluplné checkpointy s normálním commitem/push na svoji větev.
Po hotovém návrhu: `READY_FOR_REVIEW`, push, potvrď SHA a uvolni svůj editor/
build v HANDOFF. Codex načte přes GitHub PROPOSAL, posoudí důkazy a porovná
návrh se svým samostatným pohledem. Uživatel pak vybere postup.

**Po publikaci návrhu se zastav. Neimplementuj ho automaticky, ani kdybys
jeho správnosti věřil.** Pro celou následnou implementaci je plánovaný vlastník
Claude, ale začne až novým konkrétním zadáním/výběrem uživatele. Výpadek limitu
označ pravdivě s posledním vzdáleným commitem a konkrétním zbytkem analýzy.
Žádný merge do main, force push nebo přepis jiné větve.

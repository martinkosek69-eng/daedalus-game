# 0023 — Kompletní úprava kvality planet a těles

- Vlastník celé implementace: **Claude Code**. Koordinátor/integrace: Codex.
- Repo: https://github.com/martinkosek69-eng/daedalus-game
- Základ: `58cdfadde38281d754e1307da353fab08bc1a62a`.
- Pracovní větev: **codex/claude-0023-planet-upgrade**. Její zakládací commit
  obsahuje tento základ a zadání0023. Zjisti SHA přes Git; vlastní čistý
  checkout na A:, nikdy cizí kopie. Cíl předání `codex/solar-flight`, ne main.
- Uživatel po návrhu a revizi zadal „nech claude udelat tu praci“.
  **Implementace celé planetární části je nyní autorizovaná.**

## Postup a výsledek

Proveď **B s cíleně lepšími skutečnými mapami**, podle0022/PROPOSAL.md a
0022/REVIEW.md. Tvoje odpovědnost zahrnuje zdroje, materiály, geometrie,
import, runtime napojení, validaci a hratelný Unreal balíček. Nekonči pouze
obrázky či Blender scénou. Příprava musí být opakovatelná z GitHubu.

Přečti AGENTS, README, ENVIRONMENT, FOUNDATION, ARCHITECTURE, CODING_RULES,
CURRENT_STATE, VISUAL_QUALITY, CONTENT_WORKFLOW a Tasks/README. Singleplayer;
vzhled nesmí měnit kanonický stav. Vše ostré minimálně v nativním3840×2160
pro32palcový monitor;1440p zvětšené na4K není ověření. Rozlišení map vol
podle viditelného výsledku. Šetři usage, uživatel udělá většinu osobních testů.

1. **Nejprve Země:** samostatná vrstva mraků, věrohodné osvětlení a stín,
   odlesk oceánu, podložený reliéf v osvětlení, kvalitní kódování/filtrování,
   dostatečně jemný obrys. Zlepši/vyměň pruhovaný zdroj mraků. Šum sám ani
   oddělení vrstvy nezaručují opravu. Nepoužívej tvrdé bílé vystřižené fleky.
2. Porovnej ze stejného blízkého záběru jako0022/REFERENCE a z celkového
   pohledu. Pokud pobřeží nestačí, doplň skutečný16K podklad, ne zvětšené8K.
   Získání vhodných veřejných podkladů a jejich příprava jsou autorizované.
   Dlaždicové C zatím neimplementuj; zásadní změnu řešení nejdřív navrhni.
3. Po cíleném ověření a checkpointu Země pokračuj na **ostatní současná
   tělesa všech šesti soustav**: skalnatá, ledová, plynní obři, měsíce,
   menší fyzicky zobrazená tělesa i hvězdy podle jejich typu. Zlepši malé
   zdrojové mapy tam, kde nestačí. Sdílená řešení místo stovek kopií.
4. Regenerování potřebných fiktivních planetárních zdrojů je povolené,
   zachovej ID a základní charakter světů. Pro skutečná tělesa označ
   generovaný detail jako výtvarný doplněk. Nevymýšlej povrchy/polohy pro
   neznámé katalogové položky bez fyzického modelu.
5. Mraky jen tam, kde odpovídají typu tělesa; plynní obři nemají kamenný
   reliéf a hvězda nepotřebuje planetární osvětlení. Mapa může zachovat
   samostatné jednodušší, ale ostré miniatury.

Země-first je pořadí ověření, ne povinnost čekat na nový souhlas po každém
kroku. Uživatel autorizoval celou práci; po technickém/vizuálním ověření
pokračuj v rollout. Neosvědčený základ oprav před rozšířením. Konečné
estetické přijetí udělá uživatel po předání.

Zachovej schválený Daedalus/plátování, generované ostré hvězdné body, černé
pozadí, ovládání/rychlosti/R/Shift+R, kameru a kompaktní HUD/mapu. Žádné
globální rozmazání, časové AA, upscaling nebo snížení kvality kvůli planetám.
Globální streaming/LOD neměň bez doloženého problému a koordinace.
GPU je známá RTX3070. Změř skutečnou paměť a čas ve4K v celé aktivní soustavě,
ne pouze jedné planetě. Odhad1ms v návrhu není záruka.

## Přidělené cesty

Smíš měnit pouze následující cesty, výhradně pro planetární prezentaci:

- Art/Space/SolarSphere.glb, Art/Space/Textures/**, SolarSystem/**,
  SolarDetails/**, Systems/** a nové Art/Space/PlanetQuality/**.
  Zachovej původ/licence a návratový bod; nevytvářej nové soustavy.
- Game/Daedalus/Content/Solar/Textures/**, Materials/**, Models/**,
  SM_SolarSphere.uasset a nové Content/PlanetQuality/**.
  **Všechny ship/Daedalus/engine/glow/glass/light/sky/starfield/dust assety
  jsou vyloučené i v těchto složkách. M_Star bodových hvězd zachovej.**
- Game/Daedalus/Content/Data/Solar/system.json a Systems/**: pouze odkazy
  a parametry vzhledu. ID, jednotky, rozměry, pozice, hmotnosti a letová
  pravidla zachovat. Nová vzhledová pole validuj s kompatibilním fallbackem.
- Game/Daedalus/Source/Daedalus/Solar/SolarSystem.cpp, SolarRendering.cpp,
  SolarFlightGameMode.h, SolarSharpProbe.cpp, nové Solar/Planet*.h/.cpp:
  jen planetární napojení/cleanup/diagnostika. Globální sharp a ship/star
  rendering zachovat. Žádná změna simulace nebo letových pravidel.
- Game/Daedalus/Config/DefaultDeviceProfiles.ini: pouze planetární skupina,
  žádná změna globální skupinyWorld či ship kvality.
- Tools/Prepare-SolarContent.py, Prepare-SolarSystemMaterials.py,
  Prepare-SolarSystemSources.py, Prepare-SolarDetails.py, Prepare-FiveSystems.py,
  Validate-SolarContent.py, Validate-SystemAssets.py, nové Tools/*Planet*.py
  a Tools/*Planet*.ps1: jen příprava/ověření přiděleného obsahu.
- Docs/PLANET_QUALITY.md, vlastní Tasks/0023/PROGRESS.md, HANDOFF.md,
  RESULTS_CZ.md a EVIDENCE/**.

Cesty po první plné cestě v odrážce jsou pod stejnou rodičovskou složkou.
BRIEF, START, INDEX, AGENTS a ostatní společné docs vlastní koordinátor.
Žádné flight.json, lodě, galaxie/UI, .umap, cizí soubory nebo pozice hráče.
Další nutnou cestu nejdřív popiš v progress/předání pro koordinátora.

## Aplikace a ověření

Claude dostává jeden Unreal editor, přípravu assetů a build/package slot,
Codex je nepoužívá souběžně; viz Docs/APP_OWNERSHIP.md. Ověř skutečnou
.uproject vlastní kopie před MCP. Před plným buildem zavři jen vlastní
editor. Nezavírej lidské aplikace. Blender na pozadí nad vlastními uloženými
zdroji, nikdy uživatelova GUI scéna. Toolchain/cache/build/temp na A: ignored.

- Ověř cooked formáty/rozměry, lineární prostor masek, stíny mraků,
  terminátor/horizont, den/noc, přechody detailu. Normal mapa není silueta hor.
- Dodej stejné4K záběry před/po a100%výřezy. Ověř blízko/daleko/pohyb bez
  blikání, Zemi, reprezentanta každého změněného typu a nejméně jedno těleso
  každé fiktivní soustavy. Strojově validuj všechna nová napojení.
- Recipe hash zahrne nové zdroje/helpery; opakovaný import zachová výsledek.
  Validace kontroluje skutečně zapojené povrchy/normal/masky a mapové materiály.
- Editor build, relevantní kontroly a Windows package z vlastní kopie:
  Tools/Invoke-SolarFlight.ps1 -Mode Package -BuildName Build-PlanetQuality.
  Vlastní play/probe data. Nepřepisuj dosavadní balíček/savy/launcher uživatele.
  Velké obecné sady neopakuj bez důvodu. Při lifecycle změnách ověř změnu
  soustavy a cleanup vrstev/paměti. Čas/paměť označ měřením nebo odhadem.
- Publikuj editovatelné zdroje, generátory, původ/licence a potřebné importované
  assety přes LFS. Obří syrová data lze nahradit dohledatelnou přípravou,
  výsledné potřebné zdroje ale musí být dostupné.
- Po Zemi a dalších ucelených krocích PROGRESS + commit/push. Při konci kvóty
  pravdivý checkpoint; žádný merge do main/forcepush.
- Hotovo: READY_FOR_REVIEW, HANDOFF, krátké RESULTS_CZ, větev/SHA, změněné
  soubory, kontroly/omezení, cesta vlastního balíčku a návod. Výslovně uvolni
  editor/build. Codex výsledek převezme a integruje po kontrole.

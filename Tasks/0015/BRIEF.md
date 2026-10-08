# 0015 — Pět originálních soustav pro letové testování

Owner: Claude Code. Integrator: Codex. Singleplayer, žádné permanentní role.
Base commit:4e2e7b6 (codex/solar-flight; převzatý ship-source checkpoint).
Work branch: task/0015-five-original-systems. Samostatný worktree na A:.
Nepracovat v Codex checkoutu, nepřepínat větev jiného agenta. Root vlastní Unreal.

## Účel

Vytvoř přesně pět zajímavých, věrohodných fiktivních soustav. Hvězda,planety,
měsíce,prstence a pásy malých těles. Žádné vraky,stanice,lodě,boj,HUD ani
hyperprostor. Hráč má procvičovat lety v různých soustavách. Codex je později
začlení,nasvítí a připojí k3D mapě,navigaci a testům. Nedělej herní integraci.

Než začneš, fetchni GitHub, přečti AGENTS,README,Docs/ENVIRONMENT,
FOUNDATION,ARCHITECTURE,CODING_RULES,CURRENT_STATE a Tasks/README.
Zadání čti z aktuální origin/codex/solar-flight:Tasks/0015/BRIEF.md.
Veškeré aplikace/cache/pracovní data na A:. Toolchain cesty pouze v.local.
Blender pracuje přes uložené zdroje v samostatném background procesu. Živou
Unreal relaci ani jinou otevřenou Blender scénu neovládej. Koordinátor používá
Unreal souběžně; jeho projekt/content zdroje jsou mimo tvou povolenou oblast.

## Povolené změny

- Art/Space/Systems/Asterion/**
- Art/Space/Systems/Velara/**
- Art/Space/Systems/Nivara/**
- Art/Space/Systems/Caelum/**
- Art/Space/Systems/Morava/**
- Tools/Prepare-FiveSystems.py (opakovatelné vytvoření/exporty)
- Tasks/0015/PROGRESS.md a HANDOFF.md

Brief a INDEX vlastní Codex. Nesahej do Game,Docs,existujících solárních/ship
assetů ani do jiných úkolů. Nepublikuj raw logy,osobní fotografie,chat,cache.
Normální checkpoint commity a push na svou větev; žádný merge/force push.

## Pět soustav

Jména lze použít následující; fixní id a adresáře dodrž.
Fiktivní pozice jsou galaktocentrické světelné roky, +Z nad diskem.
Sol bude[26000,0,0]; vzdálenosti nejsou přímé světové souřadnice vmetrech.

| Id | Adresář/jméno | Galaxy position lightyears | Charakter |
| --- | --- | --- | --- |
| fic.asterion | Asterion | [25970,40,20] | žlutáG hvězda,blízká pozice,skalnaté světy a plynný obr |
| fic.velara | Velara | [25400,300,100] | oranžováK hvězda,oceánský svět,oblačná planeta,prstence |
| fic.nivara | Nivara | [7000,-7000,-120] | červenýM trpaslík,kompaktní malé světy,led a sopečný svět |
| fic.caelum | Caelum | [-28000,14000,180] | modrobíláF hvězda,větší soustava,dva odlišní plynní obři |
| fic.morava | Morava | [35000,-22000,-80] | klidnáG/K hvězda,rozsáhlý ledový vnější pás a malé měsíce |

Každá4–7 planet,alespoň5měsíců,alespoň1pás malých těles. Přinejmenším3
soustavy mají viditelné realistické prstence. Rozmanité barvy/povrchy,ne pouze
náhodně přebarvená Země. Rozměry,AU vzdálenosti,délky dne a povrch odpovídají
obecně plausibilním světům; nejde o tvrzení o skutečně objevených exoplanetách.

## Datový kontrakt

Každý adresář obsahuje system.json v UTF8, SOURCES.md,editovatelný.blend,
GLB potřebných opakovatelných modelů,Textures/*.png/jpg a preview.png.
Hladkou jednotkovou kouli exportuj s poloměrem1m,+Z sever; skutečné velikosti
jsou VÝHRADNĚ vdatech. Importer převede1m na100cm. Pro irregular body vlastní
jednotkový model s bounds/maxrad1m. Preview/blend může mít samostatnou
výslovně schématickou scénu kvůli čitelnosti; neměň fyzické metry vJSON.
Nepoužívej jedinýGLB s celou soustavou v astronomických rozměrech.

Top-level:
version:1,id:<system id>,name:<human name>,source:"fictional",
galaxyPositionLightYears:<výše>,primaryStarId:<id>.sun,
stellarColor:"#rrggbb",bodies:[...],belts:[...].
Belt: id,parentId,innerMetres,outerMetres,count:600,seed:<fixed>,kind:"asteroid"/"icy".

Body:
id:<systemid>.<stable slug>,name,parentId (hvězda prázdné),
kind:"star"/"planet"/"moon"/"asteroid",radiusMetres>0,
positionMetres:[x,y,z] vzhledem kLOKÁLNÍMU středu soustavy vdoublemetrech,
texture:"Textures/filename.png",rotationHours (může být záporné),
tiltDegrees,shapeScale:[1,1,1] nebo odůvodnělézploštění,
surfaceQuality:"fictional-authored",optional mesh:"Models/filename.glb".
Optional atmosphereColor:"#rrggbb",cloudTexture,nightTexture,
ringInnerMetres,ringOuterMetres,ringTexture:"Textures/filename.png" (RGBA).
Moon position je absolutní lokálnímetrová pozice vté soustavě, nikoliv parentoffset.
Hvězda je[0,0,0]. Rádia a vzdálenosti nepřekrývají rodiče/sousední tělesa.
Textury equirectangularUV, planetárním pólem+Z;2048×1024min,důležitéplanety
4096×2048. Prstence radiální1D barevnýprůřez valpha,UVx vnitřek→vnějšek.
Procedurální vlastní mapy/bakes uznej jako fiktivní; neslibuj jejich fotografický
původ. Jako textury lze použít řádně licencované zdroje, sURL/licencí/attribution.
GLB PBR standard,podporaBaseColor/roughmetal/normal,tangent normalOpenGL.
Soubor se jmenuje system.json v každém přiděleném adresáři.

## Ověření před předáním

- Přesně5soustav,všechny unikátníIDs,validní rodiče a konečné kladné metry.
- Planety/moons nepřekrývají primární/parentkonzervativní koule.
- OvěřitKepler/plausibilnídostupné rozložení; žádnéhvězdy ve velikostiplanety.
- Decode každou texturu; nikde nepoužívat sRGBORM/normal.
- Uložit a znovu otevřít.blend,ověřitGLBbounds1m;prohlédnout skutečnépreview.
- Výsledek musí jít vytvořit/exportovat opakováním sdíleného skriptu.
- HANDOFF vypíše5system.json cesty,počtytěles,velikostifiles,licence,checks,
  známéomezení a poslednícommit. Stav READY_FOR_REVIEW,nikoliv ACCEPTED.

Průběžně ukládej menšícheckpointy s normalpush; kvóta nesmí znamenat ztrátu
práce. Když se zastavíš,zaznamenej přesně hotové/neověřené věci a handoff.
Codex dotestuje assety vUnrealu a kompletuje hru. Začni pouze tímto úkolem.

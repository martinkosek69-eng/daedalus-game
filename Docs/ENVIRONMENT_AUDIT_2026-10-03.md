# Veřejné shrnutí ověření prostředí — 3. října 2026

Tento dokument obsahuje projektové výsledky a omezení. Úplné místní záznamy,
osobní cesty, nastavení počítače a diagnostické výpisy jsou v ignorovaném úložišti
a nejsou součástí veřejného repozitáře. Neobsahuje export osobní konverzace.

## Ověřené postupy

- C++ editor target v Unreal 5.8.3 byl úspěšně sestaven s Visual Studio C++
  toolchainem a Windows SDK. Sestavení nové pracovní cesty také prošlo.
- Blender 5.2.2 LTS: background bpy vytvoření a uložení .blend, opětovné
  načtení, GLB export, render a vizuální kontrola; správné předání chyby skriptu.
- Základní GLB byl importován do Unrealu jako StaticMesh a materiál přes
  commandlet. Testovací assety byly po kontrole odstraněny.
- Unreal editor: čtení/změna/obnova kamery, vytvoření/vyhledání/odstranění
  zkušebního objektu, spuštění/zastavení hry v editoru a snímek viewportu.
- Projektový Node MCP helper: čerstvý proces, build vlastního checkoutu,
  Blender save/reload/export/render a error reporting.
- Původní záloha prototypu byla ověřena kontrolními součty a zůstala zachovaná.
- Pracovní úložiště bylo přesměrováno na preferovaný vývojový disk; obsah
  přenesených souborů i zachování stavu Gitu byly ověřeny. Viz STORAGE_ON_A.md.

## Změny společného základu

Bylo opraveno escapování Windows Build.bat v Node subprocessu. Projektový helper
odvozuje projekt z vlastního umístění a místní instalace čte z ignorovaného
nastavení. Doplněné pravidlo GameFeatureData v AssetManageru odstranilo související
startup LoadErrors. Prázdný Content je zachován přes .gitkeep. Byly doplněny
konfigurace obou klientů, společné pokyny, návody a opakovatelné kontroly.

## Dosud neověřené části

Claude Code relace, skutečné předání mezi dvěma poskytovateli a upload/download
binárního LFS objektu zatím nebyly ověřeny. První číslovaný úkol ověřuje textové
předání; nepředstavuje ověření všech aplikací nebo souběžné editace assetů.

Unreal MCP je experimentální. Některé viewport parametry bylo nutné zadat
explicitně navzdory jejich označení jako volitelných. Při dřívějších startech
zůstaly dvě interní automation-test chyby s neurčenou příčinou. Engine/compiler
varování přetrvávají; úspěšný build není potvrzení bezchybnosti všech funkcí.

Neověřovalo se živé GUI/debugger Visual Studia, neuložená Blender GUI scéna,
všechny materiály/animace, balení finální hry ani výkon rozsáhlé scény.
Jednoho vlastníka editoru zajišťuje pracovní dohoda; klienti nemají společný
technický mutex. Cloudový agent potřebuje místní vykonávání pro místní aplikace.

Kontroly lze zopakovat postupy v [ENVIRONMENT.md](ENVIRONMENT.md).
Soukromé testovací výstupy patří do ignorované `.local`.

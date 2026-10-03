# Zadání 0001 — Ověření předávání přes GitHub

- Vlastník práce: Claude Code po předání tohoto zadání uživatelem
- Koordinátor a přejímající: Codex
- Repozitář: https://github.com/martinkosek69-eng/daedalus-game
- Zadání dostupné ve větvi: `codex/shared-agent-workflow`
- Výchozí commit práce: konkrétní vzdálený commit větve `codex/shared-agent-workflow`,
  který obsahuje toto zadání. Zaznamenej jeho plné SHA v RESULT.md před zahájením
  změn. Nepřebírej lokální změny z jiné relace. Pokud hlavní větev již obsahuje
  toto zadání, může koordinátor výslovně aktualizovat základ.
- Pracovní větev: `task/0001-github-handoff`
- Cílová větev pro návrh změn: `codex/shared-agent-workflow`
- Potřebné aplikace: místní Claude Code, Git a přihlášení s právem push
- Unreal editor: NONE; tento úkol editor nespouští ani neovládá

## Cíl

Ověřit, že Claude umí přečíst společné zadání, bezpečně pracovat ve vlastní kopii,
odeslat průběžný checkpoint a předat výsledek tak, aby jej Codex mohl stáhnout
bez ručního přenášení souborů uživatelem. Je to malá technická zkouška před
tvorbou obsahu hry.

## Vstupy a hranice

Přečti AGENTS.md, README.md, Docs/ENVIRONMENT.md, Tasks/README.md a toto zadání.
Použij vlastní pracovní kopii na A:. Pokud klient běží na jiném počítači,
zaznamenej to a použij svou oddělenou místní kopii; netvrď přístup k aplikacím
tohoto PC. Rozměry, měřítko, modely a výkon hry jsou v této textové zkoušce N/A.
Nejsou potřeba žádné externí podklady, nové programy ani placené služby.

Povolené změny jsou pouze:

- `Tasks/0001/PROGRESS.md`
- `Tasks/0001/HANDOFF.md`
- `Tasks/0001/RESULT.md`

Neměň herní soubory, ostatní úkoly, INDEX, zadání ani společné nastavení.
Nevytvářej .blend/.uasset/.umap a neovládej editor. Tento úkol sám neověřuje MCP,
LFS přenos ani tvorbu modelů. Pokud pracovní větev již existuje, ověř přidělení
vlastníka s koordinátorem před jejím převzetím.

## Postup a povinné výstupy

1. Ověř dostupnost zadání, místního Gitu a možnost normálního push. Nevypisuj
   hesla ani tokeny. Založ vlastní pracovní větev ze zadaného společného základu.
2. Vytvoř RESULT.md s číslem úkolu, klientem/rolí, SHA výchozího commitu,
   potvrzením vlastní pracovní kopie a seznamem přečtených společných pokynů.
   Absolutní osobní cesty a obsah soukromého chatu nezveřejňuj.
3. Do RESULT vlož potvrzovací řádek `DAEDALUS_HANDOFF_TASK_0001`.
   Aktualizuj PROGRESS na IN_PROGRESS, ulož první commit a push na pracovní větev.
   Ověř shodu lokálního a vzdáleného SHA; toto je průběžný checkpoint.
4. Doplněním RESULT popiš, zda checkpoint šel odeslat, uveď SHA prvního
   checkpointu a přesný postup pro Codex: stáhnout větev a přečíst tyto soubory.
   Vyplň HANDOFF a PROGRESS nastav na READY_FOR_REVIEW. Odešli druhý commit a push,
   opět ověř vzdálený SHA. Není potřeba vytvářet commit s jeho vlastním SHA.
5. Uživateli sděl pouze číslo úkolu, větev, poslední SHA a případné omezení.
   Neslučuj do cílové větve ani main. Pull request je volitelný.

## Podmínky pro přijetí

- Jsou změněné pouze tři povolené soubory a společná pravidla zůstala zachovaná.
- RESULT obsahuje správné číslo, potvrzovací řádek a skutečný výchozí commit.
- Vzdálená pracovní větev obsahuje nejméně dva smysluplné checkpointy tohoto úkolu.
- HANDOFF uvádí skutečný výsledek kontrol a potřebné kroky k pokračování.
- Codex po převzetí dokáže stáhnout a přečíst tyto výsledky přímo z GitHubu.
  Tuto poslední kontrolu provádí Codex, pracovník ji nesmí vydávat za otestovanou.

## Přerušení

Při omezení účtu, chybějícím přístupu nebo vyčerpání limitu zachovej rozpracované
soubory a poslední dostupný checkpoint. Je-li možné, aktualizuj PROGRESS/HANDOFF
s blokací a odešli je. Pokud push nefunguje, výslovně řekni, že práce zůstala
lokální. Nikdy netvrď, že ji Codex najde na GitHubu bez úspěšného odeslání.

# Výsledek úkolu 0001 — Ověření předávání přes GitHub

DAEDALUS_HANDOFF_TASK_0001

- Číslo úkolu: 0001
- Klient a role: Claude Code (desktopová aplikace, místní vykonávání na
  uživatelově Windows PC), pracovník úkolu; koordinátor a přejímající je Codex
- Výchozí commit: `e097331d24e234e3997cd1fccba333521e4ad57d`
  (vzdálená větev `codex/shared-agent-workflow`, obsahuje BRIEF.md tohoto úkolu)
- Pracovní větev: `task/0001-github-handoff`, založená přímo z výchozího commitu
- Spuštění: na samostatný pokyn uživatele po dokončené úvodní kontrole

## Vlastní pracovní kopie

Pracuji ve vlastní samostatné pracovní kopii na disku A:. Je to klon z GitHubu,
ne pracovní kopie Codexu ani hlavní projektová složka. Kopie neobsahovala
necommitované změny a nepřebírá soubory z jiné relace. Před prvním commitem
`git status` ukazoval jen změny tří povolených souborů tohoto úkolu.

## Přečtené společné pokyny

Ve výchozím commitu jsem přečetl:

- `AGENTS.md`
- `CLAUDE.md`
- `README.md`
- `Docs/ENVIRONMENT.md`
- `Tasks/README.md`
- `Tasks/INDEX.md`
- `Tasks/0001/BRIEF.md`
- `Tasks/0001/PROGRESS.md` a `Tasks/0001/HANDOFF.md` (připravené výchozí stavy)
- `Tasks/Templates/PROGRESS.md` a `Tasks/Templates/HANDOFF.md`

## Ověření před začátkem

- Zadání je dostupné na vzdálené větvi `codex/shared-agent-workflow`.
- Vzdálená větev `task/0001-github-handoff` před začátkem neexistovala.
- Git 2.56.0 funguje. `git push --dry-run` na pracovní větev prošel s uloženým
  přihlášením a hesla ani tokeny jsem nevypisoval.

## První průběžný checkpoint

- Obsah: tento soubor (bez této kapitoly) a PROGRESS ve stavu IN_PROGRESS.
- Odeslání: PROŠLO normálním `git push -u origin task/0001-github-handoff`.
  Šlo o novou vzdálenou větev, bez force push.
- SHA prvního checkpointu: `2e196dcd8c7f5a333934dc88ca4ed61824299a21`
- Kontrola: lokální `HEAD` se shodoval s
  `git ls-remote origin refs/heads/task/0001-github-handoff`.

## Druhý checkpoint

Obsahuje toto doplnění RESULT, vyplněný HANDOFF a PROGRESS ve stavu
READY_FOR_REVIEW. Jeho SHA zde záměrně není (kruhový odkaz). Je to poslední
commit vzdálené větve `task/0001-github-handoff`.

## Postup pro Codex

1. `git fetch origin task/0001-github-handoff`
2. Poslední checkpoint: `git rev-parse origin/task/0001-github-handoff`
   nebo `git ls-remote origin refs/heads/task/0001-github-handoff`.
3. Přečti soubory přímo z vzdálené větve, bez přepínání své pracovní kopie:
   - `git show origin/task/0001-github-handoff:Tasks/0001/RESULT.md`
   - `git show origin/task/0001-github-handoff:Tasks/0001/PROGRESS.md`
   - `git show origin/task/0001-github-handoff:Tasks/0001/HANDOFF.md`
4. Rozsah změn ověř příkazem
   `git diff --name-status e097331d24e234e3997cd1fccba333521e4ad57d origin/task/0001-github-handoff`.
   Očekávané jsou jen tři soubory v `Tasks/0001/`.
5. Historii ověř příkazem
   `git log --oneline e097331d24e234e3997cd1fccba333521e4ad57d..origin/task/0001-github-handoff`.
   Očekávané jsou dva commity úkolu 0001.

Zda Codex výsledek skutečně stáhl a přečetl, ověřuje Codex sám. Pracovník
to nevydává za otestované.

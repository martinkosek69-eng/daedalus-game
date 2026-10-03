# 0005 handoff

- Result: READY_FOR_REVIEW
- Handed over by: Claude Code
- Branch: `task/0005-sto-flight-reference` (target `codex/solar-flight`, not merged;
  no pull request created)
- Base: `d492d94ea2e522c1651461f898ba1426bd58df01`
- Verified remote checkpoints:
  - first `1e12d003a8e7d61ffff5b7de075a1ce85f96aa9a` (start, IN_PROGRESS)
  - second `a6bafd28616f94abb590ebd040943fbb282408a2` (draft research)
  - final: the last HEAD of the branch after this commit

## What was delivered and where

All files are pushed. Nothing is left only locally.

- `Docs/Research/STO_FLIGHT_CLAUDE.md`, sections:
  - 1: summary
  - 2: sources with evidence levels (OFFICIAL / DEV / COMMUNITY / TERTIARY)
  - 3: findings by topic, marked confirmed / community / inferred / unavailable
  - 4: tunable ranges for the 600 m Daedalus lab
  - 5: automatable acceptance checks A1–A12
  - 6: Czech beginner playtest checklist
  - 7: limitations
- `Tasks/0005/PROGRESS.md` and this file.

## How to read it

`git fetch origin task/0005-sto-flight-reference`, then
`git show origin/task/0005-sto-flight-reference:Docs/Research/STO_FLIGHT_CLAUDE.md`.
Scope check:
`git diff --name-status d492d94ea2e522c1651461f898ba1426bd58df01 origin/task/0005-sto-flight-reference`.
Only the three allowed paths should appear.

## Checks and evidence

| Brief requirement | Result | Evidence or limitation |
| --- | --- | --- |
| Restricted scope (3 allowed paths) | PASS | diff against base |
| No application contention | PASS | Unreal, Blender, builds and the game were not started. Research used web search, web fetch and the Claude built-in browser only |
| Primary sources | PARTIAL | 3 official ship-stat pages (S1–S3) and 2 developer interviews (D1–D2); no official manual or controls page found |
| Direct gameplay evidence with timestamps | NOT DONE | STO not run and video not measured; a measurement plan is in section 7 |
| Confirmed vs inferred separation | PASS | legend in section 3; wiki and guide formulas are not presented as developer code |
| No copied game assets | PASS | text references and links only |
| Tunable ranges and Czech checklist | PASS | sections 4 and 6 |
| Retrievable checkpoints | PASS | three pushed commits, local SHA = remote SHA after each push |

## Origin of material

Own text. External facts are linked in section 2. Community wikis are under
CC BY-NC-SA; facts are paraphrased and no wiki text blocks were copied.

## For Codex

- Confirm whether the 0006 lab uses W/S for pitch (STO default) or for thrust
  (see section 4, "WASD versus STO's W/S pitch").
- Max speed has no STO m/s equivalent. Choose it from the target approach time
  to Earth (formula in section 4).
- Open questions that only direct STO observation can answer:
  - bank angle
  - whether pitch auto-levels
  - the true pitch limit (community says about 75°)
  - acceleration and slide time constants
- Two blocked sites (stowiki.net, official forum) may hold more detail. I did
  not bypass their bot checks.
- READY_FOR_REVIEW is not acceptance.

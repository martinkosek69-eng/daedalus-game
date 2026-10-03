# Current foundation state — 2026-10-03

The implemented foundation is ACCEPTED by the coordinator on
`codex/game-foundation`. It is a reviewable delivery; main has not been merged.
Use this branch and its latest published commit when resuming foundation work.
Read AGENTS, FOUNDATION, ARCHITECTURE, CODING_RULES and task handoff before edits.

## Verified

- Unreal 5.8 editor and Windows Development game build/package succeed.
- Eleven named C++ automation groups: all succeeded, zero failures/unrun/in-process.
  Includes a 2002-system catalog, invalid-state rollback, additive catalogs,
  a 300-instance spawn burst, fixed clock/backlog, combat/body obstruction,
  travel/transport history, a guard pilot and recoverable disk generations.
- Two separate packaged processes load staged data and retain identity, damage,
  time and location across save/restart/load. Twenty subsequent system changes
  retain 21 live world Actors, rather than accumulating each visited scene.
- Rendered packaged controller-input probe passes movement, fire, travel,
  transport/return, save/load and pause, plus post-start dynamic ship presentation.
  Actual space and location screenshots were inspected. Geometry is deliberately
  synthetic; this is a technical scene, not approved finished art/controls.
- Background Blender saved/reloaded the editable 120 m original fixture and GLB;
  Unreal independently verifies 12000 cm and source BaseColorFactor. Repeated
  asset preparation preserves the existing map. Five required binary assets are
  published via LFS and verified through a fresh remote store with SHA256.
- Matching-checkout editor MCP starts with an explicit launcher flag; actual
  PIE start, running-state query and stop succeeded with the new foundation.
- The Windows PowerShell used by the user launcher also passes packaged restart.
  Game/user/save/automation output paths use the A-based checkout/configuration.

No project test failures remain. The installed compiler emits an engine warning
that it is newer than the preferred toolchain; build/test success is verified,
not a claim of unlimited compatibility. Packaged rendering emits short shader
preload warnings; these are not failed checks. Future engine/tool upgrades need
review and another full validation run.

## Play and continue

Double-click `Tools/SPUSTIT_ZKUSEBNI_HRU.cmd`, or run
`./Tools/Invoke-Foundation.ps1 Play`. The local package is under
`.local/foundation/Build/Windows`; builds and player saves are not Git content.
WASD/QE move, Space fires, T changes the test system, B transports/returns,
F5 saves, F9 loads and P pauses. See TESTING for reproduction commands.

The simulation and presentation are distinct. Current ship visuals register two
fixture types in the adapter; a new production mesh needs an explicit reviewed
mapping there. This small mapping can be externalized when real asset catalogs
arrive. Rich missions/economy/crew, walkable interiors, shuttle flight, realistic
orbits, offscreen events and planetary streaming are future implementations.
A 2002-definition test is not a performance promise for 2002 detailed battles.

A Claude introductory report arrived through GitHub while foundation work was in progress. See AgentChecks/COORDINATOR_REVIEW.md: file/tool fallbacks are reported successful, while live client connections need verification after session restart. Task 0001 still waits for a separate user start. The assistant's original worker checkout/branch is
retained locally; the corrected, accepted module is published in the coordinator
branch. Source delivery does not depend on publishing that historical branch.

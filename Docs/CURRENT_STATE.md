# Current development state — 2026-10-04

## Current six-system lab0016

Singleplayer Windows delivery on `codex/solar-flight`, main unmerged. Launch
Tools/SPUSTIT_LET_DAEDALA.cmd after packaging. M opens the3D galaxy/database;
right orbit, middle pan, wheel zoom, top/side/Home, search and deep planet focus.
Expanded Sol506 plus five accepted Claude systems63 give569catalog entries
and167physical bodies. Unknown sizes/positions are explicit; no fake measurements.
Four giant rings, sparse populations and faint zodiacal cue use sourced/labelled
bounds. Galaxy is authored, systems fictional, source epoch static/illustrative.

Reviewed3effaca Daedalus,38turrets,39details,62mounts,6glows. Fullspeed2.5s,
no side drift, smooth limits, bright sky/motion cues and native4K/no temporal blur.
Planet surface residency and texture pool were checked after actual rendering;
map labels no longer overlap deep body detail. Navigation selection leaves flight
unchanged; distances/current/fullimpulseETA and required speed are read-only.
Explicit TEST reposition permits scene testing; actual hyperdrive is future work.

All19tests, packaged3950frame4K controller/render, all5systems/two planets each,
scene return/cleanup, source/unit/210imported texture bindings and separate-process
foundation save/load PASS. Paths and details: Tasks/0016/HANDOFF.md.
All696 remote LFS objects downloaded and SHA256/size checked at e3ecef6.
Repeat asset preparation used the verified cache. See Tasks/0016/DELIVERY.md.
No new flight-lab persistence, combat, energy or hyperspace visuals. Performance
and style/flight feel await user assessment; fixed-time probe is not an FPS claim.
Claude0015 accepted; monitoring paused. Sources/shared instructions are resumable.

## Historical comparison build

## Visual correction in task0009

The user rejected the pale photo-repaint presentation and requested the original
web model/lighting/controls for comparison. Technical acceptance of0008 did not
constitute user approval of art. Its saved source and branch remain intact.
Task0009 restores a separately generated original hull, per-pixel web armor,
compact HUD, direct manual orbit and no temporal/motion-blur smear. Its separate
package is Build-WebReference, retained separately; the current launcher uses0010.
All62 tracked source/assets were independently downloaded from GitHub and
checked by SHA256/size after publication ca31f8e. Final package, all15 groups,
930-frame input/render checks and two-process
foundation restart pass. User art/control judgement remains pending; see
[task0009 handoff](../Tasks/0009/HANDOFF.md).

## First solar flight lab

`codex/solar-flight` adds a reviewed Windows playable scene using the recovered
600 m Daedalus, Earth, Sun,8920 prototype HYG stars and STO-inspired assisted
flight. See [SOLAR_FLIGHT](SOLAR_FLIGHT.md) and [task0004 handoff](../Tasks/0004/HANDOFF.md).
The final15 automation groups succeed (11 foundation +4 flight), packaged
controller/render probe passes and actual Earth/turn/Sun images were inspected.
The new IoStore package also passes the foundation's separate-process restart
write/read checks. Repeat asset preparation hits SHA256 cache and preserves the
existing SolarFlight map. Flight-lab pose is intentionally not saved yet.

Task0005 research is accepted with explicit source/measurement limitations.
The broader STO research is advisory, not approval of future combat or UI.
Task0007 original model recovery is technically verified. Task0008 photo-based
paint, separate lights, six engine effects and58 measured weapon/bay points
arrived through two GitHub checkpoints and are now accepted/integrated; see
[coordinator review](../Tasks/0008/REVIEW.md). Moving turret geometry/firing
remain future work. Source coordinates map Blender +Y port to Unreal -Y.
Final render/input probe also checks throttle-dependent glow and mouse orbit.
All51 tracked source/assets were independently fetched from remote LFS and
checked by SHA256 after final integration publication.
Codex owns source integration and Unreal after Claude's explicit handoff.
Use the task index and handoff when resuming. Main remains unmerged.

## Existing accepted foundation

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

A Claude introductory report arrived through GitHub while foundation work was in progress. See AgentChecks/COORDINATOR_REVIEW.md: file/tool fallbacks are reported successful, while live client connections need verification after session restart. The separate task 0001 is now ACCEPTED: Codex retrieved both checkpoints directly from GitHub and verified the permitted text-only scope (Tasks/0001/REVIEW.md). The assistant's original worker checkout/branch is
retained locally; the corrected, accepted module is published in the coordinator
branch. Source delivery does not depend on publishing that historical branch.



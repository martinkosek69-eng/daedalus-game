# Current development state — 2026-10-04

## Current integrated six-system lab 0026

Normal launcher: Tools/SPUSTIT_LET_DAEDALA.cmd → Build-Aurora26, retaining
the existing PlayerData. Build-Integrated25 and earlier packages remain available.
Completed0023 planet branch throughaa9072c and latest0008 ship branch
through7c35635 are reviewed/integrated. Shared source/publication branch remains
codex/solar-flight; no merge into main. See Tasks/0026/HANDOFF.md.

### Aurora and clickable pause menu 0026

Reviewed Blender source a193736 is preserved, exported with its original paint,
three materials, an 8K colour atlas and 4K other maps. The 3500 m source scale
is provisional, not a verified canon size. The catalog keeps Daedalus tuning
unchanged; Aurora turns more slowly but has a stronger drive (350 km/s normal
impulse, 2 s acceleration, 0.999c full sublight as a game balancing choice).
Actual hull radius fits its conservative 1950 m clearance sphere.

P opens the clickable paused menu; select Daedalus/Aurora there. Switching
preserves flight state and navigation, adjusts the camera/HUD, and rejects an
unsafe larger hull or incompatible attitude. Menu includes resume/exit,
read-only graphics settings and clearly unavailable flight save/load entries.
ESC closes menu/map; outside them the lab's previous quit shortcut remains.
Final game's ESC menu binding is deferred. No combat or new persistence added.

Build/package and all 23 automation tests passed. Final packaged 3840×2160
checks passed via actual mouse input: menu navigation, paused moving swaps,
full texture residency, assisted flight, Daedalus lights/H doors restored and
quit button exiting the process. Five renders written and inspected. Subjective
flight/appearance evaluation remains with the user. See Docs/AURORA.md and
Tasks/0026/EVIDENCE.json. All owned applications have exited; slot is released.

### New HUD design0024

User rejected both the first monochrome/card HUD and its minimalist revision.
Current STO-inspired instrument design has bevelled technical frames, detailed
weapon displays from real model positions, circular green ship outline and
four preserved blue shield arcs. Radar remains unframed/green. Copper turrets,
amber missiles/drive, blue beams and neutral text remain. Energy secondary,
no key hints or slider. Complex instrument treatment is now the requested style.
User approved the instrument design and authorized game integration. Runtime
sources now move DrawHUD to SolarHUD.cpp, align the lower panel sizes and add
a read-only live local minimap from canonical metre coordinates/heading.
Existing drive/navigation/pause/safety readings remain, other new systems
are placeholders with no actions. Flight/domain rules remain unchanged.
Editor build,20/20 automation tests and separate Windows package passed.
Native3840x2160 HUD/ship/planet/map rendering and real input/animation checks
passed; inspected over bright Earth and black space. Labels fit; a dark ship
instrument face preserves green outline contrast, minimap stays transparent.
See Docs/HUD.md. Most subjective evaluation remains with the user.
Historical standalone design: Tasks/0024/Preview/hud-concept.html.

### Latest ship0008

Exact600m original hull and newer darker plating, silo hatches, hangar details,
windows/masts,engine cores and source light geometry retained. Four separate
hangar door halves animate via H (2s travel); source beacons blink5s/on1/3s;
six engine effects follow throttle. Four bay/six engine point lights follow
the hull frame; bay intensity calibrated to game exposure. No duplicate added
turret barrels, no new firing or shuttle logic. Source textures/metadata,
runtime binding and coordinate frame checked. See Docs/SHIP_PRESENTATION.md.
All coordinator test applications exited; ownership is free after0025.

### Shared 4K requirement

All current and future visuals/UI target sharp native 3840x2160 or higher,
with compact readable HUD/map/text for a 32-inch monitor. This is a recorded
user requirement, not a claim that all current visuals meet it. See
Docs/VISUAL_QUALITY.md. Existing planet diagnosis0022 must evaluate against
this target; its independent proposal-only scope is unchanged.

### Independent planet-quality proposal0022

User now authorized implementation in0023: codex/claude-0023-planet-upgrade,
Claude owner, B plus targeted better real source maps, Earth-first then the
current six-system bodies at native4K. Delivery completed/reviewed/integrated:
105 textured bindings,10 cloud layers,3 relief maps,5 night maps and4 masters.
Source16K Earth/real clouds/relief and selected better moon maps are included.
Coordinator ran the required full recipe and read-only validators. Retain
the worker's1440px small-moon/tile/cloud-date limits; do not describe every
body as real4K imagery. Editor/build released. See Tasks/0023/REVIEW.md.

Claude delivered independent read-only diagnosis at c2ccd5d on
codex/claude-0022-planet-quality. Codex reviewed and integrated the proposal
and evidence; see Tasks/0022/REVIEW.md. Accepted as analysis only, with runtime
and numerical qualifications. The proposal itself contains no game changes;
implementation is now separately authorized in0023.
Recommendation: B layered materials/clouds plus targeted better real source maps,
Earth-first at native4K. The user selected proceeding with Claude in0023.
Claude never started Unreal and released editor/build ownership. Keep the
approved ship/sky/flight intact. Brief, proposal and comparison: Tasks/0022.

### User refinement0021

User approves0020ship sharpness. Runtime sky now hides the old photographed
M_Sky sphere: black background and separately generated HYG points only, with
web magnitude curve and no bloom halos. See Docs/POINT_STAR_SKY.md for NASA
references and human-view artistic limits. Galaxy map is unchanged.
Quality-first default disables texture streaming and forces highest mesh LOD.
Selective residency remains available only if streaming is enabled later.
Actual-speed steering now blends15.66°/s and36°bank at low speed to10.8°/s and
21.6°bank at full speed; angular response28.8°/s² is20%higher. No drift is retained.
Shared data remains singleplayer canonical; new curve parameters are validated.
Verification/publication is recorded in Tasks/0021/HANDOFF.md; user does most
visual/performance evaluation, as requested.

### User feedback correction0020

User rejected softness after0016. Current package enables Windows high-DPI
before window creation, applies native rendering after user settings, removes
FXAA/temporal/motion/DOF/fringe blur and keeps ship/sky textures fully resident.
Up to four large nearby celestial discs are promoted by angular size during
ordinary flight, independent of inspection/selection. Bloom/star footprint reduced.
R starts/stops normal impulse; Shift+R toggles full sublight250000km/s. All three
drive modes switch in flight without velocity/pose jumps; braking authority is
retained while returning to a slower mode. Navigation ETA uses full sublight.

Editor build,19automation groups and Windows package pass. Targeted1650frame
packaged input/render check passes using a copy of actual player settings and
the normal native launcher path:2560x1440 reported by Windows, not4K. Ship2K,
Earth8K and sky8K textures fully resident near/far; R/Shift+R/map suppression and
braking checked. No broad repeat of the prior six-system suite; user requested
efficient work and will do most visual/play testing. See Tasks/0020/HANDOFF.md.
Native unfiltered edges may alias; aesthetic acceptance and real4K user run pending.

### Previously completed0016 baseline

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



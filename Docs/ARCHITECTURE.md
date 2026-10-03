# Architecture v1

## Ownership and dependencies

`DaedalusSimulation` is a runtime C++ module depending on Core and Json only.
It contains ordinary value types, validated catalogs and the authoritative
`FSimulation`. It has no world Actors, asset references, input devices or UI.
`Daedalus` depends on it; the reverse dependency is forbidden.

`UDaedalusWorldSubsystem` belongs to GameInstance and survives scene lifecycle.
It owns one domain simulation for a singleplayer session. Scene Actors are
disposable presentation. No Actor pointer or asset path is a saved identity.
`ADaedalusGameMode` updates simulation and constructs only the active view.
`ADaedalusPawn` sends input; `ADaedalusHUD` reads committed state. Basic guard AI
commands use the same domain authority and do not modify state through visuals.

World, ships, combat and persistence are logical boundaries within these small
modules. Extract additional physical modules/plugins when dependency or reuse
requires it, rather than creating an unused framework per possible future idea.

## Definitions, instances and presentation

Canonical version-1 JSON at Content/Data/foundation.json defines galaxies,
systems, bodies, weapons, ship types, visitable locations and initial instances.
Every definition and instance has a stable namespaced string ID. Display names
may change. IDs and save schema changes need an explicit migration decision.
Catalog loading validates into a temporary catalog before replacing the old one.
Packaged games stage the same JSON as UFS content. There is no second editable
Data copy; all staged assets and runtime-load-only meshes must be cooked.

Runtime instance state contains local position/velocity, hull, shield, energy,
weapon cooldown, system identity, player identity/location and simulation clock.
Save restoration is validated transactionally; unsupported versions/references
leave the live world intact. Compatible newly added initial instances are
initialized without resetting saved instances. Dynamic spawn IDs are unique.
Removing/changing referenced definitions requires migration, never silent repair.

Art and scene construction use definitions/state, never determine weapon damage
or the authoritative simulation position. Source .blend and the engine import
are separate deliverables connected by reproducible export/import tooling.

## Coordinates and clocks

Strategic positions are galaxy-relative light-years plus galaxy ID. They are
never added to metre-scale simulation vectors. System-local simulation uses
double-precision metres and metres/second. Presentation subtracts its local
origin then converts metres to Unreal centimetres. +X forward, +Z up; one Blender
metre imports as 100 Unreal centimetres. The dedicated import checks length.
Cross-system travel changes the address, not a huge global physical position.

Simulation uses fixed 1/60-second steps. Real frame partitions should produce
the same result for the same command timeline. A bounded number of steps per
Advance preserves backlog; pause queues no new time. Backlog and unsupported
input must be visible to diagnostics. This is not a promise of unlimited CPU
capacity. Introduce interpolation/adaptive scheduling based on measured needs.

The separate solar flight laboratory uses `FFlightModel` at 1/120 s for assisted
steering and swept surface safety. Its authoritative state is a value owned by
the solar adapter; it has no persistent world identity yet. It does not replace
or write the foundation subsystem's saved ship state. Integrating the chosen
flight tuning into persistent world ships is an explicit future change; do not
keep two independently editable persistent versions of the same ship.

Inactive-system ships currently retain their state and do not run detailed AI,
movement or regeneration. Offscreen fleets/economy will need a separate coarse
time/event policy, not full scene Actors in every system. Per-system ship indexes exclude remote instances from each detailed step; active queries and
hashed ID lookup are the initial tools; a spatial index comes after measurement.

## Minimal interaction contracts

Movement, weapon fire, spawn, travel, transport and return commands validate
references and state. A destroyed ship cannot act. Beam damage consumes energy,
honours cooldown/range/planet obstruction, and transfers excess shield damage
to hull. Coarse swept sphere contacts are a foundation guard, not final collision
geometry or calibrated flight physics. Transient control inputs are not saved.

Test travel switches immediately between known systems at a validated arrival anchor.
It is a lifecycle test, not finished hyperspace travel. Authored locations use a
separate view while the ship remains a separate state object in its system.
Outgoing transport range, movement parameters and regeneration are synthetic test values. Return is an unlimited diagnostic rescue, not final transporter rules. Initial NPCs use a bounded hold-position pilot; richer decisions remain extensible.
Shuttle flight, walkable interiors, missions, factions and rich NPC decisions
are later extensions using stable identities/commands and documented interfaces.

## Persistence and extensions

The small snapshot is wrapped in a bounded JSON v1 envelope with generation and checksum.
Two verified rotating files retain a previous generation. Write/readback occurs
before replacement; loading picks the newest compatible valid generation.
Cross-process directory mutex prevents simultaneous read/write transactions.
Unsupported recognizable envelopes/definitions are protected from overwrite.
This is recoverable generations, not guaranteed durability against every power
loss or storage failure. Independent backups remain necessary.

Initial synchronous saving is appropriate for the measured small snapshot. Before
large saves, use immutable snapshots and asynchronous IO with explicit lifetime
and completion handling. Save services must not depend on a live scene actor.
The launcher supplies an explicit save/user directory next to A-based work data.
Tests use unique directories; PIE must have isolated saves. Never use player saves
for automation. A save format migration must have archived fixtures and tests.

Procedural generation later needs stable seeds AND generator versions. Freeze
visited-world definitions or migrate them explicitly; reseeding must not erase
player history. Graphics presets and detail budgets are configurable; catalog
size does not imply simultaneous detailed battles or fully realized planets.

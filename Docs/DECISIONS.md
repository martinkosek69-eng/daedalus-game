# Foundation decisions — 2026-10-03

## Confirmed scope

Pure singleplayer; long-lived large space game; multiple galaxies/systems;
capital ships normally stay in space. Transport visits authored environments;
shuttles are a later extension. The old prototype is inspiration, not mandated
source code, content or balancing. Codex integrates; helpers take individual tasks.

## Chosen approach

Keep Unreal and Blender. Build a small independent C++ domain module plus Unreal
adapters. Text catalogs, stable instance identities and versioned save snapshots
make changes reviewable and reconstructable. Use built-in GameInstance subsystem,
bounded versioned JSON saving, cooking/staging and ordinary scene components. Native SaveGame deserialization asserted on deliberately corrupt test input before validation; the JSON envelope avoids that dependency. Add further plugin or
data-oriented simulation systems only when profiling/requirements justify them.

Large world coordinate support is useful, but galaxies never share one physics
scene. Local precision, per-system state and bounded active presentation solve
different problems. World Partition may support authored terrain later; it is
not a ready-made astronomical universe or spherical planetary renderer.

Foundation rendering disables costly ray tracing/Lumen/virtual shadows for
repeatable baseline checks. Future art quality profiles can enable features and
measure their cost; these settings are not a permanent visual ceiling.

## Research and alternatives

Patterns studied in upstream sources; no GPL source code or art copied:

- [Pioneer Frame](https://github.com/pioneerspacesim/pioneer/blob/master/src/Frame.h):
  layered local frames and separate interpolated render coordinates. Adopt the
  separation; its full rotating/orbital physics is outside this initial scope.
- [Pioneer Space](https://github.com/pioneerspacesim/pioneer/blob/master/src/Space.cpp)
  and [SaveGameManager](https://github.com/pioneerspacesim/pioneer/blob/master/src/SaveGameManager.cpp):
  reconstruct objects/references before normal use and version save formats.
  Our two-generation write is our own recovery design, not a copied guarantee.
- [Celestia StarDatabase](https://github.com/CelestiaProject/Celestia/blob/master/src/celengine/stardb.h):
  catalog membership differs from nearby/visible object selection. Indexed
  lookups now, spatial partitioning later if needed. Celestia is visualization,
  so it does not establish combat or persistent-game requirements.

Primary sources inspected 2026-10-03. Future upstream changes need fresh review.
Epic references: [subsystems](https://dev.epicgames.com/documentation/en-us/unreal-engine/programming-subsystems-in-unreal-engine),
[save/load](https://dev.epicgames.com/documentation/en-us/unreal-engine/saving-and-loading-your-game-in-unreal-engine),
[LWC](https://dev.epicgames.com/documentation/en-us/unreal-engine/large-world-coordinates-in-unreal-engine-5),
[packaging](https://dev.epicgames.com/documentation/en-us/unreal-engine/packaging-your-project).

Deferred deliberately: multiplayer, GAS/Mass adoption, whole-planet streaming,
full realistic orbital physics, final mission/economy/crew systems and final art.
If scope changes, record the tradeoff and validate a small experiment first.


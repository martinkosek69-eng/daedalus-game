# 0008 — Daedalus engine-glow mesh and mounting manifest

Owner: Claude Code after user relays this task. Work branch:
task/0008-daedalus-engine-effects. Base: first published codex/solar-flight
commit containing this brief, record its full SHA. Target codex/solar-flight.
Use own clean checkout on A:. Read shared docs and this brief first.

Application ownership: Blender BACKGROUND only, exclusively assigned to Claude
for this task. Codex has completed Blender source preparation and now uses
Unreal/builds. Do not open/control Unreal, compile, or edit source ship binaries.
No GUI scene ownership. Tools/config paths remain ignored local settings.

Allowed paths ONLY: Art/Ships/Daedalus/EngineEffects/**,
Tools/Prepare-DaedalusEngineEffects.py, Tasks/0008/PROGRESS.md and HANDOFF.md.
Temporary render/check outputs .local/engine-effects. Do not change ship source,
flight code, rules, INDEX or other tasks. No merges or force pushes.

Deliver a small optional source mesh for visually glowing engine apertures on
our actual Daedalus, not a replacement hull or redesigned engines. Read-only
reference: Art/Ships/Daedalus/Daedalus.blend, Daedalus.glb and metadata.
Coordinates are +X forward, +Z up, Blender metres; hull length600m. Inspect actual
rear geometry to locate true engine outlets; do not guess their centers from
bounding box corners. If uncertain record evidence and omit uncertain outlets.

Outputs: editable .blend, one GLB containing a joined glow mesh with one
emissive cyan material; ENGINE_MOUNTS.json with unique mount IDs, centerMetres,
outwardAxis, aperture dimensions, evidence and orientation. These are local hull
coordinates. Keep glows just outside rear faces so hull does not occlude them;
avoid extremely long flames (electric/impulse effect, not a rocket plume).
Source notice must retain model credit Astrofossil CC BY-NC4.0 for derived
placement; original effects authored for this noncommercial prototype.

Provide reproducible Blender script using bpy and save explicitly. Reopen saved
source, verify mesh count/dimensions and all outlets against reference, render
rear and side previews with hull loaded read-only; keep previews ignored.
Export must contain only effect mesh, no duplicate Daedalus. Coordinator will
import and vary emissive intensity with throttle; do not make Unreal materials.

At least one intermediate commit/push plus final READY_FOR_REVIEW handoff.
Acceptance: permitted scope, tiny reviewable mesh, correct units/positions,
saved/reopened sources, export+render inspection, retrievable LFS objects,
honest uncertain mounts. Optional enhancement; does not gate initial flight.

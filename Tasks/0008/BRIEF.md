# 0008 — Reference-based Daedalus detailing, materials, weapons and engines

Owner: Claude Code after user relays the updated task. Branch:
task/0008-daedalus-engine-effects. Base: initially65d1f09 on codex/solar-flight;
if already started retain that base and record this updated brief's commit as
additional instruction. If not started use latest published codex/solar-flight
containing this revised brief and record full SHA. Target codex/solar-flight.
Own clean checkout on A:. Read shared instructions and this brief first.

UPDATED USER SCOPE: repaint and detail the actual Daedalus in Blender using
internet photographs/stills of the ship. Current coloring is not satisfactory.
Include engine outlets, turret/gun locations, forward missile silos and other
reference-visible weapon zones. Codex develops flight/scene independently.

Ownership: Claude exclusively owns background Blender and source binaries in
Art/Ships/Daedalus (except read-only Original). Codex does NOT edit those sources
until explicit handoff; its Unreal imports currently use the published baseline
copy. Do not start/control Unreal, builds or Codex's working copy. No GUI scene
ownership implied. Installation/config remains local/ignored.

Allowed: Art/Ships/Daedalus/** EXCEPT Original/** (read-only);
Tools/Prepare-DaedalusDetail.py and Prepare-DaedalusEngineEffects.py;
Tasks/0008/PROGRESS.md and HANDOFF.md. Do not edit existing
Tools/Prepare-DaedalusSource.py, gameplay, engine assets, other tasks or INDEX.
Do not merge or force push. Keep model600m,+Xforward,+Zup and compatibility.

## Deliverables

1. First study credible ship photographs/stills from several views. Record exact
source URLs and observation for hull palette, plating, bridge, engine apertures,
weapon locations. Distinguish visible evidence, uncertain hidden-side symmetry,
fan-art/model interpretations and your artistic approximation. Prefer actual
screen/production-reference images over another fan model. Do not redistribute
unlicensed reference photos or copy unrelated model assets into public repo.
2. Preserve recognizable prototype silhouette and original details while
correcting its materials, panel contrast, recesses, windows and observable
surface details. Avoid a flat white/gray result. Keep editability and renderable
PBR materials. The original exported mesh has noUVs; vertexCOLOR_0 + source base
factor is supported by Codex's Unreal material importer. Portable textures or
UVs are permitted if required, with dependencies and license/provenance.
3. Daedalus.blend + Daedalus.glb: one joined hull mesh with named materialslots,
applied metric transforms and600m length. Include your new weapon fittings or
separate export(s) clearly listed in HANDOFF. Do not weld later rotating turrets
into the only hull geometry: turret bases can remain static, moving barrels/
turrets must be separate named objects/export meshes with local pivot and axis.
4. WEAPON_MOUNTS.json: stable mountID, kind (turret,missile_silo,...),
centerMetres in shiplocal coordinates, forwardAxis/upAxis, allowed firing arc
if supported by geometry, intended moving mesh name, referenceURL and confidence.
Explicitly mark bow missile silo zones and reference-supported turret groups.
No made-up canonical counts: uncertain positions remain proposals. Units must
match export and the engine can later attach working turrets to these points.
5. Engine glow geometry/export + ENGINE_MOUNTS.json: actual outlet centers,
outward axes and aperture dimensions measured from mesh. Keep the effect small
and controllable by later throttle; no enormous rocket flame. Can be included
in source scene but export glow separately to avoid duplicate hull.
6. Reproducible bpy script(s), concise SOURCE/modification notice and updated
asset-metadata. Retain Astrofossil CC BY-NC4.0 and original-source record.
The old Prepare-DaedalusSource.py is baseline reconstruction only; document
that it must not overwrite your accepted detailed source. Do not depend on
private machine paths or undelivered textures.

## Checks / checkpoints

Save .blend explicitly, reopen, inspect export/import in Blender, check units,
length, mount axes, geometry bounds and material dependency counts. Render front,
rear, side/top views with a consistent neutral exposure to judge color; outputs
under .local/daedalus-detail and ignored. Record what cannot be confirmed from
available photos. No claims that Unreal or future guns already work.

Commit/push a useful checkpoint early, then final READY_FOR_REVIEW with source,
exports, mount manifests, checks and source links. Binary artifacts must be
uploaded through LFS. No main merge. Coordinator will review dimensions,
materials/import and rendering in its own Unreal copy before acceptance.

This enhancement is independent of initial flight code. A clean checkpoint can
be resumed after quota exhaustion; record incompleteness honestly.

Coordinator ownership transfer,2026-10-03: Claude handed off final991a823 as
READY_FOR_REVIEW. Codex retrieved both worker checkpoints and now owns source
review and integration in codex/solar-flight. Do not modify the reviewed source
concurrently. The user's preview GUI copy remains unrelated to source ownership.

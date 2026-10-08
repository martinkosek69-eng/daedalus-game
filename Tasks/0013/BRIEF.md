# 0013 — Review/import latest Claude textured ship
Owner: Codex ship-material helper. Base 4e2e7b6, branch codex/solar-flight.
Root commits/pushes/builds/imports; no Git writes or Unreal/editor calls.
Read shared README/ENVIRONMENT and foundation docs, Tasks/README first.
Allowed: Tools/Prepare-DaedalusMaterials.py, Tools/Validate-DaedalusDelivery.mjs,
Tasks/0013/PROGRESS.md, Tasks/0013/HANDOFF.md only.
Implement reusable Python assign_materials(root, unreal, tools, lib, mesh, filename)
for latest Claude glTF PBR materials/vertexcolors, PNG BaseColor+ORM+OpenGL Normal.
Stable output /Game/Ships/Daedalus/Materials and /Game/Ships/Daedalus/Textures.
Import PNGs through automated replace-existing AssetImportTasks, srgb correct,
normal flip green; no deletion of rooted expressions. One helper/module import
per commandlet; texture imports cache. For hull/addons/turrets only; effects/root
legacy fallback remain owned by coordinator. Fully save assets; no app launch.
Update Node delivery validator for 3 embedded images/UV/shared textures,38 turret
nodes/addons,62 weapon mount records including two bow rod tips outside hull.
Retain real geometry/axis/bounds/hash/source sanity; don't weaken checks broadly.
Run Node validator, Python syntax; document actual checks and remaining import
validation. Root imports module and owns runtime components/source pipeline.

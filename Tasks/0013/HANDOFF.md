# 0013 — material recipe and delivery validation

Status: READY_FOR_REVIEW. Base source: coordinator `4e2e7b6` integrating Claude
`d5846a1` and `3effaca`. Scope remains singleplayer presentation; no state or
flight data was edited.

## Changed files

- `Tools/Prepare-DaedalusMaterials.py`: callable
  `assign_materials(root, unreal, tools, lib, mesh, filename)`, returns the
  assigned mesh slot count. Root loads the module once with importlib, then calls
  it for the hull, add-ons and imported turret meshes after geometry import.
- `Tools/Validate-DaedalusDelivery.mjs`: source validation for the current
  textured delivery, replacing obsolete assumptions of zero images/58 mounts.
- This task's progress and handoff documents.

## Import behavior

The module preserves glTF base-colour factor multiplied by the albedo texture
and delivered vertex colour. Vertex colour is used only for source primitives
that contain COLOR_0; turret barrels correctly use their plain source factor.
Albedo is sRGB. ORM is linear mask compression, G roughness/B metallic with glTF
factor defaults of 1, R ambient occlusion. The OpenGL tangent normal is imported
with UE normal compression and green-channel flip. Source emission and its
KHR intensity multiplier are retained. Textures and materials have stable
`/Game/Ships/Daedalus/Textures` and `/Game/Ships/Daedalus/Materials` paths.

Textures are imported through automated replace-existing tasks and cached for
this commandlet. Identical source material recipes are shared between hull and
add-ons and reused across 38 turret nodes. Conflicting recipes sharing a name
are rejected. Existing rooted material expressions are retained; owned outputs
are reconnected and saved. Nanite is disabled for these imported meshes to
retain the full source model instead of a simplified fallback.

Effects and the old web comparison remain coordinator-owned. The helper did
not edit `Prepare-SolarContent.py`, import geometry or launch Unreal/Blender.

## Actual checks

`node Tools/Validate-DaedalusDelivery.mjs` passed, reporting 222330 hull triangles,
4 hull materials, 6 engine outlets, 62 weapon/bay mounts, 38 turret nodes and 3
plating textures. All original/source/export hashes match metadata. Embedded
PNG payloads in both hull and add-ons match the separately delivered2048² PNGs
byte-for-byte. Hull UV/colour accessor counts and actual axis-mapped dimensions
match the 600 m source. All turret node translations map to their distinct
railgun mount records. Only the two explicitly named rod tips may extend beyond
the original hull bounds; each is checked against its corresponding add-on
metadata, while all other mount bounds checks remain intact.

Python AST parsing passed for the module. This proves syntax only. No real
Unreal import, shader compilation or rendered appearance was tested by the
helper. Root must include module/source bytes in its content fingerprint,
import and save the real assets, cook the dynamic paths, and inspect the new
model in the packaged game before acceptance/publication.

# Daedalus prototype recovery

Original author: Astrofossil — SG BC-304 Daedalus,
https://www.thingiverse.com/thing:2256025 . Recorded source license:
CC BY-NC 4.0 https://creativecommons.org/licenses/by-nc/4.0/ .
Original/source metadata recovered from user's preserved web prototype, not a
claim of official production assets or a verified canonical600m dimension.
The model is used for this noncommercial fan prototype. No general license is
granted for franchise IP. Remote source page was unavailable during recovery;
the original attribution record remains under Original/daedalus-source.json.

Changes: converted prototype geometry from -Z forward to +X forward/+Zup;
joined original six damage sections, kept the silhouette and 222330 original
triangles; sampled runtime armor variation to vertex color, added hatch/vent/
window geometry and portable materials. Shader derivative seams and original
animated runtime propulsion are not baked into this static export. Exact
dimensions/counts/hashes are asset-metadata.json. 600m is a project tuning choice.
Tools/Prepare-DaedalusSource.py recreates source and export, explicitly saves
and reopens .blend. Unreal owns a material parent multiplying vertexCOLOR_0
with the source base factor; imported dimensions must be60000cm along +X.

## Detail pass (task 0008, Claude, 2026-10-03)

Modifications by this project to the CC BY-NC 4.0 work above (credit retained):

- **Hull = exact Astrofossil geometry.** The hull is rebuilt from
  `Original/daedalus.glb` (axis conversion and join only). The script asserts
  exactly 222330 triangles, 600 × 378.53 × 92.61 m, and the same count after
  re-importing the export. Proportions are unchanged.
- **Baseline fittings removed or split (user decision).** The baseline added
  14 dark hatch squares and 36 vent slats that are not part of the original
  model. At the user's request they are removed. The 30 baseline window boxes
  keep their positions but are no longer welded into the hull. They are the
  separate object `Daedalus_WindowLights` in `DaedalusLights.glb`.
- **New hull colour.** COLOR_0 now holds a reference-based palette (see
  REFERENCES.md): dark grey with an olive cast, lighter sunlit decks, dark and
  light plate patches, and dark greebles. Baked ambient occlusion adds recess
  contrast. The hull slot base factor is white, so the Unreal parent material
  can keep multiplying COLOR_0 by the base factor.
- **Material slots on existing faces.** The hull uses `Daedalus_Armor`,
  `Daedalus_EngineMetal`, `Daedalus_HangarInterior` (warm emissive bay
  interiors) and `Daedalus_LightWhite` (bow light strip). There are no texture
  files.
- **Effects outside the hull.**
  - `DaedalusEngineGlow.glb` holds six `<id>_Glow` objects sized from the
    measured outlets (`ENGINE_MOUNTS.json`). Each has its origin at the outlet
    centre and exhausts along local −X.
  - `DaedalusLights.glb` holds the window lights.
  - Both share the hull's coordinate frame, so they need no offset.
- **Weapon mounts.** Measured weapon and launch mounts are in
  `WEAPON_MOUNTS.json`, with matching `MOUNT_<id>` empties in the `.blend`.
  No turret or barrel geometry was added.

Reproduce with:

```
blender --background --factory-startup --python Tools/Prepare-DaedalusDetail.py
```

`Tools/Prepare-DaedalusSource.py` is the baseline reconstruction only. Do not
run it over this detailed source, because it overwrites the same output files.

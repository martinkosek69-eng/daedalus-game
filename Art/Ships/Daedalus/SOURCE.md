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

- **Geometry unchanged.** The hull is rebuilt from `Original/daedalus.glb`
  with the same steps and fittings as the baseline. The script asserts the
  baseline counts: 223458 triangles and 504598 vertices, 600 × 378.53 × 92.61 m.
- **New hull colour.** COLOR_0 now holds a reference-based palette (see
  REFERENCES.md): dark charcoal with an olive cast, lighter sunlit decks,
  dark and light plate patches, dark greebles. Baked ambient occlusion adds
  recess contrast. The hull slot base factor is white.
- **Material slots.** The baseline names stay first, then new slots:
  `Daedalus_EngineMetal`, `Daedalus_HangarInterior` (warm emissive bay
  interiors) and `Daedalus_LightWhite` (bow light strip). These are assigned to
  existing faces only.
- **Engine glow.** A separate effect, `DaedalusEngineGlow.glb`, with six
  `<id>_Glow` objects sized from the measured outlets (`ENGINE_MOUNTS.json`).
  It is not welded into the hull.
- **Weapon mounts.** Measured weapon and launch mounts are in
  `WEAPON_MOUNTS.json`, with empties in the `.blend`. No turret or barrel
  geometry was added.

Reproduce with:

```
blender --background --factory-startup --python Tools/Prepare-DaedalusDetail.py
```

`Tools/Prepare-DaedalusSource.py` is the baseline reconstruction only. Do not
run it over this detailed source, because it overwrites the same output files.

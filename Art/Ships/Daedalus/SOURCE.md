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
- **Hull look (series-matched).** After the user rejected the first pass (too
  light, cool and flat; orange too strong), the hull got:
  - **Plating texture.** `Textures/T_Daedalus_Plating_BaseColor/ORM/Normal.png`
    (2048², 32 m tile) are generated procedurally by the detail script (no
    third-party images). They are applied through a world-scale box-projected
    `UVMap`.
  - **COLOR_0.** It carries a neutral grey palette sampled from the on-screen
    still U13, plate patches and baked AO; the texture multiplies it (glTF
    rule).
  - **Hangars.** Dark plated hangar interiors.
  - **Engines.** The aft-facing faces of the nozzle turbine vanes and hub use
    `Daedalus_EngineInner` (warm-lit, yellow glow), so the turbine stays
    visible against the engine glow (U21).
- **Material slots on existing faces.** The hull uses:
  - `Daedalus_Armor` (textured),
  - `Daedalus_EngineMetal`,
  - `Daedalus_HangarInterior` (textured),
  - `Daedalus_LightWhite` (now the dark bow window band),
  - `Daedalus_EngineInner`.
- **Separate exports in the hull frame (no offsets).**
  - `DaedalusEngineGlow.glb` holds six `<id>_Glow` objects sized from the
    measured outlets (`ENGINE_MOUNTS.json`). Each has:
    - its origin at the outlet centre, exhausting along local −X,
    - a warm-white core disc behind the turbine vanes,
    - a faint haze,
    - a child point light in front of the vanes (KHR_lights_punctual).
  - `DaedalusLights.glb` holds:
    - the faint cyan windows (baseline windows, bow band row, bridge tiers),
    - pod spot and flood lights,
    - nav lights (starboard green, port red).
  - `DaedalusAddOns.glb` holds parts the stills show but the model lacks:
    - bridge masts,
    - two 40 m forward rods at the bow,
    - VLS hatch overlays like the close-up U16 (taupe doors, end tabs,
      checkerboard hazard stripes; `T_Daedalus_SiloHatch_*` textures),
    - hangar back walls with the split observation window like U17.

    This keeps Daedalus.glb the exact 600 m hull.
  - `DaedalusBeacons.glb` holds three orange mast-tip beacons. Object extras
    describe the blink: once every 5 s for 1/3 s. The .blend has a preview
    animation.
  - The model's own dome turrets (with their barrels) are kept. The twin
    barrels added in an earlier pass were removed at the user's request.
- **Look-dev.** The `.blend` has a `LookDev_Series` collection, matching the
  on-screen stills U13/U15 with the cooler daylight tone of U22. It is not
  exported. It contains:
  - a key sun,
  - a cool Earth/sky bounce,
  - a camera fill,
  - the camera `CAM_SeriesStill`.

  The scene uses a dark world and AgX Medium High Contrast.
- **Weapon mounts.** Measured weapon and launch mounts are in
  `WEAPON_MOUNTS.json`, with matching `MOUNT_<id>` empties in the `.blend`.
  They cover:
  - railgun domes,
  - bow VLS,
  - F-302 bays,
  - 4 Asgard beam weapons,
  - the two rod tips.

Reproduce with:

```
blender --background --factory-startup --python Tools/Prepare-DaedalusDetail.py
```

`Tools/Prepare-DaedalusSource.py` is the baseline reconstruction only. Do not
run it over this detailed source, because it overwrites the same output files.

# 0015 handoff

State: READY_FOR_REVIEW. This is not ACCEPTED; Codex tests the assets in Unreal
and integrates them.

- Worker: Claude Code. Branch `task/0015-five-original-systems`.
- Base: `4e2e7b6ad7fcb9a720f1937b86cc4704d875024c`.
- Last commit: see the branch history; the push report gives the SHA.

## Deliverables

| System | system.json | Planets | Moons | Other | Belts | Rings | Special |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Asterion (fic.asterion) | `Art/Space/Systems/Asterion/system.json` | 5 | 5 | 1 irregular protoplanet | 1 asteroid | Varunis | true double planet Tessaly + Tessa Minor; glowing lava moon Pyra; irregular Kestrel |
| Velara (fic.velara) | `Art/Space/Systems/Velara/system.json` | 5 | 5 | – | 1 asteroid | Corvan, Aurelion | ocean world Thalassa with a permanent superstorm; ringed rocky super-Earth Corvan; cracked-ice moon Brine |
| Nivara (fic.nivara) | `Art/Space/Systems/Nivara/system.json` | 5 | 5 | – | 1 icy | – | compact near 2:1 resonant chain; eyeball world Iris; lava world Ember; irregular moons Mote and Glint |
| Caelum (fic.caelum) | `Art/Space/Systems/Caelum/system.json` | 7 | 7 | – | 1 asteroid | Caelestis, Azurine | inflated hot giant Pyrrhus with a glowing night side; gapped rings; geyser moon; crimson-vegetation world Verdance; two-tone moon Nyx |
| Morava (fic.morava) | `Art/Space/Systems/Morava/system.json` | 5 | 8 | – | 1 asteroid + 1 icy (28–56 AU) | Brannock, Selk | elongated fast-spinning dwarf planet Selk (3.92 h, shapeScale 1/0.78/0.52) with a narrow ring and two moons; near-snowball Serein |

Each system directory contains:

- `system.json`: UTF-8; the brief's contract.
- `SOURCES.md`: provenance, the special feature, and a table of radius, orbit,
  Kepler period, day and tilt.
- `<Name>.blend`: export sources plus a clearly SCHEMATIC, animated preview
  scene. Textures are referenced relatively.
- `Models/UnitSphere.glb`: smooth 1 m sphere with equirectangular UVs and a +Z
  pole (128 × 64 segments, as SolarSphere).
- `Models/<irregular>.glb`: irregular unit bodies (Kestrel, Mote, Glint, Kite,
  Nam), bounds = max radius 1 m, with spherical UVs and an embedded material.
- `Models/BeltRock_A/B/C.glb` and/or `BeltIce_A/B/C.glb`: unit rock variants
  for the belts (each embeds its own 1024² texture).
- `Textures/*`, all equirectangular, +Z pole, u = longitude from −180°:
  - `<body>.jpg`: albedo; 4096 × 2048 for headline worlds, 2048 × 1024 for the
    rest,
  - `<body>_clouds.png`: RGBA cloud layer, alpha = coverage,
  - `<body>_night.jpg`: emissive lava or thermal glow,
  - `<body>_ring.png`: RGBA radial strip 2048 × 64, x from inner to outer.
- `preview.png`: render of the schematic preview.

Sizes on disk: Asterion 20 MB, Velara 23 MB, Nivara 18 MB, Caelum 22 MB, Morava 24 MB
(about 107 MB in total; .blend/.glb/.png/.jpg are stored in LFS).

Reproduce (all five systems, about 13 minutes on this PC):

```
blender --background --factory-startup --python Tools/Prepare-FiveSystems.py
```

`FIVE_SYSTEMS_ONLY=fic.velara` builds a subset. `FIVE_SYSTEMS_FAST=1` builds at
half resolution for iteration only; it must not be delivered.

## Data notes for the importer

- Positions are system-local double metres; the star is at [0,0,0]. Moons use
  absolute positions. Each planet is placed on a slightly inclined circular
  orbit at a fixed phase. Moons lie in the parent's equatorial plane, which is
  tilted by `tiltDegrees`.
- `rotationHours`:
  - Negative means retrograde (Calyx).
  - Tidally locked bodies use their orbital period.
  - Tessaly and Tessa Minor both use the pair period.
- Iris is an eyeball world. Its liquid ocean is centred on texture longitude 0
  (+X of the unit sphere). For a correct tidally locked look, orient that
  longitude toward the star.
- Selk's `shapeScale` [1, 0.78, 0.52] is a Jacobi ellipsoid from rapid
  rotation. `radiusMetres` is the long semi-axis.
- Rings lie in the planet's equatorial plane. `ringInnerMetres` and
  `ringOuterMetres` stay outside the planet radius and inside the innermost
  moon.
- Cloud layers are separate textures, so the engine can rotate them slightly
  faster than the surface, the way the .blend preview does. Night textures are
  emissive: Ember and Pyra lava, Cinder lava and the thermal glow of Pyrrhus.
- Belts: `count` 600, a fixed `seed`, and `kind` asteroid or icy. The matching
  unit rock variants are in `Models/`.
- Masses are not part of the contract. They are used only by the validator and
  recorded in SOURCES.md (Kepler periods).

## Checks run (Blender 5.2.2, background)

The script asserts the following and prints `FIVE_SYSTEMS_PASS`:

- **Contract shape.** Exactly 5 systems, unique IDs, valid parents and finite
  positive metres. Every system has 4–7 planets, at least 5 moons and at least
  1 belt. Four systems have rings.
- **No overlaps.** Each body's distance to its parent is greater than the
  parent's radius (or ring outer radius) plus its own radius. No body reaches
  0.2 × the star's radius.
- **Plausibility.**
  - Every moon lies inside 0.5 of its parent's Hill radius.
  - Adjacent planets are more than 8 mutual Hill radii apart.
  - Kepler periods are computed and listed per system in SOURCES.md. Nivara's
    chain is 23.8 → 47.8 → 102 → 204 h.
- **Textures.** Every texture referenced from the JSON was decoded. Planet
  maps are 2:1 and at least 2048 px wide; rings are 2048 × 64. There are no
  normal or ORM maps, so no sRGB/linear confusion is possible.
- **GLBs.** Every GLB re-imports with a maximum vertex radius of 1 m (± 1 %).
- **.blend.** Saved, reopened, with the unit sphere and preview present and all
  image paths resolving.
- **Previews.** The real preview images were inspected (see `preview.png`).

## Licences

All textures, meshes and data are original procedural work generated by
`Tools/Prepare-FiveSystems.py` for this project. No downloads and no
third-party images, models or catalogues were used. There is no licence or
attribution obligation beyond the repository's own terms.

## Known limitations

- The maps are procedural and stylised (fBm, craters, bands, storms); they are
  not photographic. Fine relief is baked lightly into albedo as hillshade.
  There are no normal maps.
- The preview is schematic: sizes are compressed and distances are not to
  scale. Rotation speeds are preview speeds.
- The eyeball ocean and the Iapetus-like dark hemisphere of Nyx assume an
  engine orientation for locked bodies (see above).
- Atmosphere colours are suggestions for a rim or scattering effect; the
  engine decides how to render them.
- Unreal was not used. Import scale, materials and the belt instancing are for
  Codex to verify.

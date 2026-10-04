# Solar source catalog and reproduction

Task0012 provides **37 named bodies**: Sun, eight planets, all24 moons from the
web reference, and Ceres/Vesta/Eros/Bennu. The single runtime catalog is
`Game/Daedalus/Content/Data/Solar/system.json`. Stable IDs are `sol.<web-id>`;
all radii/positions use metres and rotation periods use hours. Optional
`shapeScale` contains visual X/Y/Z multipliers; no visual mesh owns world state.

Required fields per body: `id`, `name`, `kind`, `parentId`, `radiusMetres`,
`positionMetres`, `texture`, `rotationHours`. Optional fields: `tiltDegrees`,
`atmosphereColor`, `cloudTexture`, `nightTexture`, `shapeScale`,
`ringInnerMetres`, `ringOuterMetres`, `ringTexture`, `surfaceQuality`.
The Sun's parent is empty; all planets/asteroids reference `sol.sun`, and moons
their parent planet. `asteroidBelt` defines deterministic decorative distribution
(600 instances, seed3040012, 2.1–3.3AU) separately from the named body catalog.
These instances are visual garnish, not another persistent catalog of ships/bodies.

The layout retains the protected web orbital phases and moon inclination0.12rad,
then rotates its orbital plane into Unreal XY and translates it. Earth is exactly
`[0,0,0]`; Sun remains exactly `[105781669000,-105781669000,0]` from the flight lab.
The orbital distance unit is that existing Earth–Sun distance, within about
0.00002% of one astronomical unit. It is a **fixed illustrative epoch**; no
current-date ephemeris, gravity or dynamic orbital evolution is implied.
Periods/radii were recovered from the original reference, whose sources include
[JPL planet parameters](https://ssd.jpl.nasa.gov/planets/phys_par.html) and
[JPL satellite parameters](https://ssd.jpl.nasa.gov/sats/phys_par/).

Named surfaces are mostly sphere textures. Oblateness uses the original web
equatorial/mean-radius recipe. Eros preserves the web's34.4 x11.2 x11.3km
approximation. Small irregular objects use the own generic rock. Surface safety
should consider the maximum visual axis, not only the nominal radius.

Textures are in `Art/Space/SolarSystem/Textures`; shared Earth maps stay in
`Art/Space/Textures`. New folder lookup takes precedence (notably the8k Sun).
See [complete attribution](../Art/Space/SolarSystem/SOURCE.md). Mercury/Mars/Moon
have8k color maps; Jupiter/Saturn/Sun/Venus4k, Uranus/Neptune2k. Provider8k-labelled
URLs for Jupiter/Saturn/Sun actually delivered4k images; dimensions were decoded.
Moon probe
maps range720–1440 pixels wide; these are honestly limited historical mosaics.
Five original schematic rock textures are2k, and Uranus ring strip4k.

## Reproduce without controlling any open application

Run the configured Blender executable in background with this repository as
the working directory; supply the protected web `dist` directory explicitly:

```text
<Blender> --background --factory-startup --python Tools/Prepare-SolarSystemSources.py -- --prototype <reference-dist>
```

The script parses the web data recipe, generates the canonical JSON and owned
rock/ring assets, saves and reopens the source Blender file, decodes all textures,
validates each reference and GLB structure, and refreshes `sources.json`.
It copies unchanged web textures only when missing and preserves existing
reviewed provider upgrades. A fresh checkout includes the exact higher-resolution
originals, so reproduction makes no network request.

To verify existing delivery without regenerating texture/geometry assets:

```text
<Blender> --background --factory-startup --python Tools/Prepare-SolarSystemSources.py -- --prototype <reference-dist> --verify-only
```

Verification covers all37 unique IDs, valid parents, positive radii, finite
positions, every texture dependency, dimensions, source/export hashes,
5120 exported triangles, no embedded/external image dependency in GLB, and saved
editable Blender source. Unreal import/material/collision/packaged rendering
checks belong to the coordinating task, not this source-only delivery.

# Solar System source assets

## Planet, Sun, Moon and Milky Way surfaces

Textures by **Solar System Scope / INOVE**, distributed under
[CC BY 4.0](https://creativecommons.org/licenses/by/4.0/).
[Provider catalog](https://www.solarsystemscope.com/textures/) verified 2026-10-04.
The supplied JPEG/PNG pixels are unchanged. Unreal shading, clouds, atmosphere,
ring presentation and exposure are project adaptations. No endorsement implied.

Current maps downloaded from the provider's public `textures/download/` directory:

- `8k_mercury.jpg`, `4k_venus_atmosphere.jpg`, `8k_mars.jpg`,
  `8k_jupiter.jpg`, `8k_saturn.jpg`, `8k_moon.jpg`, `8k_sun.jpg`,
  `8k_stars_milky_way.jpg`; the resolution prefix is removed in local filenames.
- `2k_uranus.jpg`, `2k_neptune.jpg`, `2k_saturn_ring_alpha.png` recovered unchanged
  from the protected web reference.
- Existing `../Textures/earth_daymap.jpg`, `earth_clouds.jpg`,
  `earth_nightmap.jpg` retain the previously downloaded 8k provider originals.
  They are shared instead of duplicated. See [existing sources](../SOURCE.md).

Actual decoded sizes are recorded in `sources.json`: Mercury/Mars/Moon/MilkyWay
are8192 x4096; Jupiter/Saturn/Sun and Venus are4096 x2048. The provider's8k-labelled
URLs for Jupiter/Saturn/Sun currently return4k files; filenames are not treated as
proof of image resolution. Uranus/Neptune are2048 x1024.

INOVE describes these maps as imagery/elevation-based visualizations with adjusted
colors and invented fill for unmapped areas. They are visual surfaces, not complete
scientific measurements. Giant planet/cloud appearances are representative, and
the Milky Way is an artistic exposure for a game sky.

## Moon maps from the original reference

`jpl-*.jpg`: **Courtesy NASA/JPL-Caltech**, with **Caltech/JPL/USGS** and
**David Seal** credited for the map mosaics/cleanup. These unchanged JPEG maps
come from the [JPL texture library](https://maps.jpl.nasa.gov/tmaps/), originally
downloaded from its `pix/` directory. The `jpl-` filename prefix is local.
Reuse follows the [JPL Image Use Policy](https://www.jpl.nasa.gov/jpl-image-use-policy/),
not a presumed blanket Creative Commons license. No agency logo or endorsement
is used. Product credits are retained here and in `sources.json`.

- Phobos/Deimos: [Mars library](https://maps.jpl.nasa.gov/tmaps/mars.html),
  Viking images, Caltech/JPL/USGS. Maps 1440 x 720.
- Io/Europa/Ganymede/Callisto: [Jupiter library](https://maps.jpl.nasa.gov/tmaps/jupiter.html),
  Voyager mosaics; Io's color adjustment also uses Galileo (David Seal).
  Maps 1440 x 720.
- Mimas/Enceladus/Tethys/Dione/Rhea/Iapetus:
  [Saturn library](https://maps.jpl.nasa.gov/tmaps/saturn.html), Voyager,
  Caltech/JPL/USGS cleaned mosaics, 1440 x 720.
- Titan: the same library's David Seal 720 x 360 **fictional cloud concept**, with
  color based on Voyager. This is not a detailed Titan surface photograph.
- Miranda/Ariel/Umbriel/Titania/Oberon:
  [Uranus library](https://maps.jpl.nasa.gov/tmaps/uranus.html), Voyager,
  Caltech/JPL/USGS, with Tammy Becker credited. Maps 1440 x 720.
- Triton: [Neptune library](https://maps.jpl.nasa.gov/tmaps/neptune.html), limited
  Voyager/USGS mosaic, Tammy Becker credited, 1440 x 720.

The JPL library explicitly warns that coverage gaps and aesthetic changes exist.
Old page descriptions reflect the era of those maps, not present-day exploration.

## Project-owned sources

`SolarRock.blend` and `SolarRock.glb`: original deterministic 5120-triangle generic
irregular rock, smooth normals and spherical UVs, maximum radius exactly 1 metre.
It is a reusable visual approximation, not a scanned asteroid.

`rock_schematic.png` and the four named `*_schematic.png` maps: original
2048 x 1024 periodic procedural rocky color/craters. Hyperion, Phoebe, Nereid,
Proteus, Ceres, Vesta, Eros and Bennu deliberately use schematic surfaces.
No scientific shape reconstruction or unverified source photo is claimed.

`uranus_ring_schematic.png`: original 4096 x 32 transparent radial illustration
of narrow ring bands. Locations are indicative; this is not an observed ring map.

The own geometry/images have no external media dependencies. Reproduction and
checks: [solar source guide](../../../Docs/SOLAR_SOURCES.md).
`sources.json` records dimensions, SHA256, download URLs and original-byte checks.

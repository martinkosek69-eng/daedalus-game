# Planet quality sources

Prepared by `Tools/Prepare-PlanetSources.py` (Blender background, OpenImageIO + numpy).
Raw inputs come from `Tools/Fetch-PlanetSources.ps1` into ignored `.local/planet-raw`,
pinned by SHA-256 where the provider serves fixed files. The prepared files are tracked
here and are what the Unreal recipe imports. `planets.json` selects bodies and their
appearance parameters for `Tools/Prepare-PlanetMaterials.py`.

## Earth

| File | Size | Content | Source and licence |
| --- | --- | --- | --- |
| `Earth/earth_day_16k.jpg` | 16384 × 8192 | Cloud-free surface colour, sRGB | NASA Blue Marble Next Generation, October 2004, without baked topography or bathymetry shading. Eight 21600² tiles (15″, about 500 m), Lanczos-3 resampled. NASA Earth Observatory, Reto Stöckli; public domain, credit NASA. |
| `Earth/earth_relief_16k.png` | 16384 × 8192 | Water mask and land height, linear. 0 = water; land = 16 + 239·√(h / 8848 m). | NOAA NCEI ETOPO 2022 60″ surface elevation (doi:10.25921/fd45-gt74), Catmull-Rom resampled; US Government work. The water mask combines BMNG open-water colour (pixel-aligned lakes and coast) with ETOPO sea floor below −10 m. |
| `Earth/earth_clouds_16k.jpg` | 16384 × 8192 | Cloud opacity, linear | NASA GIBS / Worldview VIIRS Corrected Reflectance true colour: Suomi NPP and NOAA-20, 2023-07-29, plus Suomi NPP 2023-01-29 south of 52° S. Level-5 mosaics (about 2 km), see processing below. "We acknowledge the use of imagery from the NASA Worldview application (https://worldview.earthdata.nasa.gov), part of the NASA Earth Science Data and Information System (ESDIS)." |
| `Earth/earth_night_8k.jpg` | 8192 × 4096 | City lights only, sRGB | NASA Black Marble 2016 (VIIRS Day/Night Band, 3 km): NASA Earth Observatory images by Joshua Stevens, Suomi NPP VIIRS data from Miguel Román, NASA GSFC. The dim blue moonlit base was removed (local minimum background, blue-dominance test). |
| `Meshes/PlanetSphere_256.glb`, `PlanetSphere_512.glb` | 256 × 128 and 512 × 256 segments | Finer unit spheres, same convention as `Art/Space/SolarSphere.glb` | Project-generated geometry. |
| `Defaults/*.png` | 4 × 4 | Neutral optional inputs (black, clear, flat land) | Project-generated. |

### Cloud processing

The previous cloud map, and the NASA MODIS 2001 composite it was derived from, shows
the "brushed" streak pattern reported in task 0022. A real single-day snapshot replaces it:

1. **Opacity.** I = (1 − a)·S + a·C, inverted against the cloud-free BMNG luminance S, with
   cloud reflectance C = 0.88 in linear light. A small haze floor is removed. Bright
   permanent ice (Greenland, Antarctica) stays clear because cloud and ice cannot be told
   apart there.
2. **Mosaic seams.** Daily mosaics keep the pixel nearest nadir, so neighbouring passes meet
   midway between their ground tracks. The seams are predicted from the orbit
   (sun-synchronous, 98.74°, 101.44 min). The detected phase is confirmed by edges present
   in SNPP but not in NOAA-20. Around each seam the map switches to the NOAA-20 pass between
   least-difference paths (dynamic programming, as in panorama stitching).
3. **Sun glint.** SNPP glint bands over the ocean are replaced by NOAA-20 where SNPP is
   systematically brighter at the 100 km scale. Remaining smooth, low-contrast veils over
   water are suppressed; textured and dense cloud is kept.
4. **Antarctic polar night.** South of 52° S the map cross-fades over 8° to the southern
   summer snapshot. Above 80° latitude rows blend to their zonal mean, which hides the
   date-line seam at the pole.

The result is real weather from one day, not a climatology. Small transition artefacts can
remain where the two passes, about 50 minutes apart, differ.

## Other bodies (rollout)

Every placed textured body of the six systems uses the shared masters: stars `M_PlanetStar`,
planets of 15 000 km radius or more `M_PlanetGas`, all others `M_PlanetSurface`. Existing
catalogue maps keep their stable assets and are re-encoded BC7 in the planet group. The
replacements and additions below are listed in `planets.json`.

| File | Content | Source and licence |
| --- | --- | --- |
| `Sol/io_8k.jpg` | Io colour, 8192 × 4096 | USGS Astrogeology, Io Galileo SSI / Voyager global colour mosaic, 1 km. US Government work. |
| `Sol/europa_8k.jpg`, `ganymede_8k.jpg`, `callisto_8k.jpg` | Greyscale, 8192 × 4096 | USGS Astrogeology Voyager / Galileo SSI global mosaics (Europa 500 m, Ganymede and Callisto 1 km). |
| `Sol/enceladus_4k.jpg` | Greyscale, 4096 × 2048 | USGS Astrogeology Cassini global mosaic, 110 m. |
| `Sol/moon_relief_4k.png` | Linear height, 0–255 over 19.92 km | LRO LOLA via NASA SVS CGI Moon Kit (`ldem_16_uint`, 16 px/deg). |
| `Sol/mars_relief_4k.png` | Linear height, 0–255 over 29.32 km | MGS MOLA MEGDR 16 px/deg (`megt90n000eb`), NASA PDS Geosciences Node. |
| `Fictional/<id>_clouds.png` | Cloud coverage | The authored alpha of each fictional `*_clouds.png`, unchanged. The tint is the coverage-weighted mean authored cloud colour (`cloudColor`). |

The replaced Sol maps were 1440 × 720 greyscale JPL maps. The mosaics are aligned to them in
longitude and east-west mirroring by cross-correlation of edge images
(`.local/planet-work/moons_alignment.json`). Where the old map is too sparse to match (the
Voyager-era Enceladus map is mostly blank) the convention of the confident matches is used:
east-positive, centred on 180°. The mosaics keep the previous maps' mean brightness and
contrast, and structure above about 60 km is lifted ×1.8 to match the old maps' local contrast.

Small or partial bodies keep their catalogue maps: Saturn's smaller moons, the Uranian moons,
Triton, Phobos, Deimos, the schematic minor bodies and the partial New Horizons maps.
Untextured catalogue entries keep the fallback schematic materials; nothing is invented for
them.

## Artistic supplements (not geographic data)

- **Noise below the texel.** The planet materials add fine noise, faded in only where one
  source texel covers more than one screen pixel. It erodes cloud edges, adds faint surface
  micro relief, gas-giant band turbulence and stellar granulation.
- **Relief exaggeration.** Earth relief lighting is exaggerated (×4 in `planets.json`) so the
  data-derived slopes are readable from orbit. The heights themselves are not changed.
- **Other parameters.** Cloud-shell height (×1.0015 radius), haze, glint and night-light
  strength are presentation parameters.

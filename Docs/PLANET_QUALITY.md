# Planet quality

This page describes how planets, moons and stars are presented, from task 0023 (variant B of
task 0022 plus better real maps where data exists). It covers presentation only: canonical
IDs, sizes, positions and flight rules are unchanged, and nothing here feeds back into the
simulation.

## Pipeline

1. **Raw data.** `Tools/Fetch-PlanetSources.ps1` downloads the public raw data into ignored
   `.local/planet-raw` and checks the SHA-256 of fixed files. NASA GIBS daily tiles are not
   hash-pinned because NASA may reprocess them; the prepared maps are canonical.
2. **Prepared sources.** `Tools/Prepare-PlanetSources.py` runs in Blender background with
   OpenImageIO and numpy:
   `blender -b --factory-startup --python Tools/Prepare-PlanetSources.py -- <repo> <steps>`.
   It writes the tracked sources in `Art/Space/PlanetQuality/**`. Steps are `earth-day`,
   `earth-relief`, `earth-clouds`, `earth-night`, `moons`, `relief-bodies`,
   `fictional-clouds`, `spheres` and `defaults`. Provenance and processing are in
   `Art/Space/PlanetQuality/SOURCE.md`.
3. **Manifest.** `Art/Space/PlanetQuality/planets.json` lists per-body source overrides and
   appearance parameters. `rollout: true` applies the masters to every placed textured body.
   It is validated on load; a missing file falls back to the legacy presentation.
4. **Unreal recipe.** `Tools/Prepare-PlanetMaterials.py` is called by
   `Prepare-SolarSystemMaterials.prepare()` inside the normal recipe
   (`Invoke-SolarFlight.ps1 -Mode Assets`). It builds the masters, the per-body instances, the
   texture settings and the finer spheres. The recipe hash includes these sources, helpers and
   `DefaultDeviceProfiles.ini`.
   - Faster iteration: `DAEDALUS_PLANET_ONLY=1` with the same commandlet rebuilds only planet
     presentation and does not write the shared recipe tag.
5. **Validation.** `Tools/Validate-PlanetQuality.py` is a read-only commandlet. It checks every
   instance (parent, bound sources, BC7/BC4, sRGB, planet group), cloud layers, relief, night
   maps, map discs, spheres and the retired legacy imports. Its `PLANET_BINDING` lines are
   stable between repeated imports.
6. **Probe.** `Tools/Invoke-PlanetProbe.ps1 -Source Package|Editor [-Bodies id,...]` takes
   native 3840×2160 captures. Results go to `.local/planet-probe/<label>-<time>/`
   (`result.json`, PNGs and `run.log`).
   - Earth shots: close reference-like view (with ship, and without clouds for diagnosis),
     motion, full disc, limb, terminator and glint.
   - Optional bodies are shot centred and side-lit.
   - `result.json` records GPU frame time from stat unit data and resident texture memory.

## Runtime

### Materials

Paths are stable (`/Game/Solar/Materials`, always cooked):

- `M_Body_<key>` is an instance of one master in `/Game/PlanetQuality/Materials`:
  - `M_PlanetSurface`: solid worlds.
  - `M_PlanetGas`: radius 15 000 km or more; no rock relief, Minnaert limb darkening.
  - `M_PlanetStar`: emissive, photospheric limb darkening, no planetary lighting.
- `M_Cloud_<key>`: optional instance of `M_PlanetClouds`, the separate translucent shell.

### Sampling

- Equirectangular UVs are computed from the body-space direction, with seam-safe gradients;
  bodies with authored meshes use their mesh UVs.
- Lighting is done in body space. Night lights are emissive.

### Surface master

- **Relief:** from data. With a water mask, water neighbours reuse the centre height, so
  coasts get no false cliffs.
- **Cloud shadow:** the cloud map sampled along the sun ray at the shell height.
- **Ocean:** GGX sun glint and Fresnel sky reflection.
- **Light and atmosphere:** soft terminator and aerial haze toward the limb.

### Artistic detail

Noise below the texel size, faded in only when one texel covers more than one pixel and
faded out per octave below a pixel. It erodes cloud edges and adds surface micro relief,
band turbulence and granulation. On real bodies it is labelled as an artistic supplement.

### Layers

`Source/Daedalus/Solar/PlanetPresentation.cpp`:

- Creates and destroys cloud shells with the system (shell at 1.0015 × radius).
- Sets `SunDirection` and the star tint `SunColor` (half way to white).
- Sorts translucent layers: clouds −2, air −1, so engine glows and other effects stay in front.
- Picks the sphere mesh by true projected pixel radius: 128, 256 or 512 segments, switching at
  800 and 3200 px with hysteresis, which keeps the polygonal limb under 0.25 px. This is a
  mesh swap because the project forces `r.ForceLOD=0`.

### Texture group and encoding

- `TEXTUREGROUP_Project01`, set in `Game/Daedalus/Config/DefaultDeviceProfiles.ini`: linear
  mips, aniso, 16K maximum. The World group and ship textures are unchanged.
- Colour maps are BC7 sRGB; masks (clouds and relief) are BC4 linear (`TC_Alpha`).
- Streaming stays off (project default), so all mips are resident.

## Earth

| Map | Resolution | Data |
| --- | --- | --- |
| Day | 16384 × 8192 | NASA BMNG 500 m (October), no baked shading |
| Relief and water | 16384 × 8192 | NOAA ETOPO 2022 60″ and BMNG water |
| Clouds | 16384 × 8192 | VIIRS SNPP and NOAA-20 snapshot of 2023-07-29 |
| Night | 8192 × 4096 | Black Marble 2016, lights only |

The previous 8K Earth maps are retired to 64 px, because legacy graph nodes still reference
them. `M_Earth` (required by the game mode) shares the new maps.

## Measured cost

Measured on an RTX 3070 at 3840×2160 with the whole active system loaded. See
`Tasks/0023/PROGRESS.md` and `HANDOFF.md` for the runs.

- GPU frame time stayed in the 6–8 ms range before and after; the difference is within the
  run-to-run spread.
- Resident planet texture memory in Sol:
  - 354 MiB before;
  - 674 MiB after the Earth step;
  - 1124 MiB in the package after the rollout (BC7 for all maps, the 8K moons and Earth).
- In the packaged build, GPU frame time is 6.1–7.7 ms across 34 shots.
- Visiting all six systems adds about 65 MiB of layer textures. The galaxy map keeps every
  system's map materials, and so their surfaces, loaded, as before.
- Process GPU memory is not reported reliably by this RHI and is not used.

## Known limits

- **Coast sharpness.** The Earth coastline is limited by the 16K base (about 2.4 km per
  texel). Very close views stay soft at the coast; tiled variant C was not implemented.
- **Clouds.** They are one real day, not a climatology. Small transition artefacts can remain
  where the two passes, about 50 minutes apart, meet. Clouds over permanent ice are not
  separable from the ice and stay clear.
- **Small moons.** Saturn's smaller moons, the Uranian moons, Triton, Phobos and Deimos keep
  their 1440 × 720 maps with shader detail. Venus and Titan are cloud-deck maps with minimal
  detail.
- **Moon orientation.** Moon longitudes are aligned to the previous maps. Tidal-lock
  orientation is not modelled by the game.

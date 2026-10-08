# 0023 handoff: complete planet quality (variant B with better real maps)

- **Status:** READY_FOR_REVIEW. This is not ACCEPTED.
- **Worker:** Claude Code.
- **Branch:** `codex/claude-0023-planet-upgrade`, from founding commit `91e0e01`
  (base `58cdfad`). The hand-off target is `codex/solar-flight`, not main.
- **Checkpoints:**
  - `f5d95d7`: Earth.
  - `85e5f15`: rollout.
  - Final: the commit that contains this file, which is the branch head. Check it with
    `git ls-remote origin codex/claude-0023-planet-upgrade`.
- **Documentation:**
  - technical reference: `Docs/PLANET_QUALITY.md`;
  - data provenance: `Art/Space/PlanetQuality/SOURCE.md`;
  - short summary for the user: `RESULTS_CZ.md`.

## What changed

### Earth

- **Day map:** real 16K (NASA BMNG 500 m).
- **Relief and water:** 16K, from NOAA ETOPO 2022 with BMNG water.
- **Clouds:** 16K from a real VIIRS daily snapshot. It replaces the streaky MODIS-derived map;
  seams, glint and the polar night are repaired from data.
- **Night lights:** 8K Black Marble 2016, lights only.
- **Material:** separate translucent cloud shell with a cast shadow, relief lighting from data,
  GGX ocean glint, sky reflection, aerial haze, a soft terminator, and sharper limb geometry.

### All six systems

All 105 placed, textured bodies use shared masters in `/Game/PlanetQuality/Materials`:

- `M_PlanetSurface`;
- `M_PlanetGas`: no rock relief;
- `M_PlanetStar`: no planetary lighting;
- `M_PlanetClouds`: shells on Earth and the 9 fictional cloud worlds.

The per-body instances keep the stable paths `M_Body_<key>` and `M_Cloud_<key>`.

### Better real maps

- Io, Europa, Ganymede and Callisto at 8K, Enceladus at 4K (USGS).
- Moon (LOLA) and Mars (MOLA) relief.

### Encoding

- Colour maps are BC7 sRGB; masks are BC4 linear.
- New planet texture group Project01 (linear mips, 16K), set in
  `Game/Daedalus/Config/DefaultDeviceProfiles.ini`. The World group and ship textures are
  unchanged.

### Runtime

`Solar/PlanetPresentation.cpp` adds:

- cloud shells, with creation and cleanup on system change;
- translucency order;
- star light tint;
- 128-, 256- or 512-segment spheres chosen by projected pixel radius. This is a mesh swap,
  because the project forces `r.ForceLOD=0`.

### Diagnostics

- `Solar/PlanetProbe.cpp` runs with `-SolarSharpProbe=<dir> -SolarPlanetProbe` through
  `Tools/Invoke-PlanetProbe.ps1`.
- `Tools/Validate-PlanetQuality.py` checks every hookup.

### Unchanged

The approved Daedalus and its plating, star points and the black sky (`M_Star`), controls,
speeds, R and Shift+R, the camera, the HUD and map, flight data, IDs, sizes and positions.
There is no global blur, AA, upscaling or LOD/streaming change.

## Changed files

| Area | Files |
| --- | --- |
| Source | `Game/Daedalus/Source/Daedalus/Solar/`: `PlanetPresentation.cpp` (new), `PlanetProbe.cpp` (new), `SolarSystem.cpp`, `SolarFlightGameMode.h`, `SolarSharpProbe.cpp` (planet probe entry only) |
| Config | `Game/Daedalus/Config/DefaultDeviceProfiles.ini` (new, Project01 only) |
| Tools (new) | `Prepare-PlanetSources.py`, `Prepare-PlanetMaterials.py`, `Validate-PlanetQuality.py`, `Fetch-PlanetSources.ps1`, `Invoke-PlanetProbe.ps1` |
| Tools (changed) | `Prepare-SolarContent.py`: recipe hash, `DAEDALUS_PLANET_ONLY`, legacy `M_Earth`. `Prepare-SolarSystemMaterials.py`: planet path. `Validate-SolarContent.py`: accepts instances. |
| Sources | `Art/Space/PlanetQuality/**`: Earth, Sol, Fictional, Meshes, Defaults, `planets.json`, `SOURCE.md` |
| Content | `Content/PlanetQuality/**` (masters, new textures); `Content/Solar/Materials/M_Body_*` (now instances), `M_Cloud_*` (new), `M_Map_*`; `Content/Solar/Textures/T_*` (BC7, planet group); `Content/Solar/Models/SM_PlanetSphere_*` (new), `SM_Body_*` (material re-bound) |
| Docs | `Docs/PLANET_QUALITY.md`, and `Tasks/0023/PROGRESS.md`, `HANDOFF.md`, `RESULTS_CZ.md`, `EVIDENCE/**` |

No ship, sky, starfield, dust or flight data, galaxy/UI, `.umap` or player data was
committed. `system.json` and the catalogs are unchanged; appearance lives in
`planets.json`.

## Checks (all run on this checkout)

| Check | Result |
| --- | --- |
| Editor build (`Invoke-SolarFlight.ps1 -Mode Build`) | Succeeded. |
| Automation tests (`-Mode Test`) | 19 succeeded, 0 failed, 0 not run. Flight and navigation checks pass. |
| Full recipe (`-Mode Assets`) | `SOLAR_CONTENT_PASS` with 105 planet-quality bodies; a second run reports `cached verified recipe`. |
| Repeated import | `Validate-PlanetQuality.py` bindings are identical after two planet-only runs and after the full run. |
| `Validate-PlanetQuality.py` | Pass: 105 bodies, 10 cloud layers, 3 relief maps, 5 night maps, 4 masters. Checks parent, sources, BC7/BC4, sRGB, Project01, map discs and spheres. |
| `Validate-SolarContent.py` | Pass: 210 world/map bindings. |
| Package (`-Mode Package -BuildName Build-PlanetQuality`) | BUILD SUCCESSFUL in 4 min. |
| Packaged planet probe (`Invoke-PlanetProbe.ps1 -Source Package`) | 34 shots at native 3840×2160, all pass. Earth: reference, ship, surface, motion, full, limb, terminator, glint. Plus 24 representatives covering every type and all five fictional systems. |
| Cooked formats (packaged log) | Earth day 16K and night 8K are PF_BC7; Earth clouds and relief 16K are PF_BC4; moons are PF_BC7; Moon and Mars relief are PF_BC4. All mips resident. |
| Packaged Sharp (`-Mode Sharp`) | Pass at 3840×2160: no AA or upscaling, full texture detail (new Earth maps 15/15 mips), R and Shift+R impulse checks. |
| Layer lifecycle | Cloud shells per active system match its cloud worlds (1, 2, 2, 1, 3, 1) across all system switches. |

**Measurements** (RTX 3070, 3840×2160, whole active system):

| | GPU frame time | Planet texture memory |
| --- | --- | --- |
| Before (baseline content, same camera) | 6.4–8.6 ms | 354 MiB |
| After, packaged (Sol) | 6.1–7.7 ms | 1124 MiB |
| After, packaged (all systems visited) | 6.1–7.7 ms | 1189 MiB |

All texture memory in the package is 1.14–1.21 GiB. The GPU time difference is within the
run-to-run spread.

**Evidence** (`EVIDENCE/`):

- `earth-*_before_after.jpg`: same camera and sun, half frames and 100% crops.
- `bodies_*_before_after.jpg`: representatives, 1/4 frames and 100% crops of Ganymede, Moon,
  Mars and Thalassa.
- `rollout_*_sidelit.jpg`: contact sheets.

"Before" is the baseline content rendered by the same probe build in the editor `-game`.
"After" is the package.

**Machine-readable data** (`EVIDENCE/data/`):

| File | Contents |
| --- | --- |
| `probe_earth_before_editor.json`, `probe_bodies_before_editor.json` | Probe results before, per shot: GPU ms, texture MiB, cloud shells, sphere level, system |
| `probe_after_package.json` | Probe results after, per shot, same fields |
| `cooked_planet_textures_package.txt` | Cooked formats, sizes, mips and MiB |
| `sharp_package_checks.txt` | Packaged Sharp check lines |
| `planet_bindings.jsonl` | Validator binding per body (type, sources, texture sizes) |
| `earth_clouds_stats.json`, `earth_relief_stats.json` | Earth cloud and relief preparation statistics |
| `moons_alignment.json`, `relief_alignment.json` | Moon-map and relief alignment |

## Decision log

1. **Cloud source.** The MODIS 2001 composite has the same "brushed" streaks as the old map,
   so it cannot fix them. A real VIIRS day (2023-07-29) is used instead. Repairs:
   - mosaic seams are predicted from the orbit and confirmed against NOAA-20 data, then
     stitched with least-difference paths;
   - glint is replaced by NOAA-20, and remaining smooth glint veils over water are suppressed;
   - the Antarctic polar night comes from 2023-01-29;
   - clouds over ice stay clear.
2. **16K Earth base.** Real BMNG 500 m data, not an upscaled 8K map. The October month has no
   baked topography shading; relief comes from ETOPO in the lighting.
3. **Night lights.** The Black Marble moonlit blue base is removed, so only city lights glow.
4. **No tiles.** Variant C (tiles) was not implemented, as the brief required.
5. **Sphere detail.** The project forces `r.ForceLOD=0`, so finer spheres are a runtime mesh
   swap by pixel radius rather than static-mesh LODs. Global LOD and streaming are unchanged.
6. **Appearance data.** Appearance lives in `planets.json`, not `system.json`, which is
   generated. Catalogs, IDs and geometry are untouched.
7. **Legacy Earth maps.** `M_Earth` is CDO-referenced, and deleting its graph nodes asserts in
   UE. The old 8K Earth imports are retired to 64 px instead of deleted, which saves about
   64 MiB of resident memory.
8. **Moon maps.** USGS mosaics are aligned by edge correlation. Enceladus's reference map is
   mostly blank, so it uses the convention of the confident matches. Mean brightness and
   contrast follow the previous maps.
9. **No unrelated re-saves committed.** Each recipe run re-saves unrelated legacy assets
   (graphs accumulate nodes). Those re-saves, and the ship and sky assets of the full run,
   were reverted before each commit.

## How to try it

The package is at
`A:/GPT-CODEX/Claude/worktrees/claude-0023/.local/solar/Build-PlanetQuality/Windows/Daedalus.exe`.
The user's own package, saves and launcher were not touched.

- Play: `Tools/Invoke-SolarFlight.ps1 -Mode Play -BuildName Build-PlanetQuality`, which uses
  its own `.local/solar/PlayerData`.
- Probe again: `Tools/Invoke-PlanetProbe.ps1 -Source Package -BuildName Build-PlanetQuality
  -Bodies sol.moon,sol.mars`.
- Validate: run `UnrealEditor-Cmd ... -run=pythonscript -script=Tools/Validate-PlanetQuality.py`
  with no editor running.

## Integration notes for Codex

- **Run Assets once at integration.** The recipe hash now includes the planet sources,
  helpers and `DefaultDeviceProfiles.ini`, so the first `Invoke-SolarFlight.ps1 -Mode Assets`
  re-runs the full recipe and re-tags `SM_Daedalus`. This branch deliberately does not commit
  the ship re-saves of that run.
- **Iterate on planets only.** Set `DAEDALUS_PLANET_ONLY=1` with the Assets commandlet. It
  rebuilds planet presentation only and never writes the recipe tag.
- **Raw data.** It is not committed (several GB). `Tools/Fetch-PlanetSources.ps1` fetches and
  verifies it; the prepared sources are tracked.

## Limits

- **Coast.** The Earth coast is bounded by the 16K base (about 2.4 km per texel).
  Variant C (tiles) was not implemented.
- **Clouds.** They are one real day. Small seam or transition remnants are possible, and
  clouds over permanent ice stay clear.
- **Small moons.** Saturn's smaller moons, the Uranian moons, Triton, Phobos and Deimos keep
  their 1440 px maps with shader detail.
- **Artistic detail.** The noise below the texel size is labelled artistic on real bodies.
- **Memory.** Process GPU memory is not reported reliably by the RHI. Texture memory is
  measured exactly from the resident mips.

## Applications

**Editor and build ownership is released.** No Unreal editor, commandlet or build of this
checkout is running. All test processes are closed.
The user's Blender GUI (Aurora view) was never touched.

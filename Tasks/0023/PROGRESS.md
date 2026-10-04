# 0023 progress

- Status: IN_PROGRESS. The Earth checkpoint is done and the rollout to the other bodies is
  next. This is not READY_FOR_REVIEW.
- Owner: Claude Code. Branch: `codex/claude-0023-planet-upgrade`, founded at `91e0e01`.
- Checkout: own worktree `A:/GPT-CODEX/Claude/worktrees/claude-0023`, with all LFS objects.
- Applications:
  - **Unreal editor/build:** owned by Claude for this task; only this checkout's project is
    used, through `UnrealEditor-Cmd` commandlets and `-game` probe runs. No interactive editor.
  - **Blender:** background only, on own saved sources. The user's open Blender GUI (Aurora
    view) was not touched.

## Earth checkpoint (2026-10-04)

### Data

New public data, prepared reproducibly. Provenance is in `Art/Space/PlanetQuality/SOURCE.md`.

- **Day map:** real 16K, BMNG 500 m, no upscaled 8K.
- **Relief and water mask:** 16K, ETOPO 2022 with BMNG water.
- **Clouds:** 16K, real VIIRS daily snapshot. The MODIS composite has the same streaks as the
  old map, so it was replaced. Mosaic seams, sun glint and the polar night are repaired with
  the second satellite or a second date.
- **Night lights:** 8K, Black Marble 2016 lights only.

### Materials

`Tools/Prepare-PlanetMaterials.py` builds shared masters `M_PlanetSurface` and
`M_PlanetClouds`. `M_PlanetGas` and `M_PlanetStar` exist in code for the rollout.

- **Earth instances:** `M_Body_earth` and `M_Cloud_earth` are instances at the stable paths.
- **Sampling:** analytic latitude/longitude UVs with seam-safe gradients.
- **Relief:** from data, with no false cliffs at the coast.
- **Cloud layer:** separate translucent shell with soft edges and a cast shadow.
- **Ocean:** GGX sun glint and Fresnel sky reflection.
- **Light and atmosphere:** soft terminator, aerial perspective and night lights.
- **Fine detail:** noise below the texel size, faded by pixel footprint and labelled artistic.

### Encoding

- Day and night maps are BC7 sRGB; relief and clouds are BC4 linear (`TC_Alpha`).
- All use the planet group Project01 (`DefaultDeviceProfiles.ini`): linear mips and a 16K
  limit. The engine World group is unchanged.
- The legacy `M_Earth` shares the new maps. The old 8K imports are retired to 64 px because
  disconnected nodes still reference them.

### Geometry and runtime

`Solar/PlanetPresentation.cpp` handles cloud shells, the sun tint and cleanup on system
change, plus sphere detail:

- Finer 256- and 512-segment spheres are chosen by true projected pixel radius, with
  hysteresis.
- This is a runtime mesh swap, because the project forces `r.ForceLOD=0`.
- Polygonal limb error stays below 0.25 px.

### Probe

`Solar/PlanetProbe.cpp` with `-SolarSharpProbe=<dir> -SolarPlanetProbe` takes native
3840×2160 captures of ten Earth shots. It records GPU frame time from stat unit data and
resident texture memory, and runs through `Tools/Invoke-PlanetProbe.ps1`.

### Measurements

Measured in the editor `-game` at 3840×2160 on an RTX 3070, whole Sol system loaded.

| | Before (same camera) | After |
| --- | --- | --- |
| GPU frame time | 6.4–8.6 ms | 6.6–7.4 ms (no measurable increase) |
| Planet texture memory | 354 MiB | 674 MiB (+320 MiB, all mips resident) |
| All texture memory | 405 MiB | 725 MiB |

Process GPU memory is not reported reliably by this RHI and is not used.

**Evidence:** `EVIDENCE/earth-*_before_after.jpg` pairs half frames and 100% crops of the same
camera and sun.

## Next

1. Rollout to all current bodies of the six systems. Gas giants and stars get their masters;
   rocky and icy bodies get shared detail; small source maps are improved where needed;
   fictional cloud worlds get cloud shells.
2. Validators, a full Assets run, the editor build, the `Build-PlanetQuality` package, a
   packaged probe, and the handoff documents.

The full Assets run reimports ship assets; those re-saves are not committed. Codex runs
Assets at integration, and the recipe hash includes the new sources and helpers.

# 0023 progress

- Status: **READY_FOR_REVIEW** (not ACCEPTED). See `HANDOFF.md` for results and checks,
  `RESULTS_CZ.md` for the user summary and `Docs/PLANET_QUALITY.md` for the technical
  reference.
- Owner: Claude Code. Branch: `codex/claude-0023-planet-upgrade`, founded at `91e0e01`.
- Checkout: own worktree `A:/GPT-CODEX/Claude/worktrees/claude-0023`.
- Applications:
  - **Unreal:** ownership is released. No editor, commandlet or build is running.
  - **Blender:** background only. The user's Blender GUI was not touched.

## Timeline (2026-10-04)

1. **Baseline.** Editor build and baseline 4K captures through the new planet probe
   (`SolarSharpProbe` entry with `-SolarPlanetProbe`).
2. **Earth checkpoint, `f5d95d7`:**
   - real 16K day map (BMNG), 16K relief and water (ETOPO), 16K VIIRS clouds and 8K
     night lights;
   - cloud shell, relief, glint, haze and terminator;
   - Project01 group (BC7/BC4) and finer spheres.
3. **Rollout, `85e5f15`:**
   - shared surface, gas, star and cloud masters for all 105 placed textured bodies of the
     six systems;
   - USGS 8K Galilean moons and 4K Enceladus; LOLA and MOLA relief;
   - cloud shells for the nine fictional cloud worlds;
   - `Validate-PlanetQuality.py`.
4. **Final checks:**
   - full Assets run and a cached rerun;
   - 19/19 automation tests;
   - the `Build-PlanetQuality` package;
   - packaged planet probe (34 shots at native 4K), cooked format check and packaged Sharp
     check;
   - before/after evidence.

The full Assets run reimported ship assets. Those re-saves were reverted, not committed;
Codex runs Assets at integration.

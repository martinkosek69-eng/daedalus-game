# 0022 progress

- Status: READY_FOR_REVIEW (phase 1: proposal only; not ACCEPTED, no game changes)
- Worker: Claude Code, launched by the user
- Branch: `codex/claude-0022-planet-quality`, starting from founding commit `5a0e56c`
- Checkout: its own new worktree on A:, created with `GIT_LFS_SKIP_SMUDGE`. Only
  the files needed for analysis were fetched from LFS: REFERENCE, three Earth
  maps, two Earth texture assets, the Earth material and the sphere.
- Application ownership:
  - **Unreal:** not started, no build. The diagnosis used files, asset metadata,
    engine config and engine source only.
  - **Blender:** background read-only scripts in ignored `.local/analysis`.

## Done

- Read: AGENTS, README, ENVIRONMENT (relevant parts), FOUNDATION, ARCHITECTURE,
  CODING_RULES, CURRENT_STATE, APP_OWNERSHIP, SOLAR_FLIGHT, SOLAR_REALISM,
  CONTENT_WORKFLOW, Tasks/README and BRIEF 0022.
- REFERENCE.png: 3423 × 1630, Earth close orbit, British Isles. Measured scale:
  about 0.87 km per screen pixel, so about 5.6 screen px per texel of the 8K map.
- Sources: Earth day, night and cloud maps are 8192 × 4096 baseline JPEG,
  quality about 91, 4:4:4. Real detail exists down to 2 texels (measurement in
  EVIDENCE/earth_maps_measurements.json).
- Import, from the asset registry tags of T_earth_daymap and T_earth_clouds:
  TC_Default, LODGroup World, MipGen FromTextureGroup, sRGB, no alpha,
  MaxTextureSize 8192. Engine source Texture.cpp maps TC_Default without alpha
  to DXT1/BC1.
- Engine profile: TEXTUREGROUP_World uses MaxLODSize 16384 and MipFilter=point,
  with no project override.
- Material: M_Body_earth is unlit, with emissive custom HLSL. It applies Lambert
  lighting and `lerp(day, white, Cloud.r*0.7)` in the same UV. There is no
  separate cloud layer, normal map, water specular or relief.
- Runtime: streaming off (all mips resident), AA off, `r.Tonemapper.Sharpen 0.25`,
  aniso 16. The sphere is 128 × 64 (16,128 triangles), scaled per body.
- Texel sizes per body are from Art/Space/SolarSystem/sources.json.
- Evidence (EVIDENCE/):
  - the composite of the source maps reproduces the streaky cloud band of
    REFERENCE;
  - an offline BC1 simulation shows a smaller block effect.

- Wrote PROPOSAL.md (diagnosis F1–F9, variants A–D, recommendation B, the
  Earth verification plan and rollout), SUMMARY_CZ.md and HANDOFF.md.

## Next

Codex reviews and the user chooses. Implementation waits for a new
instruction. Unreal was never started; editor and build ownership is released.

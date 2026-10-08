# 0022 handoff: independent proposal, phase 1

- **Status:** READY_FOR_REVIEW. This is not ACCEPTED; the proposal only.
- **Worker:** Claude Code.
- **Branch:** `codex/claude-0022-planet-quality`, from founding commit
  `5a0e56c`.
- **Last pushed commit:** see the branch head. The commit that contains this
  file is the last one.
- **Scope kept:** only PROPOSAL.md, SUMMARY_CZ.md, PROGRESS.md, HANDOFF.md and
  EVIDENCE/** changed.
  - No game source, material, asset, map, data or config change.
  - No reimport, no SaveAll, no bulk download or regeneration.

## Results

- `PROPOSAL.md`: the technical part.
  - Diagnosis F1–F9, each labelled CONFIRMED, HYPOTHESIS or UNKNOWN, with
    paths and lines.
  - Variants A–D and the recommendation: **B**, a multi-scale planet material
    with a separate cloud shell, relief and specular, correct encoding, a
    planet texture group with linear mips, and a finer sphere. For Earth only,
    A's 16K base is optional and C is held in reserve.
  - The Earth-only verification plan V0–V4 with acceptance criteria.
  - The rollout, the data and licences, and the unknowns.
- `SUMMARY_CZ.md`: one page for the user.
- `EVIDENCE/`, derived from Solar System Scope CC BY 4.0 maps and REFERENCE:
  - `earth_uk_sources_vs_reference.png`,
  - `earth_bc1_simulation.png`,
  - `earth_maps_measurements.json`.

## Checks that were run

| Check | Result |
| --- | --- |
| REFERENCE scale | Measured: about 0.87 km per px, about 5.6 px per texel of the 8K map. |
| Earth JPEGs | DQT/SOF parsed: 8192 × 4096, quality about 91, 4:4:4. Detail test down to 2 texels. |
| Earth texture assets | Registry tags read: TC_Default, World, FromTextureGroup, sRGB, no alpha, MaxTextureSize 8192. |
| Engine format mapping | UE 5.8 `Texture.cpp:4360-4376`: TC_Default without alpha gives DXT1. |
| World group | `BaseDeviceProfiles.ini:184`: MipFilter=point, MaxLODSize=16384. |
| Material and runtime | Material generator and runtime rendering code read. |
| Composite and BC1 | Offline composite and BC1-style simulation in Blender background. |

**Not run:**

- Unreal editor or build, so no in-engine measurement of the cooked format,
  colours or GPU cost. These are listed as UNKNOWN in PROPOSAL §8.

## Applications

- Unreal was not started and no build was run. **Editor and build ownership is
  released.**
- Blender was used only in the background on read-only copies. No GUI was
  used and no assets were created.

## Next

Codex compares this with its own view and the user chooses. Implementation
needs a new explicit instruction. This phase stops here.

The local helper scripts stay in ignored `.local/analysis` of this checkout.

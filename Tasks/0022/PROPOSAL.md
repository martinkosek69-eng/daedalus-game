# 0022: Independent diagnosis and proposal for planet and cloud quality

**Author:** Claude Code, phase 1, analysis only.

**Basis:** founding commit `5a0e56c` (game `906567c`). No game files, assets,
materials or settings were changed. Unreal was not started.

**Status labels** used throughout:

- **CONFIRMED:** shown by files, asset metadata, engine config or source, or a
  reproducible measurement.
- **HYPOTHESIS:** supported but not yet proven in the engine.
- **UNKNOWN:** needs a test.

## 1. Symptoms in REFERENCE.png

REFERENCE.png is 3423 × 1630 and shows Earth in close orbit with the British
Isles. The ship and HUD are crisp, but the planet is not:

- **S1. Soft, low-detail land.** Coastlines and terrain have no fine structure.
- **S2. Clouds smeared over land and sea.** They appear as diagonal brushed
  streaks, and the same streaks run across the land.
- **S3. Blocky white flecks.** Where clouds meet land they turn into squares
  and stair-steps.
- **S4. Flat look.** There is no relief, no water glint and no cloud depth or
  shadow. Colours are much more saturated and brighter than the source maps.
- **S5. Banding toward the horizon.** Zones of different softness appear near
  the limb.

## 2. Findings

### F1: The view magnifies the texture about 5.6× (CONFIRMED, measured)

| Quantity | Value |
| --- | --- |
| Britain in REFERENCE | about 967 km of latitude (49.96°–58.67° N) over about 1110 px |
| Screen scale | about **0.87 km per pixel** |
| Texel of the 8192 × 4096 map | 360°/8192 E–W and 180°/4096 N–S, about 4.89 km per texel N–S |
| Britain in the map | about 198 texels N–S |
| Magnification | **≈ 5.6 screen pixels per texel** (± about 20 %, the pose is not known exactly) |

At this view the texture is stretched 5–6×. No filtering can show detail
finer than 4.9 km here, so this alone explains S1. The ship looks sharper
because its plating is about 1.6 cm per texel; a planet cannot be built that
way.

A 16K equirectangular map would still be about 2.8× magnified. About 1 px per
texel would need about 46K, which is only possible with tiles or virtual
textures (variant C).

### F2: Clouds are painted into the surface, and the cloud source is streaky (CONFIRMED)

The material code is `Tools/Prepare-SolarSystemMaterials.py:115-118`:

```
lerp(T, float3(.88,.92,.97), Cloud.r*.7)
```

- Clouds use the same UV, texel size and pixel as the ground. There is no
  separate shell, no parallax and no shadow, so clouds can never be sharper
  than the ground.
- The Solar System Scope `earth_clouds.jpg` (8K) contains brushed, diagonal
  streak structure over the North Atlantic and Britain.
- Composing the source maps exactly like the material reproduces the REFERENCE
  pattern. In `EVIDENCE/earth_uk_sources_vs_reference.png` the panels are, in
  order:
  1. day map,
  2. cloud map,
  3. composite, all three magnified 4×,
  4. the REFERENCE crop.
- The streaks in S2 therefore come from the source content, composited at 70 %
  and then magnified. They are not a rendering bug.

### F3: Block compression (CONFIRMED setting; visible share is a HYPOTHESIS)

**Asset tags** of T_earth_daymap and T_earth_clouds:

| Tag | Value |
| --- | --- |
| CompressionSettings | `TC_Default` |
| LODGroup | `TEXTUREGROUP_World` |
| MipGenSettings | `TMGS_FromTextureGroup` |
| HasAlphaChannel | False |
| Colour space | sRGB |
| MaxTextureSize | 8192 |
| Source | JPEG |

**Engine source** (UE 5.8 `Runtime/Engine/Private/Texture.cpp:4360-4376`):
`TC_Default` without alpha maps to **DXT1/BC1**. Each 4×4 texel block then
stores two endpoint colours plus two interpolated ones. At 5.6× magnification
a block covers about 22 × 13 screen px. The cloud mask, which is a single
channel, is also stored as RGB BC1, so only 4 levels per block.

**Offline BC1-style simulation** (`EVIDENCE/earth_bc1_simulation.png`):

- RMS error 1.6 % on the day map and 2.8 % on the clouds;
- visible stair-steps on cloud edges over land;
- clearly smaller than the effect of F1 and F2.

This makes BC1 a likely contributor to S3, not the main cause. The exact
cooked format, including any Oodle RDO, still needs an editor check.

### F4: Mip filtering between levels (CONFIRMED setting)

- **Setting:** `Engine/Config/BaseDeviceProfiles.ini:184`, the
  `[GlobalDefaults DeviceProfile]`, sets `TEXTUREGROUP_World` to
  `MipFilter=point` (bilinear within a level, a hard switch between levels).
  The project config has no override.
- **Effect:** on a sphere seen at a growing angle toward the horizon, this
  creates zones of different softness, which matches S5.
- **Other settings:** `MaxLODSize=16384`, so 16K would be allowed.
  `r.TextureStreaming=0` (`DefaultEngine.ini:4`), so all mips are resident;
  missing mips are not the cause.

### F5: Lighting is flat (CONFIRMED)

- The material is `MSM_UNLIT` with emissive custom HLSL
  (`Prepare-SolarContent.py:117,137`).
- Ground lighting is `c*max(.026,d)` from vertex normals only.
- There is no normal or relief map, no water specular and no ground
  roughness. Atmosphere is only an additive edge rim (`M_Air_*`).
- Without micro-variation the magnified texels are fully exposed (S4).

### F6: Post-processing (CONFIRMED setting; effect is a HYPOTHESIS)

- `SolarRendering.cpp:14-18` sets AA 0, `r.Tonemapper.Sharpen 0.25` and
  `r.MaxAnisotropy 16`.
- The sharpening approved for the ship also amplifies BC1 block and JPEG edges
  on the magnified planet.
- The unlit emissive values pass through the filmic tonemapper without a planet
  calibration. This probably explains the more saturated, brighter colours than
  the source (S4). **UNKNOWN** without an in-engine measurement.

### F7: Geometry (CONFIRMED, minor)

- `Art/Space/SolarSphere.glb` is a 128 × 64 UV sphere: 8383 vertices, 16,128
  triangles, scaled per body (`SolarSystem.cpp:378`).
- The chord between vertices is 2.81°. For Earth the arc sag is about 1.9 km,
  which is about 2 px at the REFERENCE pose.
- The limb is therefore slightly polygonal, and the equirectangular map pinches
  at the poles. This is a secondary issue.

### F8: Other bodies have an even lower texel density (CONFIRMED, from Art/Space/SolarSystem/sources.json)

The **full-disc magnification** column is the screen px per texel at the disc
centre when the planet's diameter fills 3840 px. It equals 1920·2π / map width.

| Body | Map width | km per texel at the equator | Full-disc magnification |
| --- | --- | --- | --- |
| Earth | 8192 | 4.9 | 1.5× |
| Moon | 8192 | 1.3 | 1.5× |
| Mars | 8192 | 2.6 | 1.5× |
| Mercury | 8192 | 1.9 | 1.5× |
| Venus (atmosphere) | 4096 | 9.3 | 2.9× |
| Jupiter | 4096 | 107 | 2.9× |
| Saturn | 4096 | 89 | 2.9× |
| Uranus | 2048 | 78 | 5.9× |
| Neptune | 2048 | 76 | 5.9× |
| Io, Europa, Ganymede, Callisto (JPL) | 1440 | 6.8–11.5 | 8.4× |
| Titan (JPL) | 720 | 22.5 | 16.8× |
| Fictional systems 0015, headline worlds | 4096 | depends on radius | 2.9× |
| Fictional systems 0015, other bodies | 2048 | depends on radius | 5.9× |

Notes:

- The provider's files named "8k" for Jupiter, Saturn and the Sun are
  4096 × 2048.
- In close orbit, every body is magnified much more than the full-disc figure.
- **Implication:** for Earth, the Moon and Mars the problem appears only up
  close (F1). For gas giants, the JPL moons and the fictional bodies it is
  visible even on a full disc.

### F9: The source data is good for its size (CONFIRMED)

The measurements are in `EVIDENCE/earth_maps_measurements.json`:

- **JPEG:** baseline 8192 × 4096, quality about 91, 4:4:4. It is not an
  upscale.
- **Real detail:** a down/up 2× test leaves 0.28 to 0.44 of the local
  standard deviation, so detail is present down to 2 texels.

Neither the JPEG quality nor a hidden upscale is the cause. The limit is the
texel size, the cloud content and the way they are combined.

### Summary of causes for REFERENCE

1. **Main cause:** 5.6× magnification of a 4.9 km per texel map (F1).
2. **Look of the clouds:** streaky source clouds composited into the ground at
   70 % (F2).
3. **Contributors:**
   - BC1 blocks (F3),
   - point mip filtering (F4),
   - flat unlit lighting with no relief, specular or cloud shadow (F5),
   - sharpening and uncalibrated tonemapping (F6),
   - the 128 × 64 limb (F7).

**What this means for "regenerate everything in 4K":**

- **Earth, the Moon, Mars and Mercury:** this would be a **2× worse** texel size
  (8K is already in use).
- **Gas giants, the JPL moons and the 2K fictional bodies:** it would help only
  in full-disc views.
- **Close orbit:** it would not help at all.

## 3. Variants

### A: Higher-resolution static maps plus correct encoding

**Scope**

- Earth day map at 16384 × 8192 from NASA Blue Marble Next Generation. That
  data is about 500 m, giving 2.4 km per texel.
- Albedo as BC7, the cloud mask as BC4, the night map as BC1, linear mip
  filtering.
- Bodies below 4K get better existing sources.

**Results**

| Aspect | Assessment |
| --- | --- |
| Quality | Up close the magnification drops from 5.6× to about 2.8×, so coastlines are 2× sharper. Clouds stay painted onto the ground, with no depth or relief. Far views barely change. |
| Memory | Streaming is off, so the full chain stays resident: 16K BC7 is about 171 MB per map. Earth day + clouds + night would be about 360 MB. |
| Effort | Low to medium: data preparation, import settings. |
| Maintenance | Simple. |
| Coverage | Only for bodies with real high-resolution data. Fictional worlds would need regeneration. |

### B: Multi-scale planet material (recommended)

Keep the base maps and add layers that work at any distance.

1. **Separate cloud shell.**
   - A sphere about 0.2 % above the surface (about 12 km for Earth) with its
     own material.
   - BC4 coverage, light from the Sun, a shadow on the ground (the cloud
     sample offset along the light direction) and a slow drift.
   - **Procedural edge breakup:** 2–3 noise octaves in object space at a 1–8 km
     scale, which remap coverage only near edges. Cloud edges become crisp at
     any magnification while the large shapes stay from the data.
   - Over Britain the streaky source could be smoothed by a gentle low-pass
     before the breakup is applied (to be confirmed in the test).
2. **Surface relief and specular.**
   - **Earth, Moon and Mars:** a normal map baked from a real elevation model.
     For Earth, NOAA ETOPO 2022 at 30″ (about 926 m) or 15″ (about 463 m).
   - **Water:** a mask (or the Solar System Scope specular map) giving sun
     glint.
   - **All bodies:** object-space procedural micro-relief (no UV seam, no pole
     pinch), modulated by the base colour and a land, ocean or ice class.
3. **Lighting.**
   - Either a lit material, or an extended custom model with the normal,
     specular, a soft terminator and atmospheric attenuation at the limb.
   - Calibrate the emissive or tonemapping against the source colours (F6).
4. **Encoding and filtering.**
   - BC7 for albedo, BC5 for normals, BC4 for masks.
   - A **separate texture group for celestial bodies**, for example
     `TEXTUREGROUP_Project01` set to "Planets", with linear mip filtering. The
     approved ship's group stays untouched.
5. **Geometry.** A 256 × 128 sphere, or better a cube-sphere for the planet and
   the cloud shell, which also removes pole pinch.

**Results**

| Aspect | Assessment |
| --- | --- |
| Quality up close | Perceived sharpness comes from 1–8 km scale detail (relief, cloud edges, glint), which is 2–5× finer than the texel. Clouds become separate layered shapes with shadows. Coastlines are still limited by the base texel (4.9 km; with the step-2 16K base, 2.4 km). |
| Quality far | The detail fades out with distance (MIP/derivative-driven), so full discs do not flicker. |
| Memory | Earth step 1 with the 8K base: day BC7 43 MB + clouds BC4 21 MB + night 21 MB + normal BC5 43 MB, about 130 MB. That is about 2× today's 64 MB. |
| Effort | Medium: materials, the cloud shell, elevation preparation and parameters per body class. |
| Maintenance | Good: one master material with body parameters. |
| Coverage | **All types.** Rocky and icy bodies (relief), gas giants (detail flowing along the bands, no relief), clouds of any body, moons, and fictional worlds (procedural is unlimited). |
| Integrity | Procedural detail is presentation, not data. It must not pretend to be measured geography. Real coastlines and colours stay from the data. |

### C: Tiled or virtual textures on a cube-sphere (only where real data exists)

**Scope**

- The Earth surface as 6 cube faces at 8K or higher, in streaming virtual
  textures (SVT/UDIM).
- Sources: BMNG at 15″ (463 m) and ETOPO at 15″.
- 6 × 8K faces give about 1.2 km per texel almost uniformly, about 4× better
  than an 8K equirectangular map. At the REFERENCE pose that is about 1.4 px per
  texel.

**Results**

| Aspect | Assessment |
| --- | --- |
| Quality | Real geographic detail up close, no poles. |
| Effort | High: reprojection tooling (GDAL or our own), SVT set up and tested in a package, cube-sphere geometry. |
| Data | Earth alone needs about 0.5–2 GB of source plus cooked data. |
| Memory | Pages only for the visible region, given correct SVT settings. |
| Coverage | Only Earth, the Moon, Mars and Mercury. Not gas giants or fictional worlds. |
| Clouds | A comparably sharp global cloud source has to be verified. Until then the clouds stay variant B. |

### D: Regenerate everything to 4K (the user's idea, for comparison)

- **Simple**, but **does not solve close orbit**.
- **Worse** for the 8K bodies.
- Useful only as part of the rollout for bodies at 1440–2048.

### Comparison

| | Up close | Far | Memory | Effort | Coverage of all types |
| --- | --- | --- | --- | --- | --- |
| A | + | ~ | − | low/medium | partial |
| **B** | **++** (perceived) | + | ~ | medium | **yes** |
| C | +++ (real) | + | ~ (SVT) | high | Earth, Moon, Mars only |
| D | 0/− | +/− | ~ | low | yes, but no help |

## 4. Recommendation

**B as the common foundation for all bodies,** plus three Earth-only steps:

- **A's 16K base:** as step 2 for Earth only, if the coastlines in the test
  are still too soft.
- **C:** only if the user wants real geography at close orbit after
  step 2.

**Why B:**

- It addresses all the observed causes F2–F7.
- It works for gas giants and fictional worlds.
- It is memory-efficient and has a predictable effort.

**Why not plain resolution:** resolution alone (A, D) leaves the clouds in the
ground, the flatness and the blocks unsolved.

## 5. First verification step: Earth only, no bulk changes

**Pose.** Fix one camera pose replicating REFERENCE:

- Earth close orbit, view of the British Isles, sun direction as in the photo;
- FOV 52°, native 2560 × 1440 plus a 3840 × 2160 capture.

The pose is added to the existing Sharp/Visual probe.

**Variants (new test assets only; the approved originals stay):**

| Variant | Content |
| --- | --- |
| V0 | Current state. |
| V1 | Encoding only: BC7 day, BC4 clouds, the planet group with linear mips. Isolates F3/F4. |
| V2 | V1 + separate cloud shell with edge breakup and shadow. Isolates F2. |
| V3 | V2 + ETOPO normal map, water specular, calibrated tonemapping. Isolates F5/F6. |
| V4 (optional) | V3 + 16K BMNG base. Tests A on top of B. |

**Measurements:**

- crops at 100 % of the coast, a cloud edge and the horizon;
- texture memory (`memreport` or `stat RHI`) and GPU time at 4K (`stat gpu`);
- the same full-disc pose, to check for flicker while rotating.

**Acceptance at the REFERENCE pose, at 100 %:**

1. No 4×4 blocks or brushed streaks over land.
2. Cloud edges have a transition of at most about 3 px, clouds read as a
   separate layer, and a shadow is visible.
3. Relief (Scotland, the Alps) is readable at an oblique sun.
4. Glint near the Sun's direction, and ocean and land colours are not visibly
   more saturated than the source maps.
5. A full disc does not flicker during rotation.
6. Earth texture memory is at most about 150 MB (B) or 300 MB (with V4).
   **UNKNOWN:** the user's GPU budget, which needs to be confirmed.
7. GPU frame time rises by at most about 1 ms at 4K.

The user makes the final visual call.

## 6. Rollout after Earth is accepted

1. **Moon, Mars, Mercury:**
   - B with relief from real elevation models: Moon LOLA, Mars MOLA,
     Mercury MESSENGER;
   - sources and licences to be verified, likely USGS/NASA public domain.
2. **Gas giants:**
   - B without relief, with detail flowing along the bands and a turbulence
     mask;
   - check better base sources than the 4096 or 2048 maps;
   - no invented storms presented as data.
3. **JPL 1440/720 moons:** replace them with better global mosaics where they
   exist (for example USGS Astrogeology; licence and resolution to be verified).
   Otherwise B detail over the existing map, labelled as presentation.
4. **Fictional systems 0015:**
   - `Tools/Prepare-FiveSystems.py` is deterministic. Regenerate headline
     worlds at 8K, add normal maps and use B detail, all procedural.
   - This needs a separate approval, because the 0015 assets are accepted.

## 7. Data, tools, origin and limits

**Sources**

- **NASA Blue Marble Next Generation:** a monthly cloud-free mosaic at about
  500 m. Data courtesy of Reto Stöckli (NASA/GSFC) and NASA Earth Observatory,
  per [SVS 3523](https://svs.gsfc.nasa.gov/3523/) and
  [Earth Observatory](https://science.nasa.gov/earth/earth-observatory/blue-marble-next-generation/).
  The exact tile dimensions and NASA's usage rules must be read on the
  download page before use.
- **NOAA ETOPO 2022:** 15″ (about 463 m), 30″ and 60″ grids in GeoTIFF or
  NetCDF. The required citation includes
  [DOI 10.25921/fd45-gt74](https://www.ncei.noaa.gov/products/etopo-global-relief-model).
- **Solar System Scope:** stays for the current maps. It is CC BY 4.0, "based
  on NASA elevation and imagery data"
  ([source](https://www.solarsystemscope.com/textures/)). The same provider
  lists an Earth normal and specular map at 8K. That would be a small targeted
  download for B step 1, with the same licence, after approval.
- **Global cloud data sharper than 8K:** not identified. Until then the
  clouds stay data at 8K plus procedural edges.

**Tools**

- Reprojection or tiling for C needs GDAL or our own Python tool, plus enough
  RAM (BMNG 15″ is 86400-class).
- Normal maps are baked from elevation in numpy or Blender in the background.

**Limits**

- Procedural detail is not a fact.
- Earth from a single date has no seasons.
- Clouds are static, apart from optional drift.

## 8. What I did not verify (UNKNOWN)

1. The actual cooked format in the user's build (DXT1 is inferred from tags
   and engine source) and any Oodle RDO. Next step: texture editor, "Format".
2. The tonemapper's exact share of the colour shift.
3. The REFERENCE pose and resolution (my magnification ±20 %).
4. The user's GPU and memory budget.
5. Whether an SVT package works with our launcher (only relevant for C).

## Evidence

All under `Tasks/0022/EVIDENCE/`:

- `earth_uk_sources_vs_reference.png`: Solar System Scope maps (CC BY 4.0)
  next to a REFERENCE crop.
- `earth_bc1_simulation.png`: three panels, uncompressed 6×, BC1-style 6× and
  REFERENCE. The REFERENCE panel there is only illustrative, not the same crop.
- `earth_maps_measurements.json`.

The analysis scripts stayed in ignored `.local/analysis`.

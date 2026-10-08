# 0019 handoff

Status: READY_FOR_REVIEW, not ACCEPTED. Coordinator alone owns Git/build/runtime.

Changed assigned new paths only:

- `Tools/Prepare-SolarDetails.py`: repeatable Pillow/numpy source resampling,
  masks, explicitly schematic fallback maps and physical-radius ring atlas recipe.
- `Art/Space/SolarDetails/Sources/`: unchanged public PIA20658 and PIA19866 JPEGs.
- `Art/Space/SolarDetails/Textures/`: 6 surface JPEGs, 2 observation masks,
  4 RGBA8192x32 ring atlases.
- `Art/Space/SolarDetails/surface-views.json`, `ring-views.json`, `SOURCE.md` and
  concise map/ring previews.
- This task's PROGRESS/HANDOFF files.

Integration contract:

1. `ring-views.json` top-level `views[]`; each `parentId`, `innerMetres`,
   `outerMetres`, `texture` path relative to SolarDetails. Radial U increases
   outward; V=0.5. Clamp, RGB sRGB, alpha linear and straight. Parent-centred
   equatorial ring orientation. Source component records preserved.
2. 31 source components, 30 rendered: Jupiter5, Saturn8, Uranus12, Neptune5.
   Zeta skipped rather than invented. F-ring and upper-limit widths are explicitly
   authored representatives. Exact pixel coverage preserves subpixel widths.
3. Neptune dedicated NSSDCA widths disagree with NASA Science's source table;
   both records and policy are documented. Do not silently rewrite canonical
   measurement data with these mixed-source display choices.
4. `surface-views.json` real map IDs `sol.pluto` and `sol.moon.jpl_901`.
   Pluto is a native2400x1200 partial monochrome mosaic resized4096x2048;
   Charon native9520x4760 resized4096x2048. Source black unknown regions are
   preserved, separate mask available. No true-color map or physical spin phase
   claimed. Do not treat black unmapped regions as measured dark geology.
5. Eris/Makemake/Haumea/Ceres maps are explicitly schematic placeholders,
   `observedTerrain:false`. Prefer separate reviewed real assets where available.
   Charon/Pluto far hemispheres have intrinsically lower observed resolution.
6. Alpha values are authored readability choices, not measured optical brightness;
   no azimuthal Neptune arcs, eccentric ring edges or vertical halo reconstruction.

Checks passed: two offline runs identical hashes for 12 textures + two view JSONs;
all image decode/dimensions/aspect; RGBA mode and identical rows; nonopaque alpha
and real gaps; 31 preserved/30 rendered counts; exact covered interval math.
Real mosaics and radial previews viewed. Source credit/licence URLs and hashes are
recorded. No Unreal/app/build/streaming/lighting test is claimed. No existing
SolarSystem/SolarCatalog/game file mutated; no Git action performed.

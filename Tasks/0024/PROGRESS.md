# 0024 progress

- Status: IN_PROGRESS. Owner: Codex.
- Inspected HUD location and0023 allowed paths: HUD implementation.cpp is
  separate from Claude's assigned header/planet/system sources.
- Current1280x720-based scale makes HUD three times larger at native4K.
  Runtime font already uses target-pixel sizing; preserve that sharpness.
- User chose new HUD and supplied STO references. Design preview only first,
  game integration follows user design evaluation.
- New green layout complete, actual mesh-derived Daedalus silhouette,
  separate four shield arcs, energy placeholders, local computer/throttle demo.
- Headless installed Edge render3840x2160 passes layout, text25.6px,
  computer/throttle interaction;390px layout no horizontal overflow.
- Docs/HUD.md and repeatable outline/template generator prepared.
- No game source/asset or user application changed.

## Revision after user rejected monochrome/card design

- Unframed transparent radar with three rings/cross/arrow; location labels above.
- Neutral text/hull, green radar, muted blue shields, amber drive/missiles,
  copper railguns, blue beams. No key hints, no throttle control slider.
- Energy reduced to secondary small readouts below hull, no dedicated panel.
- Three weapon selectors from actual WEAPON_MOUNTS, local-only highlight of
  38turret,16silo,4beam positions. No invented ammo/cooldown/fire behavior.
- Browser3840x2160:24.96px base text, zero radar border/transparent fill,
  three selectors, zero hints, all three exclusive zone groups PASS.
-390px layout has no horizontal overflow; screenshot inspected.
- New screenshot: ignored.local/hud/hud-minimal-4k.png. Game unchanged.
- No editor/build/package while Claude owns0023 applications.

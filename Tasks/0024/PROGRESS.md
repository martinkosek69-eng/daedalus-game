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

## Instrument revision after user rejected minimalism

- The minimalist revision is superseded. Preserved unframed radar and the
  four blue shield arc paths; hull now shares the green radar token.
- Added bevelled technical instrument housings, circular ship chassis,
  secondary four-channel energy instrument, segmented drive display and modes,
  detailed weapon modules, and a framed four-section onboard computer.
- Weapon diagrams use the actual mesh projection and source mount coordinates;
  bow VLS detail is enlarged. Selection still highlights only its real group.
- No fake energy/shield/ammo/cooldown values or functioning combat controls.
- Browser3840x2160:24.96px base text, no page errors/overflow/clipped labels,
  radar transparent/no border, all three exclusive weapon interactions PASS.
- Narrow390px preview reflows without horizontal overflow. Screenshots inspected:
  ignored.local/hud/hud-sto-4k.png and hud-sto-instruments-4k.png.
- Game integration NOT_STARTED; user design review pending, Claude owns0023 apps.

## Approved layout: runtime source checkpoint

- Status: IN_PROGRESS. User approved the instrument design and requested
  aligned panel sizes, game integration and a functioning minimap only.
- New SolarHUD.cpp owns DrawHUD; removed only the previous DrawHUD from
  SolarFlightGameMode.cpp. No header/flight/input/planet/data/asset changes.
- Lower panels share208 logical height and bottom baseline,868-wide assembly
  at1920x1080. Runtime text uses actual pixels; hull aspect stays unchanged.
- Read-only radar uses authoritative metre positions/heading, nearest physical
  context,1/2/5 scale, real known local bodies, selected off-range bearing and
  vertical hints. Camera movement does not affect the radar.
- Existing speed/throttle/profile/navigation/pause/safety warnings retained;
  remaining new systems have no actions or invented values.
- Generator now also emits runtime mesh-derived vector hull/mount data.
- Added radar coordinate/boundary tests; static API/source inspection complete.
- NOT YET COMPILED/TESTED IN UNREAL. Claude checkpointf5d95d7 still owns editor
  and all builds for0023. No ownership transfer or READY_FOR_REVIEW yet.
- Old package/launcher preserved. Next: await slot release, compile, run
  Daedalus.HUD.RadarCoordinates and a targeted native4K HUD rendering check,
  package independently, then point the user launcher to the verified package.

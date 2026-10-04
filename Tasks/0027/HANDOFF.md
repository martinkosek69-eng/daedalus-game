# 0027 — Selectable Ancient HUD for Aurora

Owner/coordinator: Codex. Technical acceptance on codex/solar-flight.
Base 13c9d300571925e5711c3fc80d0e64804c394bcf; source checkpoint ffe7831.
Main remains unchanged. Normal launcher selects Build-Ancient27; prior packages
and PlayerData are preserved. All source/settings/tools/notes are tracked.

## Delivered

- P → Settings on Aurora exposes Earth and Ancient HUD choices. New Ancient
  profiles default to Ancient; Daedalus always uses Earth. A presentation-only
  family map is in Content/Data/Solar/hud.json. Preference is saved in A-based
  GameUserSettings, separate from world save data, and survives ship swap/restart.
- Distinct original vector HUD: blue double-contour chamfered glass, amber/
  copper bands, cyan schematics, crystal-channel module, octagonal hull/shield
  instrument, angled thrust segments, drone/pulse diagrams and glyph-like
  ornaments. Matching computer, pause frame and map palette. The original Earth
  renderer is retained, using shared sharp drawing primitives and canonical radar.
- Reviewed original Jumper hologram, Orion console and Atlantis database frames
  informed the design. Full sources/interpretation boundaries: Docs/ANCIENT_HUD.md.
  No TV images, downloaded font, simulator executable or shader screenshots used
  as game assets. Decorative symbols are original, not an alleged translation.
- HUD reads real flight/radar/navigation measurements. Energy/weapons/shields/
  new computer actions remain deferred and display dashes; no fabricated readings.
  Style commands change UI preference only. Physics/assets/save schema untouched.

## Checks

- Initial compile rejected shadowed names and an implicit UE 5.8 JSON shared-string
  conversion; corrected naming and explicit FString conversion. Final game and
  editor targets and Windows package passed with existing toolchain settings.
- Two packaged native 3840×2160 processes passed: actual simulated mouse choice,
  default family/style, switch both ways, pause clock/flight preservation, Daedalus
  isolation, intervening ship swap, retained Earth choice after complete process
  restart and switch back to Ancient. Test user data stayed in a unique ignored
  A-based directory; real player settings were never used for testing.
- Inspected HUD above bright Earth and black space, settings page and Ancient
  galaxy map. Shortened the Aurora display name to avoid ellipsis afterward;
  repackaged and repeated only the targeted rendering/input phase. Persistence
  code stayed unchanged, so the completed second-process check was not repeated.
  Final Earth-mode screenshot checked for preserved original layout.
- No domain rules changed; 23 passing baseline automation checks from 0026 were
  not needlessly rerun. This task's verification is the native UI/restart boundary
  check, not a claim of newly executed domain tests or measured normal frame rate.
- Source review/diff check passed; owned build/game processes exited. Human apps,
  other worker checkouts and original model assets were not operated or rewritten.

## Reproduce / remaining review

Tools/SPUSTIT_LET_DAEDALA.cmd launches the delivery. P → Přepnout loď → Aurora
Class, then P → Nastavení → HUD: antická technika / HUD: pozemská technika.
P resumes. All canonical flight controls remain. Build/package through
Invoke-SolarFlight.ps1; Invoke-AncientHUDProbe.ps1 performs the two-process check.
Its -RenderOnly option repeats just rendering/input after a cosmetic correction.
Screens/logs stay ignored; public concise evidence is EVIDENCE.json.

User judges style, compactness and personal legibility. Current Ancient hull
projection adapter handles Aurora; a future hull still needs its reviewed
presentation adapter/outline as well as family mapping. No new ship gameplay,
menus for energy/combat, adjustable graphics presets or flight saving are implied.

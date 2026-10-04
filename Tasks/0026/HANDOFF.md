# 0026 — Aurora and clickable pause/ship menu

Owner/coordinator: Codex. Technical acceptance complete on codex/solar-flight.
Base: 62ec47d9a7a157ab5a845e7e21db5e9d2c4a1cd2. Reviewed source a193736 from
claude/aurora-ancient-warship is integrated. Main was not merged. The ordinary
launcher selects Build-Aurora26; Build-Integrated25 and PlayerData remain.

## Result

- Original saved Aurora scene inspected; geometry, axes, dimensions, shaders
  and source attribution reviewed. Original .blend unchanged. Export preserves
  the source's copper/steel paint, window material and blue emissive orb, with
  three slots, 647612 triangles, 8K colour and 4K ORM/emission/normal maps.
  Independent import recipe and full-detail resident texture policy are tracked.
- Separate data-only ship catalog preserves Daedalus tuning exactly. Aurora
  uses a 3500 m provisional source scale, 1950 m conservative clearance sphere,
  slower turning and stronger drive: normal impulse 350 km/s, max in 2 s.
  Full sublight 0.999c is an explicit game choice inspired by Tria, not a general
  canon performance claim. Research and assumptions are separated in AURORA.md.
- P opens a clickable pause menu. Choose Daedalus/Aurora there; resume or quit.
  Graphics page is a readout; flight save/load entries are explicitly unavailable.
  Final game's ESC menu binding is deferred; current ESC closes menu/map and
  retains the old lab quit shortcut outside them. No F1/F2 ship binding remains.
- Swapping preserves canonical flight pose/speed/throttle/clock/navigation,
  refits camera and HUD, and safely rejects incompatible hull clearance/attitude.
  Visuals never mutate domain state. Latest Daedalus H doors, beacons and engine
  lights work after switching back. No combat, energy gameplay, new hyperspace
  or flight-lab persistence is introduced.

## Checks

- Source/export hashes, UVs, finite data, dimensions, slots and map sizes passed.
  Cleaned 11685 near-zero STL faces; 38 numerical tiny residual faces below
  1e-9 m² remain, with zero nonmanifold edges. Measured hull radius 1843.215 m
  fits the conservative 1950 m sphere. Source .blend hash remains unchanged.
- Owned independent Unreal import, editor build and Windows package passed.
  All 23 domain/UI-projection automation tests passed, including three new
  catalog validation/rollback, Aurora drive/turn/no-drift and surface swap groups.
  Later menu/probe-only corrections were compiled/packaged and exercised in the
  targeted package check; the unchanged domain suite was not repeated.
- Final packaged 3840×2160 probe passed through actual simulated mouse input:
  pause/cursor, selection/back/resume, paused moving swaps, settings/save pages,
  8K/4K cooked texture residency, impulse/no drift, map guard, restored four
  Daedalus doors/six glows/ten lights, H animation and actual quit-button exit.
  Five images were saved and visually inspected across the two package probes;
  final ship selection/restored Daedalus images were checked after the screenshot
  timing correction. Output remains ignored. See EVIDENCE.json.
- All owned applications exited. No human app or Claude checkout was operated.

## Reproduce / limits

Double-click Tools/SPUSTIT_LET_DAEDALA.cmd. P → Přepnout loď → Aurora Class.
Prepare-Aurora.py reads the saved source in background Blender; Validate-Aurora.py
checks export/source. Invoke-SolarFlight.ps1 AuroraAssets imports only this hull;
Assets runs both independent recipes. Build/Test/Package then Invoke-AuroraProbe.ps1
reproduce technical checks. Local settings and all outputs stay on A: and ignored.

Aurora's provided scene has no separated animated hangar doors/engine modules;
none are invented. Its orbit view shares the system's lighting, rather than the
Blender preview studio. 4K output and full source maps do not provide unlimited
close-up texture detail or prove normal frame rate. User performs most subjective
flight/appearance evaluation. Source/license attribution is retained; working
scale and drive settings remain editable. Foundation save contracts are separate.

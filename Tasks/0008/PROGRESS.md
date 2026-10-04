# 0008 progress

- State: READY_FOR_REVIEW (not ACCEPTED)
- Worker: Claude Code (launch relayed by the user)
- Branch: `task/0008-daedalus-engine-effects`
- Base: `98c4bd137703e675a51af36666d2e35bae1e724a`. This is the latest
  `codex/solar-flight` containing the revised brief.
- Last update: 2026-10-04, Europe/Prague

## User clarifications during the task

- **Proportions.** Do not change them; only colours, lighting and effects
  around the hull. The hull vertices remain exactly the Astrofossil geometry
  (222330 triangles, 600 m).
- **Removed fittings.** The dark hatch squares and vent slats are not
  original, so they were removed. The windows became a separate light export.
- **Asgard beams and effects (U9–U12).** 2 Asgard beams at the bow and 2 under
  the hull between the hangars. Weapon effects: orange railgun projectiles,
  missiles from the bow silos, blue beams.
- **First detailed look rejected.** Free rein to match the series. This gave
  the procedural plating, the U13 palette and the look-dev (`3effaca`).
- **Final pass ("almost there; finalize it"), main target the 4K still U15:**
  - VLS hatches exactly like the close-up U16,
  - hangars like U17,
  - small faint blue windows (U18–U20),
  - blinking mast-tip lights (about once every 5 s),
  - engines with visible internal turbine vanes (U21),
  - overall tone and light from U22 (tone only),
  - remove the twin barrels added on the turrets, because the original domes
    already have guns (U23–U25).

## Done (final pass)

- **VLS hatches.** 16 overlay slabs on the measured hatch plates in
  `DaedalusAddOns.glb`, each with procedural hatch textures: taupe door, frame
  and lip, slot with tick marks, grey end tabs with bolts. Hazard stripes on
  alternate hatches (checkerboard).
- **Hangars.** A new back wall per bay in front of the model's X-truss frame,
  with a hexagonal split observation window (dark green glass), light frame
  bars and fixtures. Spotlights above each opening, floodlights under the pod
  edges, nav lights (starboard green, port red).
- **Windows.** Faint cyan windows: a row of 7 in the now-dark bow band, rows on
  the bridge tower tiers, and the 30 baseline windows dimmed.
- **Masts and beacons.** Two masts on the bridge top tier and three tall masts
  beside the tower, plus 3 orange beacons in `DaedalusBeacons.glb`. They blink
  once every 5 s for 1/3 s (object extras, plus a preview animation in the
  .blend).
- **Engines.** The turbine vane and hub faces use the new hull slot
  `Daedalus_EngineInner` (warm-lit, yellow glow). In the glow GLB, the core disc
  sits behind the vanes, a point light in front of the vanes lights them, and
  the haze is fainter.
- **Look-dev.** A cooler Earth/sky bounce and fill, from the U22 daylight
  tone.
- **Turrets.** `DaedalusTurrets.glb` and its barrel objects were removed; the
  original dome turrets are kept.
- **Checks.** Renders compared against U13/U15/U16/U17/U18–U21/U22 in
  `.local/daedalus-detail/final` (ignored). The pipeline asserts and mount
  verification pass.

## Checkpoints

- `06312a0`: first checkpoint
- `991a823`: first READY_FOR_REVIEW
- `d5846a1`: Asgard mounts and silo plates (later reworked)
- `3effaca`: series-look pass (plating, palette, look-dev)
- `1d1b442`: final pass (hatches, hangars, windows, beacons, engines)
- next commit: fixes from user screenshots U26–U31 (clipped hangar wall, bay
  detail and lights, forward superstructure window and units, darker hull),
  READY_FOR_REVIEW

## Next

Codex review and import in its own Unreal copy. See HANDOFF for the material
setup (textures × COLOR_0) and beacon blinking.

# 0008 progress

- State: READY_FOR_REVIEW (not ACCEPTED)
- Worker: Claude Code (launch relayed by the user)
- Branch: `task/0008-daedalus-engine-effects`
- Base: `98c4bd137703e675a51af36666d2e35bae1e724a`. This is the latest
  `codex/solar-flight` containing the revised brief. The task was not started
  before.
- Last update: 2026-10-03, Europe/Prague

## User clarifications during the task

- "Do not change the model proportions; only colours and lighting, plus
  effects around the hull, not the model itself."
- The baseline's dark hatch squares are not in the original model and should
  go.
  - All 14 hatch squares and 36 vent slats were removed.
  - The 30 window boxes moved to a separate `DaedalusLights.glb`.
  - The hull is now the exact Astrofossil geometry: 222330 triangles at
    600 m, with proportions unchanged.
- After `991a823` the user added references U9–U12 and asked for two more
  changes:
  - Bow silo hatch plates in the orange of U11 (plain paint, natural
    intensity).
  - 4 Asgard beam weapons: 2 at the bow (the "Asgard beam turret" on the U12
    schematic) and 2 under the hull between the hangars. Each still shows one
    side; the other mirrors it.
  - The user also described the weapon effects: orange railgun projectiles,
    missiles from the bow silos, blue beams.

## Done

- **Reference study.** Art/Ships/Daedalus/REFERENCES.md records:
  - source URLs,
  - evidence classes,
  - the mapping to the model.

  User images are not redistributed.
- **Tools/Prepare-DaedalusDetail.py** is a reproducible pipeline:
  1. imports Original,
  2. paints the COLOR_0 palette with baked AO,
  3. assigns the engine metal, hangar interior and bow light slots,
  4. measures engines, turrets, VLS and bays,
  5. writes the mounts and empties,
  6. exports three GLBs,
  7. reopens and re-imports them with assertions.
- **Tools/Prepare-DaedalusEngineEffects.py** holds the outlet measurement and
  glow library. It can also run standalone.
- **Measured:**
  - 6 engine outlets,
  - 26 dorsal and 12 ventral railgun domes,
  - 16 bow VLS silos (matches the wiki count) with their orange hatch plates,
  - 2 F-302 bays,
  - 4 Asgard beam weapons.
- **Mount axes.** Conventions are documented in WEAPON_MOUNTS.json. The
  empties in the .blend were verified against the JSON after reopening.
- **Checks.** Final renders were made in .local/daedalus-detail/renders
  (ignored). All checks pass (see HANDOFF).
- **Blender use.** Only background Blender was used for the assets. A separate
  GUI window showed an ignored preview copy at the user's request.

## Checkpoints

- `06312a0`: first checkpoint (exports and mounts)
- `991a823`: READY_FOR_REVIEW (SOURCE/REFERENCES notes, mount axis conventions
  and assertions, HANDOFF)
- next commit: READY_FOR_REVIEW again, with orange silos, the Asgard beam
  mounts and the weapon effect hints

## Next

Codex review and import in its own Unreal copy.

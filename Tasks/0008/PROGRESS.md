# 0008 progress

- State: IN_PROGRESS (first checkpoint)
- Worker: Claude Code (launch relayed by the user)
- Branch: `task/0008-daedalus-engine-effects`
- Base: `98c4bd137703e675a51af36666d2e35bae1e724a` (latest `codex/solar-flight`
  containing the revised brief; task was not started before)
- Last update: 2026-10-03, Europe/Prague

## User clarification during the task

"Do not change the model proportions; only colours and lighting, plus effects
around the hull, not the model itself."

- No hull geometry is added or reshaped.
- Mounts are measured points only.

## Done

- Reference study in Art/Ships/Daedalus/REFERENCES.md. It records source URLs
  and evidence classes. User images are kept local and not redistributed.
- Tools/Prepare-DaedalusDetail.py rebuilds the baseline geometry from Original
  and asserts the counts. It then:
  - repaints COLOR_0 with the reference palette, plate patches and baked AO,
  - assigns engine metal, hangar interior and bow light slots to existing
    faces,
  - measures engines, turret domes, bow silos and hangar openings with
    ray-cast depth maps,
  - writes ENGINE_MOUNTS.json and WEAPON_MOUNTS.json,
  - exports Daedalus.glb (one hull mesh) and DaedalusEngineGlow.glb,
  - reopens and re-imports the exports for checks.
- Tools/Prepare-DaedalusEngineEffects.py holds the engine outlet measurement
  and glow builder. It also runs standalone.
- Measured:
  - 6 engine outlets (main 17.5 m, pods 13 m aperture),
  - 28 dorsal and 12 ventral turret domes,
  - 16 bow VLS silos (matches the wiki count),
  - 2 F-302 bay openings.
- Background Blender only. At the user's request, a separate GUI window shows
  a live preview copy (.local/daedalus-detail/live.blend). It is not an asset.

## Next

Final colour review against the stills and renders of all required views.
Then fill HANDOFF with checks and limitations and push READY_FOR_REVIEW.

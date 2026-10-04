# 0024 handoff

Status: READY_FOR_REVIEW (instrument design only; user feedback pending).

Owner: Codex. Branch: codex/solar-flight. Head via Git history.
Files: Preview/**, Tools/Prepare-HudOutline.py, Docs/HUD.md, task docs.

The latest instrument design supersedes both the monochrome and minimalist
prototypes. The unframed radar remains unchanged. Real model-derived hull
outline is green; four shield arcs remain blue. Bevelled instrument housings,
secondary energy channels, segmented drive display, detailed weapon-group
diagrams and framed onboard computer now follow the requested STO direction.
No online UI, key hints, new gameplay or invented shield/energy/ammo values.

Weapon diagrams derive from the same mesh projection and WEAPON_MOUNTS data:
38railgun mounts,16VLS mounts,4beam mounts. These are measured model features,
not accepted combat counts. Local selectors exclusively highlight each group
and deselect correctly. Onboard computer switches demonstration content only.

Standalone preview: Tasks/0024/Preview/hud-concept.html, after Git LFS pull.
Regenerate with Python3/Pillow Tools/Prepare-HudOutline.py when geometry changes,
then node Tasks/0024/Preview/build-preview.cjs after template changes.
No server, extra dependencies or live editor needed for the HTML preview.

Browser check: native3840x2160,24.96px base text, no page errors, horizontal
overflow or clipped tested labels. Transparent radar/no border, three weapon
selectors, no key hints. Computer and all three weapon interactions PASS.
390px adaptive layout has no horizontal overflow. Inspected screenshots in
ignored.local/hud/hud-sto-4k.png and hud-sto-instruments-4k.png.
Reference background does not establish current scene quality; the check
verifies the new HUD at native output pixels.

No game source/asset changes, Unreal/build/package or current launcher changes.
Claude retains0023 editor/build ownership. Next: user evaluates this appearance;
implement the selected design only after approval and app coordination.

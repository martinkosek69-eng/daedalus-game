# 0024 handoff

Status: READY_FOR_REVIEW (design only; user feedback pending).

Owner: Codex. Branch: codex/solar-flight. Head via Git history.
Files: Preview/**, Tools/Prepare-HudOutline.py, Docs/HUD.md, task docs.
Standalone preview has original green HUD layout, real model-derived outline,
four shield arcs, schematical minimap, placeholders and local demo controls.
4K screenshot saved in ignored.local/hud/hud-concept-4k.png. Browser check:
3840x2160,25.6px text, no page errors/overflow, computer/throttle interactions
PASS.390px adaptive preview no horizontal overflow. Screenshot inspected.
No Unreal/game build/test done or claimed. No changes to gameplay/C++/assets.
Next: user evaluates appearance; refine before implementing in game. Claude
retains all0023 app ownership. Visual reference background does not establish
4K scene quality; the new HUD is rendered at native output pixels.

Latest revision supersedes the monochrome prototype above. See Docs/HUD.md:
transparent unframed radar, function-specific muted colours, secondary energy,
no key hints/throttle slider and three weapon selectors. Model-based projected
positions use38railgun mounts,16VLS mounts,4beam mounts; selectors highlight
only their group and deselect correctly. Native4K render24.96px base text,
no overflow/errors, radar no border/background, keyboard hint count0. Browser
check PASS. New inspected screenshot.local/hud/hud-minimal-4k.png.
Still design review only, no game changes or new combat functionality.

# 0010 — Complete solar-system flight playground

Owner/integrator: Codex. Base c56ff1d, branch codex/solar-flight.
Target codex/game-foundation, draft review only; singleplayer.
Allowed: solar Unreal adapter/content/config, Art/Space, Tools, public Docs/Tasks.
Root owns Unreal editor/build/import and ship integration. No user application
may be closed. Claude's ship source branch is read-only until final handoff.

Build the complete solar system from the web reference: Sun, eight planets,
principal moons, rings and asteroid belt, detailed sourced surfaces, prominent
stars and readable movement cues. Canonical body definitions use double metres
and stable IDs; visuals project from those values without writing simulation
state. Add simple body selection/inspection and explicitly marked local test
reposition commands so every planet can be tested without hours of flight.
Do not add hyperspace/combat. Solar flight-lab persistence remains future work.

Reach full speed within three seconds, eliminate lateral drift, smoothly settle
bank and pitch limits (0011). Provide native-resolution rendering, readable 4K
HUD and verify actual 3840x2160 pixels; avoid temporal smear/upscaling. User may
change fullscreen/window size. Keep a new package separate from prior builds.
Review new Claude ship source when available; it does not gate completion.
Check all required assets/licences, builds, domain tests, packaged input/render
probe at native 4K and foundation restart. Publish checkpoints and source LFS.

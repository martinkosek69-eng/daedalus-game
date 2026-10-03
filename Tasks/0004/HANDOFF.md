# 0004 handoff — first playable solar flight lab

Owner/reviewer Codex; branch `codex/solar-flight`, target
`codex/game-foundation` for review. Main has not been merged. Pure singleplayer.

## Delivered

- Original prototype Daedalus recovered and imported at60000cm; Earth with8k
  day/night/clouds, static Sun near1AU, black prototype HYG star background.
- Value-only assisted flight domain, configurable persistent throttle, reverse,
  acceleration/coast/brake, limited pitch and smooth turn/bank/drift. New solar
  adapter, level chase/orbit/zoom camera and compact Czech flight HUD.
- Physical local double metres remain authoritative. Camera-centered distant
  body projection preserves angular size; visual scales do not affect collisions.
- Source Blender/GLB, licensed textures, Unreal assets and repeatable tooling;
  Windows launcher `Tools/SPUSTIT_LET_DAEDALA.cmd`. See Docs/SOLAR_FLIGHT.md.
- Claude0005 report reviewed and integrated preserving authorship. Additional
  broad STO report retained as advisory reference, not a game-wide design order.

## Verification performed 2026-10-03

Editor C++ build and Windows Development Package succeed. All15 named automation
groups succeeded, zero failed/unrun/in-process:11 existing foundation groups
plus four flight groups covering throttle/coast/reverse/brake including sideways
slip, bounded yaw/pitch/bank/drift,60vs144fps partitions/pause/backlog/invalid
inputs,1AU coordinates and fast swept surface contact without tunneling.

The final staged IoStore game passes an880-frame controller-key probe: content,
8920 stars, persistent20% E throttle, R zero/full, D/W turn/pitch, acceleration,
Space brake to exact rest, Q quarter reverse, reset, both profiles, pause clock,
zoom/level camera, Home return, fixed-step stability and actual screenshots.
Earth, turning and Sun screenshots inspected. This controlled-time test is not
an FPS benchmark. Material usage warnings and legacy Nanite streaming warning
were resolved in the final package. Normal startup shader warmup can still occur.

The final package also passes two distinct foundation processes (write/read),
retaining persistent state and correct scene lifecycle. The solar lab itself
starts afresh and does not yet save its flight pose. Repeated asset recipe hits
the SHA256 cache, zero commandlet errors/warnings, and leaves map SHA256 unchanged:
5e6a29e499857b7ced170ded955457b04214b3a39d73e6dc4c0dad8899b8fb37.

Local ignored evidence: `.local/solar/tests-c88ebdf4b25b408aa76d34e07b805d6a`,
`visual-950f484b110540568bc16f3e690a30fd`,
`foundation-restart-60af2cd278c24a5cb8cc99c1d9e72a21`, build5.log,
package3.log and assets.log. Public documentation summarizes checks; raw machine
logs, player data and packaged binaries stay local. Source LFS delivery is
checked separately after publication; do not confuse a local save with a push.

## Limits / next step

Flight is our tunable STO-inspired approximation, not copied proprietary code
or measured exact STO physics. Static reference bodies have no gravity/orbits.
Surface safety uses a conservative ship sphere. No warp, combat, system travel,
landing, atmospheric simulation or flight-lab save is claimed.

Baseline ship coloring does not yet meet the user's photo-based request.
Task0008 is published for Claude: original SG BC-304 repaint/details, engine
outlets/effects, separate movable weapon pieces and measured mount manifests.
No worker branch/delivery existed at last fetch. User relays START_CLAUDE.txt;
Codex will inspect source, scale, material dependencies and import before
accepting. No permanent world/ship role split. Background Blender and source
binaries are reserved for Claude; Codex uses its own Unreal baseline import.

First user flight feedback should tune speed/turn/inertia/camera before adding
other planets or combat. Integrate the selected flight into persistent world
ship state with an explicit migration and restart tests as a separate change.

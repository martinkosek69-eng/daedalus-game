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

Task0008 subsequently arrived and is ACCEPTED/integrated: photo-based SG BC-304
paint, separate lights, six throttle-responsive engine effects and measured
weapon/bay points. Codex owns source integration after handoff. Moving turret
geometry/firing are later work. Review/limits in Tasks/0008/REVIEW.md; there are
no permanent world/ship roles. Final checks rerun after the new art: all15 tests,
render/controller probe including actual orbit/zoom and motor brightness,
foundation separate-process restart through Windows PowerShell5.1, cached asset
recipe and unchanged map SHA256. Current evidence supersedes the baseline logs:
tests-614dece1fc464cf297eff6533c29417c,
visual-4436bc494a0146b29cbcfae115acc493,
foundation-restart-ecf22534fbe6430f9feb317aad0b6aa0,
build-claude.log and package-claude.log.

First user flight feedback should tune speed/turn/inertia/camera before adding
other planets or combat. Integrate the selected flight into persistent world
ship state with an explicit migration and restart tests as a separate change.

# 0029 handoff — READY_FOR_REVIEW

Codex, 2026-10-08. Source checkpoint 2295b6b on codex/solar-flight.
Singleplayer, presentation only. User appearance approval remains pending.

Replaced 0028's electrical web with finite tapered tears, translucent folded
green sheets, irregular broken bright edges and a mint-white core. Native
per-pixel Unreal material; Blender was not used. See REFERENCE_REVIEW.md for
supplied-footage observations and online imagery actually inspected.

The separate Build-Hyperspace29 rehearsal loops opening, bow-first entry and
closure. Tools/SPUSTIT_HYPERPROSTOR.cmd launches it; R/1 restarts, Space pauses,
Escape quits. Normal flight launcher and its player data are unchanged. Earlier
blue transit remains optional via Invoke-HyperspacePreview.ps1 -IncludeTransit.
No flight-domain, ship/planet assets or ordinary lighting changes.

## Verification

- Material generation, C++ build and final package passed.
- Final capture: .local/hyper/render-20261008-214250, 116 frames at 3840x2160;
  runtime passed, canonical flight state unchanged.
- Check-HyperspaceCapture.py passed: opening frame24=19867 changed bright pixels,
  full frame38=352223, closed frame103=0 in the documented fixed-camera ROI.
- Inspected frames24,38,61,103, including frame38 at original native resolution.
- .local/hyper/Daedalus-green-window-4K.mp4 decoded all 116 frames, 30 fps,
  3.87 seconds, 3840x2160. Hashes in EVIDENCE.json. Raw media/logs remain ignored.
- PowerShell parser passed. Full 24-test domain suite was not rerun: no domain
  or timeline changes; previous 0028 suite passed and new capture tests state.

First revision was technically valid but rejected internally as neon-like lines.
Second revision exposed late initial material visibility despite file/state
checks passing. Added explicit PSO readiness/zero-strength draw warmup and a
content regression check; the final early frames now contain the aperture.

## Limits / review

This is a revised visual proposal, not an assertion of photorealism or an exact
match to studio VFX. Six layered noise samples approximate visual depth; they
are not a volumetric simulation. Glow/bright core intentionally soften some
light, while contour detail is generated at output resolution. User judges
the resulting shape, colour, brightness and motion. Real-time performance was
not benchmarked; offline capture is not FPS evidence. Sound and actual travel
integration are outside this task. All owned build/capture/encoder processes
finished; application slot released. No main merge performed.

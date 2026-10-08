# 0029 progress

READY_FOR_REVIEW. User rejected 0028 appearance; technical pass did not establish
visual acceptance. Original supplied full frames revisited; web research done
and supplemental series imagery being inspected. Codex owns one sequential
material/build/capture slot in its fc0b checkout; initial inventory found none
of UnrealEditor, UnrealEditor-Cmd or Daedalus running.

First revision compiled/packaged and passed a ten-frame native 4K capture, but
visual review found overly uniform bright creases. Reworked it into six sampled
depth layers with ragged, interrupted sheet edges, varying translucency and
fewer dominant long tears. Second native preview built/captured. Visual review
found a cold PSO delay: early aperture frames were missing despite the older
file/state-only capture check passing. Added material readiness gating and a
zero-strength draw warmup before the cinematic, plus an image-content check
that detects missing opening/full-window pixels and residual closure light.
Default preview now loops just the window; old transit is explicitly optional.
Final build/package and 116-frame native 4K capture passed. Image-content check
confirms opening, full aperture and clean closure; MP4 decoded all 116 frames.
Selected frames inspected, including full aperture at original 3840x2160.
Source checkpoint 2295b6b published. Appearance awaits the user's judgement;
no photorealistic likeness acceptance claimed. Owned processes exited.
Material helper now preserves assets whose source recipe is unchanged; restored
the unchanged tunnel binary after first generator rewrote its expression IDs.
No flight-domain, ship/planet art or ordinary lighting changes.

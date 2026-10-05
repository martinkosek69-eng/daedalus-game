# 0028 — Daedalus hyperspace presentation

ACCEPTED technically by the coordinator. User reference-likeness review pending.
Owner: Codex. Branch: codex/solar-flight. Base: ef356d35e60bcf8720feb8df8c88b0231794387e.
Initial source checkpoint: da964ba. Final delivery follows it on this branch.

## Result

Separate reusable Unreal animation: Daedalus acceleration, expanding white/mint
green energy window, bow-first depth-tested crossing, delayed collapse to a point,
and a blue/cyan moving transit tunnel. Existing reviewed Daedalus model is reused.
The three supplied excerpts were inspected as contact sheets and full frames;
timing/colour notes are in Docs/HYPERSPACE_PRESENTATION.md, hashes in REFERENCES.json.
Original procedural shaders render directly at native resolution; no movie imagery
or audio is incorporated. Reference material stays in ignored A-based storage.

Source: Art/Effects/Hyperspace, Content/Data/Hyperspace/entry.json, generated
Content/Hyperspace/Materials, Solar/HyperspaceTimeline and Solar/SolarHyperspace.
Preparation/capture helpers are Tools/Prepare-HyperspaceMaterials.py,
Tools/Inspect-HyperspaceReferences.py and Tools/Invoke-HyperspacePreview.ps1.

## Review on this PC

Launch Tools/SPUSTIT_HYPERPROSTOR.cmd after packaging Build-Hyperspace28.
R or 1 restarts, Space pauses/resumes, 2 jumps to the interior, Escape exits.
It loops automatically. Keyboard bindings are implemented but not end-to-end
input-probed; subjective playback and timing review is left to the user.
The local silent video is .local/hyper/Daedalus-hyperspace-4K.mp4: 3840x2160,
30 fps, 10.70 seconds. Regenerate it with the helper's Render mode.

## Checks and corrections

- Full game/editor build and final separate package passed. Both materials cooked.
- All 24 automated checks passed, including HyperspaceTimeline: validated input,
  aperture/ship crossing times, continuous run-up speed, complete hull clearance
  before closure, complete collapse and transit activation.
- Initial ten-frame 4K still capture passed; inspected and reduced aperture size,
  adjusted energy structure and darkened blue tunnel channels.
- Final render passed: 321 native 3840x2160 frames. Formation, crossing, late
  collapse, empty exterior and transit frames visually inspected. MP4 metadata
  independently read as native 3840x2160/30 fps, 10.70 seconds.
- Runtime capture verified canonical position/velocity/simulation time unchanged.
- Python/PowerShell syntax and git diff whitespace checks passed.
- Initial material generation failed on Unreal's CustomInput constructor; fixed
  by setting its input_name property. Re-generation and shader/package succeeded.
- Own processes exited. No human application or worker checkout was touched.

## Scope and limits

This is an animation rehearsal, not functional travel/navigation. The exterior
and interior are joined by a cinematic cut, not a traversable continuous portal.
Source playback timings are estimates, not recovered production animation data;
no pixel-identical match is claimed. Sound, Aurora and travel controls are deferred.
The normal flight launcher still opens Build-Ancient27, with its existing data.
The preview uses isolated user data and does not advance canonical flight state.
Native 4K technical acceptance does not replace the user's visual approval.

# Daedalus: hyperspace window rehearsal (0029)

Pure singleplayer presentation rehearsal, launched separately through
`Tools/SPUSTIT_HYPERPROSTOR.cmd`. R or 1 replays the entry, Space pauses/resumes,
Escape closes the preview. It loops automatically. Default is window-only.
The optional -IncludeTransit helper switch restores the earlier tunnel and key 2.
Normal flight and its launcher remain the accepted 0027 delivery while the
user reviews this animation. This is not a destination/travel/save implementation.

## Supplied reference analysis

The user supplied three original-series excerpts. Local copies/contact sheets
stay ignored on A:. No movie frames, audio, VLC chrome or footage are game assets.
Source hashes are recorded in Tasks/0028/REFERENCES.json.

- entry-1: 11.12 s, 1920×1036 capture. Initial player pause/ordinary flight and a
  blue defensive shield in front of the ship are not the hyperspace window. The
  relevant isolated window appears around 8.1 s, fills around 8.8–9.0 s, receives
  the ship around 9.2–9.5 s and contracts to a pinprick around 10.3–10.5 s.
- entry-2: 12.60 s, 1920×1036. Relevant first entry begins around 2.4 s; aperture
  growth ~0.7–0.9 s, hull crossing around 3.7–4.0 s. The window holds briefly then
  contracts over about one second. A second larger vessel enters later; this is
  useful aperture structure reference, not the Daedalus motion/scale reference.
- transit: 8.58 s, 1920×1080. Initial repeated paused frames excluded from motion
  judgement. Interior is a deep cobalt tunnel with broad cyan/near-white flowing
  lobes and dark longitudinal channels. Camera travels past the forward hull.

Times are approximate readings from 4 samples/second and selected full-resolution
frames, not claims of recovered production curves or physical speeds. The source
has capture/player chrome and compression; native 4K output does not invent
missing measured reference detail.

Window structure is a filled luminous aperture, irregular around the edge, with
a near-white mint core, green/turquoise translucent sheets and thin trailing tears.
It is not an empty geometric ring. A sample from entry-1 at 9.25 s gave sRGB
median [233,249,243] for the bright core and [76,128,128] for selected green/cyan
midtones (selection-dependent, not a complete calibrated colour transform).
Preserve near-white energy, green/cyan halo and black surrounding space together.

## Revised implementation

Art/Effects/Hyperspace contains editable procedural HLSL, reconstructed from the
observed structure. Prepare-HyperspaceMaterials.py generates only two owned Unreal
materials, preserving each asset when its source recipe is unchanged. 0028's
dense electrical-web appearance was rejected. 0029 uses finite tapered tears,
eight dominant irregular extensions, broken luminous edges and six sampled
noise layers for folded translucent sheets. These are artistically layered
fields, not a physical volumetric simulation. Local glow remains restrained.
No low-resolution
image sequence is enlarged. The translucent plane depth-tests against the hull:
the advancing bow becomes occluded by the core while the stern remains visible.
Ship visibility ends only after the complete 600 m hull has cleared the plane.
The green point light gives the hull a local reflection; real model engine outlets
brighten during run-up. The stable window plane does not chase the moving ship.

The separate blue tunnel is a continuous cylindrical perspective field with
angular/longitudinal motion, behind the intact 3D ship. A cinematic cut follows
the completed exterior closure. It is not an actual continuous spatial portal.
No ship-model stretching or scene-wide motion blur, temporal upscaling or reduced
render scale is enabled. Optical glow is enabled only in this separate preview.

Content/Data/Hyperspace/entry.json is the single authoring timeline in seconds
and metres. Opening starts at 0.5 s, reaches full size at 1.25 s, bow and stern
cross at 1.90/2.15 s, collapse starts 2.35 s and finishes 3.40 s; interior cut
at 3.85 s. Run-up joins crossing at continuous speed. These are artistic fits to
the supplied timings, not gameplay propulsion speeds. Exact style and timing
acceptance belongs to the user.

The window-only loop ends at 3.85 s. Material PSO readiness and a zero-strength
draw warmup precede playback/capture; otherwise first-launch pipeline creation
can cause early aperture frames to disappear. The separate visual regression
check catches this failure even when capture dimensions and flight state pass.

FHyperTimeline validates into a temporary value and samples deterministically.
SolarHyperspace is the scene adapter. It never advances or overwrites canonical
flight state, inter-system navigation or save data. Existing mesh assets and
planet binaries are not rewritten. Generated materials, timeline and original
shaders are reusable when an authorized travel command is added later.

## Reproduce

Configure engine and optional ffmpeg executable in ignored .local/toolchain.json
or DAEDALUS_FFMPEG. Run Invoke-HyperspacePreview.ps1 in Materials, then Package
mode, sequentially with no other editor/build/game using the shared slot.
Stills creates ten native 3840×2160 frames and checks flight-state preservation.
Render creates a deterministic native 4K/30 fps sequence and an MP4 preview under
.local/hyper, using an isolated user-data folder. Play launches the looping scene.
Default output is .local/hyper/Daedalus-green-window-4K.mp4 (116 frames).
Run Tools/Check-HyperspaceCapture.py with the Render output folder using a Python
environment with Pillow and NumPy. Its fixed-camera pixel tests check opening,
full aperture and closure, not artistic likeness. Verification status, failed
iterations and remaining limits are in Tasks/0029/HANDOFF.md; additional online
reference inspection is in Tasks/0029/REFERENCE_REVIEW.md.

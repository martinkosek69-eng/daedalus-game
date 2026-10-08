# Reference review and revised visual criteria

The user rejected the electrical/web appearance of 0028 and requested a sharp,
photorealistic-looking green rupture, preserving the already approved ship art.
Technical acceptance of 0028 is superseded by this appearance revision.

## Sources actually inspected

- The same three user-supplied local clips identified by hashes in
  ../0028/REFERENCES.json. Full-size entry-1 at 9.25 s and entry-2 at 3.70 s
  re-inspected: irregular white/green fissures separated by dark space and
  broader illuminated surfaces. Fine detail is concentrated at tearing edges.
- Online research: https://stargate.fandom.com/wiki/Hyperdrive and
  https://stargate.fandom.com/wiki/Hyperspace . Browsed the image gallery and
  visually inspected the original-series Experimental Jumper image, showing
  a small green-white ragged window ahead of the ship:
  https://static.wikia.nocookie.net/stargate/images/2/2d/Experimental_Jumper.png/revision/latest?cb=20130731185627
- Also viewed Katana Labrea's ship from that gallery: this is an interior
  tunnel shot, useful for separating the deferred transit look from the window.
- https://rdanderson.com/stargate/lexicon/entries/hyperspace.htm was read for
  context only. It is not evidence of a specific green appearance or production
  shader design. Fan-made mod/test videos surfaced in search were not accepted
  as original-series visual targets. Some web fetches failed; browser inspection
  supplied the actual image comparison. No claim of watching online video.

## What was wrong

0028 used two thresholded fractal-noise contours plus repeated angular spokes.
This makes a dense lightning/web pattern and a flat overexposed disc. Higher
resolution cannot fix those shape and motion choices. The large thin tendril
network also competes with the aperture silhouette and reads as magic.

## Revision direction

Replace the web with a limited set of finite, uneven tearing lips and translucent
sheets, varying their length, width, bending, local detail and opening time.
Start with a narrow split, spread the lips and widen the luminous interior,
then contract after the hull clears. Keep dark gaps between longer fissures.
White/mint light at the lips, green in the sheets, restrained spill at their
edges. Fine edges use pixel derivatives for stable native rendering.

Maintain the existing ship/planet assets and normal quality defaults. No global
blur, temporal upscaling or enlarged image texture. The material is an artistic
reconstruction of a fictional event, not a physically verified spacetime model.
User likeness review remains required; native 4K is necessary but insufficient.
Transit redesign is deferred. The existing +X-forward ship crossing is preserved.

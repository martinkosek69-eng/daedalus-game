# Solar detail sources and display contract

Task 0019 source assets only. This folder does not change the canonical game
catalog. Rebuild with `python Tools/Prepare-SolarDetails.py --offline`; Pillow
and numpy are required. The recipe writes only this folder, reads the assigned
SolarCatalog ring records and existing INOVE Saturn source, and uses no GUI app.

## Real partial surface maps

Credit both source images to **NASA/Johns Hopkins University Applied Physics
Laboratory/Southwest Research Institute**. The image-specific captions contain
no separate restrictive copyright notice. Reuse follows the
[JPL Image Use Policy](https://www.jpl.nasa.gov/jpl-image-use-policy/), preserving
the stated credits and without endorsement or logos. Do not describe these as
blanket Creative Commons imagery.

- [PIA20658, Pluto: A Global Perspective](https://www.jpl.nasa.gov/images/pia20658-pluto-a-global-perspective/),
  2016-05-02. `Sources/PIA20658.jpg` is the unchanged public JPEG; its native
  **2400x1200** pixels are resampled to `Textures/pluto_lorri_4k.jpg`,
  **4096x2048**. This upscale adds no observed detail. The mosaic is panchromatic,
  with much lower source resolution on the hemisphere away from the encounter.
- [PIA19866, Global Map of Pluto's Moon Charon](https://www.jpl.nasa.gov/images/pia19866-global-map-of-plutos-moon-charon/),
  2015-07-30. `Sources/PIA19866.jpg` is the unchanged **9520x4760** public JPEG;
  `Textures/charon_lorri_4k.jpg` is downsampled to **4096x2048**. The primary
  caption specifies simple cylindrical projection and zero longitude at centre.

The actual downloaded CDN URLs and source SHA-256 values are recorded in
`surface-views.json`. Both maps have north at top. Pluto's map centre is the
encounter hemisphere, not a verified zero-longitude reference. Runtime spin phase
and UV conventions need separate integration; no observed phase is invented.

Black unmapped southern areas remain black. `*_observation_mask.png` is a binary
visual indication from source luminance greater than 2, nearest-neighbour sampled;
it is not a calibrated science confidence map or measured percentage of spherical
surface area. Source-pixel coverage is approximately 67.74% Pluto and 64.69% Charon.
For an unknown-region display overlay, sample the mask separately; do not interpret
black as a measured dark terrain feature. No inpainting, fake relief or colorization.

## Explicit schematic placeholders

`eris_schematic_albedo.jpg`, `makemake_schematic_albedo.jpg`,
`haumea_schematic_albedo.jpg` and `ceres_schematic_albedo.jpg` are original,
project-owned **2048x1024** procedural albedo placeholders. Every entry is marked
`explicit-fictional-unmapped-schematic`, `observedTerrain:false`. Their modest
smooth variation is authored, not a photographed crater map or measured terrain.
The Ceres placeholder does not imply Dawn data is unavailable; this task imports
no Ceres mission mosaic. Prefer a reviewed real Ceres source if another task supplies
one. There is no fabricated photographic texture for unobserved dwarf worlds.

## Radial ring views

`ring-views.json` contains `views[]`, each with `parentId`, planet-centred
`innerMetres`, `outerMetres`, `texture`, dimensions and all assigned component
records. Image paths are relative to this folder. Runtime radial sampling:

`U = (planetCentredRadius - innerMetres) / (outerMetres - innerMetres)`; `V=0.5`.

Use clamp addressing; **RGB is sRGB, alpha is linear, straight/unpremultiplied**.
All 32 rows are identical. The four textures are **8192x32 RGBA**. Plane orientation
is parent equatorial and requires the parent pole. These views are axisymmetric:
they do not reconstruct eccentric azimuthal edges, Neptune arcs or vertical halos.

| Planet | Full radial extent, metres | Rendered / source records |
| --- | --- | --- |
| Jupiter | 89400000 - 280000000 | 5 / 5 |
| Saturn | 66900000 - 480000000 | 8 / 8 |
| Uranus | 41836250 - 106200000 | 12 / 13 |
| Neptune | 40900000 - 62937500 | 5 / 5 |

The Zeta record is preserved but skipped because assigned numeric geometry is
unknown. Broad ring boundaries are preserved. Width ranges choose their arithmetic
mean and explicitly label this representative circular display. Saturn F uses
an authored 100 km width below its published upper limit, starting from the source
inner-edge reference. Exact per-pixel interval coverage keeps narrow, subpixel
bands from being widened into false measured structures. These bands may be faint
at a distance and need correct runtime sampling/LOD.

Source geometry and optical depth remain in each component's `sourceRecord`.
Authored alpha values improve game readability; they are not a calibrated
conversion from optical depth into brightness. Jupiter and Neptune use faint dust,
Uranus uses dark narrow rings and faint outer rings, Saturn is brighter ice with
source albedo gaps. There is no opaque distant dust cloud.

Source factsheets: [Jupiter](https://nssdc.gsfc.nasa.gov/planetary/factsheet/jupringfact.html),
[Saturn](https://nssdc.gsfc.nasa.gov/planetary/factsheet/satringfact.html),
[Uranus](https://nssdc.gsfc.nasa.gov/planetary/factsheet/uranringfact.html).
Dedicated [Neptune ring facts](https://nssdc.gsfc.nasa.gov/planetary/factsheet/nepringfact.html)
provide widths omitted or different in the
[NASA Science summary](https://science.nasa.gov/neptune/neptune-facts/).
The former gives Galle approximately 2000 km wide, Leverrier/Arago under 100 km,
Lassell approximately 4000 km and Adams approximately 15 km. The view selects these
historical widths, using an authored 50 km representative for upper-limit cases;
assigned summary reference radii remain unchanged. Both sources and the disagreement
are retained. This is a mixed-source approximate circular view, not a new authoritative
ring ephemeris or claimed exact edge reconstruction.

Saturn's existing albedo source is by **Solar System Scope / INOVE** under
[CC BY 4.0](https://creativecommons.org/licenses/by/4.0/), from the
[provider catalog](https://www.solarsystemscope.com/textures/). The recipe samples
the unchanged source from its original **74.5 - 140.22 million metre** radial span
into the larger atlas. RGB and alpha adaptation are expressly attributed;
`ring-views.json` records its original path, hash and radial mapping. It is an
artist's visualization, not raw measured albedo/optical-depth data.

## Verification

Offline recipe repeat produced identical hashes for all textures and two view
JSONs. Image decode, real-source 2:1 aspect, source hashes, output dimensions,
RGBA mode, row identity, nonopaque authored alpha, actual gaps, component counts
and exact integrated narrow-width coverage passed. Original mosaics and the
full-range atlas overview were visually inspected. `rings_preview.png` is a
diagnostic strip overview, not a physical illumination rendering. No Unreal scene,
lighting, GPU, streaming or collision test is claimed; root owns those checks.

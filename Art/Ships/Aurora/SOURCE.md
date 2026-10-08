# Aurora-class warship: source and modifications

Game integration in task0026 now includes Aurora.glb, GAME_ASSET.json and baked
PBR maps through Tools/Prepare-Aurora.py. The original saved scene below remains
unchanged; its no-UV limitation describes that source, not the prepared game
export. See Docs/AURORA.md for cleanup, working scale, material/flight choices
and verification. The source has no separate animated hangar/engine modules.

This is a Blender look-development scene of the Ancient Aurora-class warship
from Stargate Atlantis. The user requested it as a library asset for later use.
It is not a numbered task and not a game-ready asset.

## Files

| File | Content |
| --- | --- |
| `Aurora_Class.blend` | `SM_Aurora` mesh, materials, lighting rig and cameras. Textures are referenced relatively. |
| `Textures/T_Aurora_PlateID.png` | 4096 px tile. R and G are per-plate random values, B is the panel and greeble detail. |
| `Textures/T_Aurora_Grime.png` | 4096 px tile. R is broad grime, G is patchiness, B is the small-window mask. |
| `Textures/T_Aurora_SGA_Normal.png` | 4096 px tile with the plate relief. |
| `preview.png` | Render of this scene through `CAM_SGA_Main`. |

## Source geometry

The geometry comes from "Aurora Class Battleship from Stargate" by **Martin**
on Printables:

- Model: <https://www.printables.com/model/66477-aurora-class-battleship-from-stargate>
- File: `xxx_-_aurora_whole.stl`, a public download made on 2026-10-04.
- Licence: Creative Commons Attribution, as listed on Printables.
- Attribution required: "Aurora Class Battleship from Stargate" by Martin
  (Printables), CC BY.

The Aurora design itself belongs to the Stargate franchise (MGM). This is a fan
asset for the project. Any public or commercial release needs its own
clearance.

## Modifications made here

**Mesh cleanup**
- The STL triangle soup is welded.
- Orientation is bow +X and up +Z.
- The ship is scaled to 3500 m long, the figure given on the Stargate wiki.
  The size is 3500 × 1252 × 690 m.
- Sharp edges are set by a 32° angle.

**Faceting**
- Large rounded patches (the bow, the side lobes) are reduced with a collapse
  decimation that is symmetric in Y.
- They get chamfer normals snapped to 26 directions. The face attribute `facet`
  marks these faces.
- The top dome stays round. The face attribute `dome` marks it.

**Panel grid**
- Faces larger than 450 m² are cut into a panel grid: X every 36 m, Y and Z
  every 30 m.

**Paint**
- The face attribute `paint` holds 1 for copper paint and 0 for dark steel.
- The layout was transferred from a Stargate Atlantis promotional still that
  the user supplied:
  - the camera was aligned to the still's silhouette (IoU 0.81);
  - a hue-based copper mask was made from the still;
  - each face was tested for visibility, the hidden side was mirrored, and
    each modelled panel takes one decision.
- **The still and the derived mask are not published** (third-party
  copyright). Only the resulting per-face values are in the file.

**Materials**
- `M_Aurora_Promo`:
  - dark slate steel with copper paint taken from `paint`;
  - light trims, grime and wear;
  - textures box-projected in object space with a 900 m tile.
- `M_Aurora_WindowGlass`: the 8 wing windows, which are glass panes in the
  model geometry.
- `M_Aurora_DomeGlow`: a small blue orb at the dome.

**Lighting and cameras**
- The lighting rig has two keys, a fill, a rim and a bounce.
- AgX view transform with exposure -0.35.
- `CAM_SGA_Main` matches the promo framing.
- `CAM_3Q_Front`, `CAM_3Q_Rear`, `CAM_Side`, `CAM_Top` and `CAM_Bow_Close` are
  general views.
- `CAM_Ref*` are older reference views.

## Statistics

- 640,092 triangles and 333,060 vertices.
- Three materials, 14 lights, nine cameras.

## Known limitations

- **No UV unwrap.** The textures are box-projected, so Unreal use needs UVs,
  baking and LODs first.
- **The paint follows a different model.** The still shows another author's
  model, so the copper parts are close to the still but not identical. For
  example, the long copper strips along the dorsal ridge and the band under the
  bow's top edge transferred only partly.
- **Viewing only.** Open the file in Blender 5.2 or later and use rendered
  viewport shading (EEVEE).

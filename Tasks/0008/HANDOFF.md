# 0008 handoff

State: READY_FOR_REVIEW. This is not ACCEPTED; the coordinator reviews
dimensions, import and rendering in its own Unreal copy.

- Worker: Claude Code. Branch `task/0008-daedalus-engine-effects`.
- Base: `98c4bd137703e675a51af36666d2e35bae1e724a` (revised brief).
- Checkpoints:
  - `06312a0` (first checkpoint),
  - final READY_FOR_REVIEW commit (this one; the SHA is in the push report).

## User decisions during the task

1. "Do not change the model proportions; only colours and lighting, plus
   effects around the hull, not the model itself."
   - No hull vertex is moved, and no turret or barrel geometry is added.
   - Mounts are measured points only.
2. The user saw dark squares in the Blender window and said that they are not
   part of the original model.
   - The baseline's 14 dark hatch squares and 36 vent slats were removed. A
     comparison render against `Original/daedalus.glb` confirmed that neither
     exists in the original.
   - The baseline's 30 window boxes keep their positions as a separate light
     export.
   - The hull is therefore exactly the Astrofossil geometry: 222330 triangles,
     against 223458 in the baseline.
3. After the first READY_FOR_REVIEW push (`991a823`), the user supplied more
   references (REFERENCES.md U9–U12) and asked for two more changes:
   - **Bow silos in the orange of U11** ("same shade, natural intensity").
     The 16 raised hatch plates are painted `#c98f35` in COLOR_0, as plain
     paint with no emission. The shade was matched by eye, because the image was
     only shown in chat.
   - **Asgard beam weapons:**
     - 2 at the bow and 2 under the hull between the hangars, mirrored. This
       comes from the user's stills U9/U10.
     - The bow position is labelled "Asgard beam turret" on the SGA Tech Journal
       schematic U12.
     - The user said to ignore the red forum circles on that image.
   - The user also described the weapon effects: orange railgun projectiles,
     missiles from the dorsal bow silos, and blue beams. These are recorded as
     `fireEffectHint`.

## Deliverables (Art/Ships/Daedalus)

| File | Role |
| --- | --- |
| `Daedalus.blend` | Source scene. Contains:<br>• `SM_Daedalus` (hull, applied transforms, metres)<br>• collection `EngineGlow` (6 objects)<br>• `HullLights` (`Daedalus_WindowLights`)<br>• `WeaponMounts` (60 `MOUNT_<id>` empties) |
| `Daedalus.glb` | One hull mesh `SM_Daedalus` with 222330 triangles, COLOR_0 and slots `Daedalus_Armor`, `Daedalus_EngineMetal`, `Daedalus_HangarInterior`, `Daedalus_LightWhite`. No textures, no UVs needed. |
| `DaedalusEngineGlow.glb` | Six `<outletID>_Glow` objects. Each has an emissive core disc inside the nozzle plus a short translucent haze (length 0.35 × aperture radius). Origin = outlet centre on the lip plane; exhaust = local −X; extras `outlet_id`, `exhaust_axis`, `throttle_hint`. |
| `DaedalusLights.glb` | `Daedalus_WindowLights`: the 30 window boxes, cool white emissive (`Daedalus_Glass`, strength 4). |
| `ENGINE_MOUNTS.json` | 6 outlets with centre, outward axis, aperture, lip outer diameter and recess depth. |
| `WEAPON_MOUNTS.json` | 60 mounts with axis and azimuth conventions, notes, group counts, `fireEffectHint` and unplaced systems. |
| `asset-metadata.json` | Hashes, counts, checks, removed fittings, export list. |
| `REFERENCES.md` | Source URLs, observations with evidence class, mapping to the model. |
| `SOURCE.md` | Astrofossil CC BY-NC 4.0 credit retained, plus the modification notice. |

All three GLBs share the hull frame: ship-local, +X forward, +Z up, metres,
origin at the hull bounding-box centre. Import them at the same transform with
no offsets.

Scripts:

- `Tools/Prepare-DaedalusDetail.py` runs the whole pipeline.
- `Tools/Prepare-DaedalusEngineEffects.py` is the outlet measurement and glow
  library. It can run alone to rebuild only the glow.

Reproduce:

```
blender --background --factory-startup --python Tools/Prepare-DaedalusDetail.py
```

Running it again gives byte-identical GLBs. Do not run
`Tools/Prepare-DaedalusSource.py`: it is the baseline only and overwrites the
same files.

## Unreal import notes (for Codex)

- The hull material is unchanged in principle: COLOR_0 × base factor. The hull
  slot base factor is white. The other slots carry their own base colour, and
  their COLOR_0 is 1.
- Emissive strengths are Blender values and may need tuning:
  - window lights 4,
  - bow strip 6,
  - hangar 0.45,
  - glow core 6,
  - plume 0.9 at alpha 0.16, blended.
- Glow throttle: scale the glow object's local X for plume length and scale
  emission for brightness.

## Mounts (WEAPON_MOUNTS.json)

Conventions:

- `forwardAxis` = mount local +X: barrel rest direction, silo launch direction
  or bay exit.
- `upAxis` = local +Z: the turret yaw axis.
- `traverseFreeAzimuthDeg` is measured about ship +Z: 0 = bow, 90 = port. It is
  sampled every 5° at 5° elevation, and only superstructure occlusion is
  checked, not game rules.

| Group | Count | IDs | Basis | Confidence |
| --- | --- | --- | --- | --- |
| dorsal_railguns | 26 | `RG_D_xx_P/S` | Modelled dome turrets (≈3.5 m diameter, ≈3.7 m tall). Centre = dome base. x −283…225 | medium: dome present, canon count unconfirmed (wiki 32 railguns, fan sheet 26) |
| ventral_railguns | 12 | `RG_V_xx_P/S` | Ventral domes, upAxis [0,0,−1] | medium |
| bow_vls | 16 | `VLS_01…16` | Bow dorsal hatch doors at x 116…222, y −10.75 / +12.75, z 16.63. Launch +Z. `hatchPlateBoundsXY` = the orange plate | high: 16 matches the wiki VLS count |
| f302_bays | 2 | `BAY_01_P/S` | Pod front openings, rim x 23.0, opening 83 × 20.5 m, depth 134.5 m. Exit +X | high: measured |
| asgard_beams | 4 | `ASG_01…04_P/S` | Bow: the outer dome on each lower side ledge (x ≈ 223, y ±44, base z −18.6). This is the "Asgard beam turret" of U12; the inner dome there stays a railgun. Ventral: chamfered front of the block between the hangar pods (x ≈ −20, y ±79.5), with `surfaceNormal` pointing forward and down | bow medium, ventral low-medium (no dedicated emitter modelled) |

Mount IDs were renumbered after `991a823`: two former `RG_D` domes are now
`ASG`. Every mount has a `fireEffectHint`.

The only remaining `unplacedReferenceSystems` entry is the two bow rods, which
are not modelled.

Turret moving meshes do not exist yet. `intendedMovingMesh` proposes
`SM_Daedalus_RailgunTurret` as a separate later asset. Nothing here implies that
guns work in Unreal.

## Engines (ENGINE_MOUNTS.json)

| Outlets | Centre (m) | Aperture | Lip outer | Recess |
| --- | --- | --- | --- | --- |
| `ENG_Main_Port` / `ENG_Main_Starboard` | x −300.0, y ±53.6, z −24.06 | 17.5 m | 21.5 m | 6.66 m |
| `ENG_Pod_<side>_Inner` / `ENG_Pod_<side>_Outer` | x −277.2, y ±127.0 / ±154.1, z −27.88 | 13 m | 15 m | 5.22 m |

Measured by ray-cast depth maps from behind, then a 24-ray radial profile per
outlet. Topology is unusable, because the STL-derived mesh is triangle soup.

## Checks run (Blender 5.2.2, background)

- The pipeline prints `DAEDALUS_DETAIL_PASS`.
- Dimensions are 600.0001 × 378.5321 × 92.6071 m. Units are metric, scale 1.
- Hull triangles are exactly 222330, asserted before export and after GLB
  re-import.
- The `.blend` is saved, then reopened:
  - one hull mesh,
  - 4 slots,
  - COLOR_0 present,
  - 0 image dependencies (no textures).
- GLB re-import:
  - hull: 1 mesh, 222330 triangles, 600 × 378.532 × 92.607, 4 materials,
    colour attribute present,
  - glow: 6 objects and 2 materials,
  - lights: 1 object.
- Mount axes, checked by reopening the `.blend` and comparing it with
  `WEAPON_MOUNTS.json`:
  - all 60 empties are present, with no extras,
  - worst deviation of position or axis is 1e-7,
  - all frames are right-handed (determinant 1).
- Glow origins sit within 0.6 mm of `ENGINE_MOUNTS.json`; the JSON is rounded
  to 3 decimals.
- Rendered views:
  - front, rear, side, top and 2 perspective views with a neutral grey world
    and identical exposure,
  - 4 "space" views (dorsal, rear, engines, hangar),
  - a comparison of the original and current pod and neck.

  These are in the ignored `.local/daedalus-detail/renders` and are not
  published. They were inspected visually:
  - no dark squares or slats,
  - windows lit,
  - warm light only inside the hangar openings,
  - small engine glow with no oversized flame.

  Close views of the orange silo plates and of the 4 Asgard mounts (blue
  markers with a direction line) are in `.local/daedalus-detail/asg`. They are
  also ignored and not published.

## Limitations

- Colours are an artistic approximation of on-screen stills, not sampled
  production colour. The first, darker attempt read too green and was
  lightened. Final judgement belongs in Unreal lighting.
- Some reference-visible details are not modelled or placed, because geometry
  changes were excluded:
  - lilac and red navigation point lights,
  - the two long forward bow rods,
  - Asgard emitters,
  - separate turret and barrel meshes.
- The hangar interior material covers only the single-layer skin faces seen
  through the openings (156 faces). There is no extra interior geometry.
- Turret free arcs ignore elevation limits and other ships.
- User reference images (U1–U8) are only described in REFERENCES.md and are not
  redistributed. Two wiki stills named in captions could not be retrieved
  (archive 404).
- The live `daedalus_apps` MCP was not used. Blender ran in background
  processes with saved files.
- At the user's request, a separate GUI Blender window showed a preview copy
  (`.local/daedalus-detail/live.blend`, ignored). It was never the source.
- Unreal was not started or edited.

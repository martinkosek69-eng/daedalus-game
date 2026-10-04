# 0008 handoff

State: READY_FOR_REVIEW. This is not ACCEPTED; the coordinator reviews
dimensions, import and rendering in its own Unreal copy.

- Worker: Claude Code. Branch `task/0008-daedalus-engine-effects`.
- Base: `98c4bd137703e675a51af36666d2e35bae1e724a` (revised brief).
- Checkpoints:
  - `06312a0`,
  - `991a823`,
  - `d5846a1`,
  - `3effaca`,
  - final pass (this commit; the SHA is in the push report).

## User decisions during the task

1. **Proportions.** "Do not change the model proportions; only colours and
   lighting, plus effects around the hull." The hull vertices are still exactly
   the Astrofossil geometry: 222330 triangles, 600 m.
2. **Removed fittings.** The baseline's dark hatch squares and vent slats are
   not in the original, so they were removed. The windows became a separate
   light export.
3. **Asgard beams and effects (U9–U12).**
   - 4 Asgard beam weapons: 2 at the bow, where the U12 schematic labels them,
     and 2 under the hull between the hangars.
   - Weapon effect hints: orange railgun projectiles, missiles from the bow
     silos, blue beams.
4. **First detailed look rejected.** It was too light, cool and flat, and the
   orange was too strong. The user gave free rein to match the series. That led
   to the procedural plating texture, a palette sampled from U13 and series-like
   look-dev (commit `3effaca`).
5. **Final pass ("almost there; finalize it").** The user gave the 4K still
   U15 as the main target, plus close-ups:
   - **VLS hatches exactly like U16:** long taupe doors in dark frames, grey end
     tabs with bolts, yellow/black hazard stripes on alternate hatches
     (checkerboard). They are overlays on the measured hatch plates.
   - **Hangars like U17:** a new back wall in each bay with the split
     observation window (dark green glass, light frame bars) in front of the
     model's older X-truss frame. Spotlights above each opening, floodlights
     under the pod edges, nav lights.
   - **Small, faint blue windows (U18–U20):**
     - a row in the now-dark bow band,
     - rows on the bridge tiers,
     - the 30 baseline windows dimmed.
   - **Blinking mast-tip lights (U20):**
     - re-placed masts: two on the bridge top tier and three tall ones beside
       the tower,
     - three orange beacons that blink once every 5 s.
   - **Engines (U21):** the internal turbine vanes must stay visible, not a
     flat orange disc. The vane and hub faces are warm-lit, the core disc sits
     behind them, and a point light in front of the vanes lights them.
   - **Overall tone and light from U22** (only tone, not details): a cooler
     Earth/sky bounce, so the hull reads blue-grey in shadow as in daylight.
   - **The twin barrels added on the turrets were removed.** The original dome
     turrets already carry their own guns (U23–U25), so having two made no
     sense.

## Deliverables (Art/Ships/Daedalus)

| File | Role |
| --- | --- |
| `Daedalus.blend` | Source scene. Contains:<br>• `SM_Daedalus` (hull)<br>• `EngineGlow` (6 glow objects + 6 point lights)<br>• `HullLights` (`Daedalus_WindowLights`, `Daedalus_DetailLights`)<br>• `AddOns` (`Daedalus_AddOns`)<br>• `Beacons` (`Daedalus_Beacons`, preview animation)<br>• `WeaponMounts` (62 empties)<br>• `LookDev_Series` (3 suns, `CAM_SeriesStill`)<br>Textures are referenced as `//Textures/*.png`. |
| `Daedalus.glb` | One hull mesh `SM_Daedalus`: exactly 222330 triangles, 600 m, with `UVMap` and COLOR_0. Slots:<br>• `Daedalus_Armor` (plating textures)<br>• `Daedalus_EngineMetal`<br>• `Daedalus_HangarInterior` (plating textures)<br>• `Daedalus_LightWhite` (dark bow window band)<br>• `Daedalus_EngineInner` (turbine vane and hub faces, warm glow)<br>Textures are embedded. |
| `Textures/T_Daedalus_Plating_{BaseColor,ORM,Normal}.png` | 2048², tiling, 1 tile = 32 m; ORM G = roughness, B = metallic; normal map in OpenGL (+Y) convention. |
| `Textures/T_Daedalus_SiloHatch_{Plain,Striped}_BaseColor.png`, `..._Normal.png` | 2048 × 822, one hatch (15.45 × 6.2 m) per image. Final albedo (COLOR_0 = 1 on those faces). |
| `DaedalusEngineGlow.glb` | Six `<outletID>_Glow` objects, each with:<br>• a core disc behind the vanes<br>• a faint haze<br>• a child point light (KHR_lights_punctual, range 1.3 × aperture radius)<br>Origin = outlet centre, exhaust = local −X. |
| `DaedalusLights.glb` | `Daedalus_WindowLights` (30 faint windows) and `Daedalus_DetailLights`:<br>• faint cyan windows in the bow band and on the bridge tiers<br>• pod spotlights and floodlights<br>• nav lights (starboard green, port red) |
| `DaedalusAddOns.glb` | `Daedalus_AddOns`. Materials: Armor, Hangar, `Daedalus_SiloHatch_Plain/Striped`, `Daedalus_HangarWindow`, `Daedalus_TrimLight`. Contains:<br>• bridge masts<br>• two 40 m forward rods (estimated)<br>• 16 VLS hatch overlays (U16)<br>• two hangar back walls with windows (U17) |
| `DaedalusBeacons.glb` | `Daedalus_Beacons`: three orange lamps on mast tips. Object extras: `blinkPeriodSeconds` 5, `onSeconds` 1/3, `emissionOn` 8, `emissionOff` 0. |
| `ENGINE_MOUNTS.json` | 6 outlets (unchanged). |
| `WEAPON_MOUNTS.json` | 62 mounts (see below). |
| `asset-metadata.json` | Hashes for all exports and textures, counts, checks, add-on, light and beacon info. |
| `REFERENCES.md` | Sources (R1–R3, U1–U25, F1), U13/U22 colour samples, observations, mapping. |
| `SOURCE.md` | Astrofossil CC BY-NC 4.0 credit retained, plus the modification notice. |

`DaedalusTurrets.glb` from `3effaca` is deleted (user decision 5).

All GLBs share the hull frame: ship-local, +X forward, +Z up, metres, origin at
the hull bounding-box centre. Import them at the same transform with no
offsets.

Reproduce (about 10 minutes; the AO bake is the slow part):

```
blender --background --factory-startup --python Tools/Prepare-DaedalusDetail.py
```

`Tools/Prepare-DaedalusEngineEffects.py` is the outlet and glow library. Do not
run `Tools/Prepare-DaedalusSource.py`: it is the baseline only and overwrites
the same files.

## Unreal import notes (for Codex)

- **Hull and add-on materials need textures** (TEXCOORD_0).

  | Material input | Value |
  | --- | --- |
  | BaseColor | `BaseColorTexture × COLOR_0` (the glTF rule; factor white) |
  | Roughness | ORM.G |
  | Metallic | ORM.B |
  | Normal | Normal map (OpenGL green; flip G for DirectX) |

  The VLS hatch materials have base colour and normal only (roughness 0.6,
  metallic 0.3). Untextured slots use their factors.
- **Beacons blink in the engine.** Use emission 8 for 1/3 s, then 0, repeating
  every 5 s. Values come from the object extras.
- **Engine glow.** Scale the glow object's local X for plume length; scale
  emission and point-light power for throttle.
- **Lighting reference.** `LookDev_Series` in the .blend matches U13/U15 with
  the U22 tone.

  | Light | Strength | Colour | Travel direction |
  | --- | --- | --- | --- |
  | Key sun | 4.6 | (1, 0.98, 0.95) | (0.3, −0.5, −0.81) |
  | Cool Earth/sky bounce | 2.4 | (0.62, 0.8, 1) | (−0.25, 0.6, 0.6) |
  | Camera fill | 0.6 | — | — |

  The world is dark blue-black, with AgX Medium High Contrast. A hard sun alone
  makes the sides pitch black, which does not match the series.
- **Emissive strengths (Blender values):**

  | Element | Strength |
  | --- | --- |
  | Baseline windows | 0.4 |
  | Detail windows | 0.32 |
  | Spotlights, floodlights and nav lights | 8 |
  | Engine core | 8 |
  | Turbine vanes | 0.6 |
  | Hangar window | 0.08 |

## Mounts (WEAPON_MOUNTS.json)

Conventions:

- `forwardAxis` = mount local +X: barrel rest, silo launch or bay exit.
- `upAxis` = local +Z: the yaw axis.
- `traverseFreeAzimuthDeg` is measured about ship +Z: 0 = bow, 90 = port. A
  negative start wraps.

| Group | Count | IDs | Basis | Confidence |
| --- | --- | --- | --- | --- |
| dorsal_railguns | 26 | `RG_D_xx` | The model's own dome turrets with barrels (part of the hull) | medium |
| ventral_railguns | 12 | `RG_V_xx` | Ventral domes, upAxis [0,0,−1] | medium |
| bow_vls | 16 | `VLS_01…16` | Bow hatches, launch +Z, `hatchPlateBoundsXY` | high (16 = wiki) |
| f302_bays | 2 | `BAY_01_P/S` | Pod front openings, exit +X | high |
| asgard_beams | 4 | `ASG_01…04` | Bow: outer ledge dome (U12 label). Ventral: chamfered block front between the hangar pods | bow medium, ventral low-medium |
| bow_rods | 2 | `ROD_01_P/S` | Tips of the forward rods (DaedalusAddOns.glb), +X | low |

Every mount has a `fireEffectHint`. Nothing here implies that weapons work in
Unreal. A rotating railgun turret would need its dome extracted from the hull
as a separate mesh.

## Checks run (Blender 5.2.2, background)

See `asset-metadata.json` → `checks`. The pipeline prints
`DAEDALUS_DETAIL_PASS` only if all asserts pass:

- **Hull geometry.** Exactly 222330 triangles before export and after GLB
  re-import; 600.0001 × 378.5321 × 92.6071 m.
- **Reopened `.blend`:**
  - one hull mesh,
  - 5 slots,
  - COLOR_0 and UVMap present,
  - all texture files resolve,
  - every mount empty matches the JSON frame (re-verified separately: 62
    mounts, no deviations).
- **GLB re-import:**
  - hull: UV map and embedded images present,
  - add-ons: hatch and hangar-window materials present,
  - beacons: blink extras present,
  - glow: 6 objects and 6 lights,
  - lights objects present.
- **Rendered checks.** Renders from `CAM_SeriesStill` (compared against U13/U15
  and U22) plus close views of the silos, hangar, engines, bridge and bow. They
  are in the ignored `.local/daedalus-detail/final` and are not published.

## Limitations

- The look is an approximation of the on-screen ship. The plating and hatch
  textures are procedural, not the production textures. Canon colours are
  unknown, and final judgement belongs in Unreal under the game's lighting.
- The texture repeats every 32 m. COLOR_0 patches break up the repetition.
- Some items are estimated from stills and are separate objects, so they are
  easy to adjust or drop:
  - mast and rod positions,
  - the hangar back-wall depth,
  - the beacon count.
- The hatch overlays are 0.55 m above the model's hatch plates; they cover the
  older door design.
- The ventral Asgard emitters and the rods have no dedicated modelled emitter.
  The small lilac pod lights are not placed.
- User reference images are described in REFERENCES.md and are not
  redistributed.
- Blender ran in background processes with saved files. A separate GUI window
  showed a preview copy (ignored). Unreal was not started or edited.

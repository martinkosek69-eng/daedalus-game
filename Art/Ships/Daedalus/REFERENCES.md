# Daedalus reference study (task 0008)

Purpose: correct colour, lighting and visible detail zones of the existing
Astrofossil-based 600 m Daedalus without changing its shape (user instruction:
keep proportions, change only colours/lighting and add effects around the hull).
No reference images are stored in this public repository.

## Sources

| ID | Source | Type | Weight |
| --- | --- | --- | --- |
| R1 | Stargate wiki, "BC-304", archived 2024-01-12: https://web.archive.org/web/20240112040915/https://stargate.fandom.com/wiki/BC-304 | fan wiki text summarising on-screen facts with episode citations | specifications only |
| R2 | Infobox still `Daedalus.jpg` from R1: https://static.wikia.nocookie.net/stargate/images/0/03/Daedalus.jpg (archived copy https://web.archive.org/web/2024im_/https://static.wikia.nocookie.net/stargate/images/0/03/Daedalus.jpg) | on-screen CG still, dorsal three-quarter view above Earth | primary visual |
| R3 | R1 gallery captions identifying stills: "Railguns firing" (`The_Hive.jpg`), "VLS missile tubes firing" (`Odyssey_1.jpg`), "Asgard plasma beams firing", "The George Hammond's engines" | wiki image captions | identified, images not retrievable (archive 404) |
| U1–U15 | Images supplied by the user in chat, original URLs unknown | mix (see below) | local study only, not redistributed |

User images (sources not stated, kept local):

- U1: dorsal-rear close view with engines, shield impact and ship explosion (on-screen CG)
- U2: classic front-quarter still (on-screen)
- U3: "FOR 3D PRINTING" fan model render (fan interpretation)
- U4: "Stargate Now" spec sheet of a model it calls screen-used (fan publication, its own disclaimer)
- U5: same image as R2
- U6: promotional/on-screen still with a Stargate and Wraith ships
- U7: Asgard beam firing at a Dart above Earth (on-screen)
- U8: dark still with a lit hangar bay (on-screen)
- U9: small on-screen still, ship seen from below and the side. Two blue
  beams leave it: one at the bow and one under the hull between the hangar pods.
- U10: on-screen still of two BC-304s firing blue-white beams
- U11: high-detail model render sheet ("INTERFACING" frame): a top view plus
  insets of the bridge, twin-barrel dome turrets, an engine and a hangar. It
  shows amber-orange markings on the bow VLS hatches. Origin is unknown, so it
  is treated as a fan or production render.
- U12: "SGC Intelligence Briefing — Daedalus Battleship Schematic" from the
  Stargate Atlantis "Tech Journal". It labels the bridge, main engine, Asgard
  hyperdrive, shield generator, missile tubes, forward sensor array, F-302 bay
  and **Asgard beam turret** (bow, lower side). The user's copy carries red
  forum circles about the bridge position; at the user's instruction they are
  ignored.

- U13: high-resolution on-screen still, a sunlit dorsal three-quarter view
  from front starboard above Earth. **This is the primary colour reference**
  (sampled, see below). It shows:
  - dense rectangular plating in a patchwork of lighter and darker greys,
  - faint ochre dashes at the bow VLS hatch edges,
  - bridge masts,
  - long thin rods ahead of the bow, one with a glowing tip,
  - small blue-white windows at the bow front.
- U14: small dark on-screen still, backlit, with very dark hull sides.
- U15: dramatic fan render. It shows cyan window lights and long bow barrels
  with glowing tips. It is used only for silhouettes.

The user rejected the first detailed result in Blender. The hull was too light,
cool and flat. The orange read as "gold bricks", but should be almost
invisible. The lighting did not match. The user then gave free rein to make the
ship look as close to the series as possible.

Sampled from U13 (sRGB 0–255; region means; p10–p90 shows the plate contrast):

| Region | Mean | p10–p90 |
| --- | --- | --- |
| lit rear deck | 145 / 142 / 139 | 93–189 |
| lit bow top | 164 / 160 / 157 | 119–196 |
| bow side, away from the sun | 109 / 107 / 104 | 64–167 |
| pod side | 110 / 110 / 108 | 64–162 |
| silo markings | 154 / 141 / 110, only 0.25 % of the silo area | n/a |

The hull is therefore a neutral grey with a very slight warm cast, not
olive-green or blue. It has strong plate-to-plate contrast. The orange is a
desaturated ochre, barely different from the grey.

For comparison, the user's Blender view of the first result measured: deck
177 / 182 / 184, bow side 154 / 162 / 162, with low contrast. That is too light,
too cool and too flat.

The user also described the weapon effects from the footage:

- Turrets fire orange projectiles.
- Missiles leave the dorsal bow silos.
- The Asgard weapons are 4 beams: 2 at the bow and 2 under the hull between
  the hangars. Each still shows one side firing; the other side mirrors it.

On-screen stills (R2, U1, U2, U5, U7–U10, U13, U14) take precedence; U13 is
the colour reference. Fan material (U3, U4, U11, U15) is used only for
silhouettes and never for counts.

## Observations

| Topic | Observation | Evidence class |
| --- | --- | --- |
| Hull palette | Neutral mid grey with a very slight warm cast (R ≥ G ≥ B by a few levels); lit decks sRGB 145–165, sides away from the sun about 105–110 | sampled (U13) |
| Plating | Dense rectangular plates (roughly 1.5–5 m) in rows. Groups of plates are noticeably lighter or darker (patchwork). There are dark seams and many small dark boxes | visible (U13, R2) |
| Bridge | Tower on the rear deck with lit window rows, a cross bar and tall thin masts | visible (U13, R2, U5) |
| Hangar bays | Side pods with front openings; dark plated interior with lights (warmer and brighter in U8) | visible (U13, U8) |
| Lights | Small blue-white windows (bow front, bridge); small lilac lights on pod sides; red points near the bow | visible (U13, U1, U8, U15) |
| Engines | Large circular sublight outlets at the rear of the hull and the pods; yellow-white core with orange halo; radial vanes inside | visible (U1, U7); vanes in U3/U4 |
| Railgun turrets | Small dome turrets with short twin barrels along the deck rim, bow edges and lower surfaces | domes visible (R2, U13); twin barrels in U11 inset |
| Bow missile silos | Two columns of 8 hatches on the dorsal bow; R1 states 16 VLS tubes. Hatches stay hull grey, with faint ochre dashes at their edges | count R1; zone R2, U12, U13; marking U13 |
| Asgard beam weapons | R1: 4 weapons. Blue-white beams leave the bow and the underside between the hangar pods, one side per still. U12 labels the "Asgard beam turret" on the lower side of the bow | visible (U9, U10), labelled (U12) |
| Weapon effects | Railgun turrets: orange tracer projectiles. VLS: missiles leave the bow silos upward. Asgard: continuous blue-white beam | user description of the footage; U9, U10 |
| Forward rods | Long thin rods ahead of the bow (one per side), one with a glowing tip | visible (R2, U2, U13); length and base are estimated |

## Mapping to the model (approximations)

- **Hull plating texture.** `Textures/T_Daedalus_Plating_*.png` (2048², one tile
  = 32 × 32 m) are generated procedurally by the detail script (seed 304); there
  are no third-party images. They contain:
  - plates in staggered rows with a light/dark patchwork, dark seams, inset
    hatches, vent rows and small dark boxes,
  - a matching normal map (seams, raised plates, boxes),
  - ORM (roughness G, metallic B).

  `UVMap` is a world-scale box projection per face. The same maps are used on
  the hangar interiors.
- **Hull colour.** COLOR_0 carries the large-scale tone, which the texture
  multiplies:
  - orientation palette: top `#9a9a98`, sides `#8c8c8a`, under `#7c7c7a`,
    neutral,
  - plate patches: ×0.70 or ×1.15 over 23 × 17 m cells,
  - darker greebles: ×0.80,
  - engine section: ×0.88,
  - baked AO (7 m).

  With the texture mean of about 0.78 sRGB, the lit deck lands near the U13
  values under the look-dev light. The olive tint of the first pass was removed.
- **Silo markings.** Only the outer raised frame ridge around each VLS door
  gets a faint ochre tint (COLOR_0 × (1.12, 0.98, 0.74)). The plates and doors
  stay hull grey, as in U13. An earlier, stronger tint still read as gold frames
  and was toned down.
- **Hangar interiors.** `Daedalus_HangarInterior` is dark plated grey with a
  weak warm emission (0.12).
- **Bow light strip and windows.** These are cool blue-white:
  - bow strip `#a9d6ff`, strength 2.5,
  - windows `#bfe3ff`, strength 3.0, in `DaedalusLights.glb`.

  The baseline dark hatch squares and vent slats were removed (not in the
  original model).
- **Asgard beam weapons.**
  - Bow pair: the outer dome on each lower side ledge, where U12 labels the
    turret.
  - Ventral pair: the chamfered block fronts between the hangar pods.
- **Add-ons (`DaedalusAddOns.glb`, separate from the 600 m hull).**
  - Two tall and two short bridge masts plus a cross arm.
  - Two forward rods: 40 m long, ahead of the bow front at y ±29.5 m,
    z −14 m. Their base and length are estimated from U13.
- **Turret barrels (`DaedalusTurrets.glb`).** Every railgun dome gets twin
  barrels (about 5.5 m) as a separate object with its pivot on the mount. Where
  forward would hit the hull, the rest direction turns into the free arc.
- **Look-dev (`LookDev_Series` in the .blend, not exported).** The scene
  matches U13:
  - key sun from port-aft above (4.6, near-white),
  - blue Earth bounce from below-starboard (2.0),
  - a weak camera-side fill (0.5),
  - near-black world,
  - AgX Medium High Contrast,
  - camera `CAM_SeriesStill`.
- **Engines.** Nozzle faces use darker metal. The glow is a separate effect
  (`DaedalusEngineGlow.glb`) sized from the measured outlets.
- **Not done.**
  - Lilac and red point lights are not placed.
  - Exact canon colours and production textures are unavailable; the
    texture is a procedural approximation.

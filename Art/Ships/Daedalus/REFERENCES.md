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
| U1–U33, F1 | Images supplied by the user in chat, original URLs unknown | mix (see below) | local study only, not redistributed |

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
- F1: dramatic fan render. It shows cyan window lights and long bow barrels
  with glowing tips. It is used only for silhouettes.
- U15: 4K version of the U13 shot. **This is the main reference for the final
  pass.** It shows:
  - bridge masts with orange glowing tips,
  - green nav lights on the starboard pod,
  - small blue windows,
  - a glowing rod tip.
- U16: VLS close-up. Each hatch is a long worn taupe door in a dark frame with
  a raised grey lip and a dark slot with tick marks. Light end tabs carry two
  bolts. In a checkerboard, every other hatch has yellow/black hazard stripes on
  both end tabs.
- U17: hangar close-up. Dark plated bay with large split doors on the back
  wall. The user clarified that they are doors, not windows: dark teal-grey
  halves that open apart at the middle seam, upward and downward. Small fixtures. Two
  round spotlights sit above the opening, light clusters under the front edge
  and small green lights at the side.
- U18: bow window band. A dark recessed strip with a row of small, faint cyan
  windows.
- U19: forward superstructure with faint cyan window panel and small cyan
  lights.
- U20: bridge close-up. Tiers with rows of small cyan windows, the cross bar
  and a dish. Two thin masts on the top tier have orange tip lights, beside a
  cluster of very tall thin masts. The user says the tip lights blink about once
  every 5 s.
- U21: engine still (battle). The nozzles show their internal turbine vanes
  around a bright warm-white core, not just an orange disc.
- U22: full ship above Earth in daylight. It is used **only for the overall
  tone and lighting** in a normal game environment, not for details. Sampled:
  lit decks about 145–165 sRGB with a cool cast (B > R by about 15), sides about
  95–115 with a blue-green cast.
- U23–U25: the user's Blender screenshots. They show that the added twin
  barrels duplicated the original turrets' own guns.
- U26–U27: the user's Blender screenshots of the hangar. The back wall
  protruded at the top-left, and the surroundings looked unfinished.
- U28–U29: hangar stills (series), with the window frame, wall structure and
  lights.
- U30–U31: forward superstructure in the user's Blender and in the series, with
  its window panel and two cyan units.
- U32–U33: the hangar doors in the user's Blender and in the series, used for
  the door tone.

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
the colour reference (U15 is its 4K version). Fan material (U3, U4, U11, F1) is used only for
silhouettes and never for counts.

## Observations

| Topic | Observation | Evidence class |
| --- | --- | --- |
| Hull palette | Neutral mid grey with a very slight warm cast (R ≥ G ≥ B by a few levels); lit decks sRGB 145–165, sides away from the sun about 105–110 | sampled (U13) |
| Plating | Dense rectangular plates (roughly 1.5–5 m) in rows. Groups of plates are noticeably lighter or darker (patchwork). There are dark seams and many small dark boxes | visible (U13, R2) |
| Bridge | Tower on the rear deck with lit window rows, a cross bar and tall thin masts | visible (U13, R2, U5) |
| Hangar bays | Side pods with front openings; dark plated interior with lights (warmer and brighter in U8) | visible (U13, U8) |
| Lights | Small faint cyan windows (bow band, bridge tiers); pod spot and flood lights; green nav lights on the starboard pod; orange blinking beacons on the bridge mast tips | visible (U15, U17–U20, U1, U8) |
| Engines | Large circular sublight outlets at the rear of the hull and the pods; bright warm-white core; the radial turbine vanes inside stay visible while the engines run | visible (U1, U7, U21) |
| Railgun turrets | Small dome turrets with their own short barrels along the deck rim, bow edges and lower surfaces; the model already has them | visible (R2, U13); model geometry |
| Bow missile silos | Two columns of 8 long hatches across the dorsal bow; R1 states 16 VLS tubes. Taupe doors in dark frames, grey end tabs; hazard stripes on alternate hatches | count R1; look U16; zone R2, U12, U13 |
| Asgard beam weapons | R1: 4 weapons. Blue-white beams leave the bow and the underside between the hangar pods, one side per still. U12 labels the "Asgard beam turret" on the lower side of the bow | visible (U9, U10), labelled (U12) |
| Weapon effects | Railgun turrets: orange tracer projectiles. VLS: missiles leave the bow silos upward. Asgard: continuous blue-white beam | user description of the footage; U9, U10 |
| Forward rods | Long thin rods ahead of the bow (one per side), one with a glowing tip | visible (R2, U2, U13, U15); length and base are estimated |
| Hangar bays | Dark plated bays; back wall with large split doors (halves open up and down from the middle seam), dark teal-grey (U33: about sRGB 40/46/46) | visible (U17, U28, U29, U33); door function from the user |

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
  - orientation palette: top `#8c8c8a`, sides `#7f7f7d`, under `#707070`,
    neutral (darkened about 9 % at the user's request),
  - plate patches: ×0.70 or ×1.15 over 23 × 17 m cells,
  - darker greebles: ×0.80,
  - engine section: ×0.88,
  - baked AO (7 m).

  With the texture mean of about 0.78 sRGB, the lit deck lands near the U13
  values under the look-dev light. The olive tint of the first pass was removed.
- **VLS hatches (U16).** `DaedalusAddOns.glb` places an overlay slab on each
  of the 16 measured hatch plates, about 0.55 m above the plate top. It covers
  the model's older door design. Each slab carries a 15.45 × 6.2 m hatch with
  its own textures (`T_Daedalus_SiloHatch_Plain/Striped_BaseColor`,
  `T_Daedalus_SiloHatch_Normal`, procedural, 2048 × 822):
  - a long worn taupe door in a dark frame with a raised grey lip,
  - a dark slot with tick marks,
  - grey end tabs with two bolts.

  Yellow/black hazard stripes cover the tabs on alternate hatches, in a
  checkerboard as in U16. The earlier faint ochre tint on the hull is gone.
- **Hangar interiors (U17, U26–U29).** `Daedalus_HangarInterior` is dark
  plated grey with no emission. Each bay gets a new back wall
  (`DaedalusAddOns.glb`):
  - It sits just in front of the model's older X-truss frame, about 46.5 m
    behind the rim.
  - It is clipped to the measured, asymmetric bay cross-section (72 rays), so it
    never pokes through the pod skin. An earlier rectangle did, and the user
    spotted it.

  The wall carries:
  - the split hangar doors: two plated metal halves, separate objects
    `HangarDoor_<P|S>_Upper/Lower`, meeting at a seam with the raised
    trapezoid. They are dark teal-grey (U33) and fitted between the walls; the
    object extras give `openAxis` and `openDistanceMetres`,
  - a heavy frame and the raised centre bars (`Daedalus_TrimLight`),
  - a ledge, ribs and fixtures.

  The bay also gets:
  - vertical ribs on its vertical walls,
  - ceiling beams,
  - two floor rails,
  - two dim teal point lights, so the interior reads as finished rather than
    bare skin.
- **Lights (`DaedalusLights.glb`).** All windows are small and faint cyan
  (`#57c6dd`): the 30 baseline windows at strength 0.4, plus
  `Daedalus_DetailLights`:
  - 7 windows in the dark bow band; the old white strip is now a dark recess,
  - rows of windows on the bridge tower tiers (0.32),
  - two round spotlights above each hangar opening,
  - three floodlights under each pod front edge,
  - nav lights: starboard green, port red.
- **Beacons (`DaedalusBeacons.glb`).** Three orange (`#ff6a2a`) lamps on the
  mast tips (two top-tier masts, the tallest mast). They blink once every 5 s
  for 1/3 s (object extras `blinkPeriodSeconds`, `onSeconds`, `emissionOn`).
  The .blend has a looping preview animation.
- **Engines (U21).** Inside each nozzle, the aft-facing faces of the turbine
  vanes and hub use `Daedalus_EngineInner` (warm-lit metal, yellow glow 0.6).
  The inner walls stay dark. In `DaedalusEngineGlow.glb`:
  - the warm-white core disc now sits behind the vanes,
  - a point light just in front of the vanes (range 1.3 × aperture radius)
    lights them,
  - the haze is shorter and fainter.

  The turbine therefore reads as structure, not as a flat orange disc.
- **Forward superstructure (U19, U31).** On the front face of its upper tier
  (x ≈ 66 m) there is a dark window panel with 5 faint cyan windows. Two small
  cyan-lit units stand on the deck in front of it.
- **Railgun turrets.** These are the model's own domes with their barrels. The
  twin barrels added in the previous pass duplicated those guns and were
  removed at the user's request (U23–U25).
- **Asgard beam weapons.**
  - Bow pair: the outer dome on each lower side ledge, where U12 labels the
    turret.
  - Ventral pair: the chamfered block fronts between the hangar pods.
- **Add-ons (`DaedalusAddOns.glb`, separate from the 600 m hull).**
  - Bridge masts (U15, U20): two thin masts on the top tier and three very tall
    thin masts beside the tower.
  - Two forward rods: 40 m long, ahead of the bow front at y ±29.5 m,
    z −14 m. Their base and length are estimated from U13.
- **Look-dev (`LookDev_Series` in the .blend, not exported).** The scene
  matches U13/U15, with the cooler daylight tone of U22:
  - key sun from port-aft above (4.6, near-white),
  - cool blue Earth/sky bounce from below-starboard (2.4),
  - a weak cool camera-side fill (0.6),
  - near-black world,
  - AgX Medium High Contrast,
  - camera `CAM_SeriesStill`.
- **Not done.**
  - The small lilac lights on the pod sides are not placed.
  - Exact canon colours and production textures are unavailable; the
    texture is a procedural approximation.

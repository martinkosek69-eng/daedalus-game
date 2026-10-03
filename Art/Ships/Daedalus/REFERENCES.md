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
| U1–U8 | Eight images supplied by the user in chat, original URLs unknown | mix (see below) | local study only, not redistributed |

User images (sources not stated, kept local):

- U1: dorsal-rear close view with engines, shield impact and ship explosion (on-screen CG)
- U2: classic front-quarter still (on-screen)
- U3: "FOR 3D PRINTING" fan model render (fan interpretation)
- U4: "Stargate Now" spec sheet of a model it calls screen-used (fan publication, its own disclaimer)
- U5: same image as R2
- U6: promotional/on-screen still with a Stargate and Wraith ships
- U7: Asgard beam firing at a Dart above Earth (on-screen)
- U8: dark still with a lit hangar bay (on-screen)

On-screen stills (R2, U1, U2, U5, U7, U8) take precedence. Fan model material
(U3, U4) is used only to confirm silhouettes of engines/turrets and never for
colour or counts.

## Observations

| Topic | Observation | Evidence class |
| --- | --- | --- |
| Hull palette | Dark charcoal grey with a slight olive/green cast on vertical and lower surfaces; sunlit upper deck reads medium grey | visible (R2, U2, U8) |
| Plating | Dense rectangular plating. The upper deck has a patchwork of noticeably darker and lighter plates; panel seams read lighter at edges in raking light | visible (R2, U8) |
| Deck greebles | Many small dark structures and boxes on the upper deck read darker than the deck plates | visible (R2, U5) |
| Bridge | Raised superstructure on the rear upper deck with lit window rows and tall thin masts | visible (R2, U5); the model's tower is kept as modelled |
| Hangar bays | Side pods with front openings; interior lit warm white with ribbed walls | visible (U8, R2) |
| Lights | Small white/cool lights along bow and hull edges; bright white strip at the bow front; small lilac lights on pod sides; red points near the bow | visible (R2, U1, U8). Lilac/red points are not mapped to geometry yet (see below) |
| Engines | Large circular sublight outlets at the rear of the hull and the pods; yellow-white core with orange halo; radial vanes inside | visible (U1, U7); vanes confirmed in U3/U4 |
| Railgun turrets | Small dome turrets with barrels along the dorsal deck rim, bow edges and lower surfaces | visible as domes (R2, U8); twin barrels in U4 (fan) |
| Bow missile silos | Grid of hatches on the dorsal bow; R1 states 16 VLS tubes | count from R1; zone visible in R2 |
| Asgard beam emitters | R1: 4 emitters, location not identifiable on available stills | not visible |
| Forward rods | Two long thin rods project from the bow in R2/U2 | visible, **not modelled** and not added (no geometry changes) |

## Mapping to the model (approximations)

- **Hull colour.** Encoded in vertex colour COLOR_0 per face. Orientation
  palette: top `#5c615e`, sides `#464b48`, under `#2f3433`. Sides and undersides
  get an olive tint. Plates are ×0.62 for a dark patch and ×1.20 for a light
  patch over 23 × 17 m cells, with fine 6.5 × 4.5 m variation. Small deck
  greebles are ×0.66. Baked ambient occlusion (7 m distance) darkens recesses.
  This is an artistic approximation of the stills, not sampled colour.
- **Hangar interiors.** `Daedalus_HangarInterior` is a warm emissive material
  on the faces inside the measured bay openings.
- **Bow light strip.** `Daedalus_LightWhite` on the back face of the measured
  horizontal bow slot.
- **Windows.** The baseline fitting windows stay at the same positions, now
  brighter cool white.
- **Engines.** Nozzle faces use darker metal. The glow is a separate effect
  (`DaedalusEngineGlow.glb`) sized from the measured outlets.
- **Not done.** Lilac and red point lights are not placed; they would need a
  separate light-effect layer. Exact canon colours are unknown, and the
  production textures are not available.

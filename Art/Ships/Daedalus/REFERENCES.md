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
| U1–U12 | Images supplied by the user in chat, original URLs unknown | mix (see below) | local study only, not redistributed |

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

The user also described the weapon effects from the footage:

- Turrets fire orange projectiles.
- Missiles leave the dorsal bow silos.
- The Asgard weapons are 4 beams: 2 at the bow and 2 under the hull between
  the hangars. Each still shows one side firing; the other side mirrors it.

On-screen stills (R2, U1, U2, U5, U7–U10) take precedence. Fan model material
(U3, U4) is used only to confirm silhouettes of engines/turrets and never for
counts. The orange silo colour (U11) is an explicit user art direction.

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
| Bow missile silos | Grid of hatches on the dorsal bow; R1 states 16 VLS tubes. The hatches carry amber-orange markings | count from R1; zone visible in R2, U12; colour from U11 (user art direction) |
| Asgard beam weapons | R1: 4 weapons. Blue-white beams leave the bow and the underside between the hangar pods, one side per still. U12 labels the "Asgard beam turret" on the lower side of the bow | visible (U9, U10), labelled (U12); placement on this model is an approximation (see mapping) |
| Weapon effects | Railgun turrets: orange tracer projectiles. VLS: missiles leave the bow silos upward. Asgard: continuous blue-white beam | user description of the footage; U9, U10 for the beams |
| Forward rods | Two long thin rods project from the bow in R2/U2 | visible, **not modelled** and not added (no geometry changes) |

## Mapping to the model (approximations)

- **Hull colour.** Encoded in vertex colour COLOR_0 per face. Orientation
  palette: top `#777d7a`, sides `#5a605d`, under `#3f4442`. Sides and
  undersides get an olive tint from `#868f8a`.
  - Plates: ×0.55 for a dark patch and ×1.25 for a light patch over 23 × 17 m
    cells, with fine 6.5 × 4.5 m variation.
  - Small deck greebles are ×0.66.
  - The engine section is ×0.82 and the pods ×0.93.
  - Baked ambient occlusion (7 m distance) darkens recesses.
  This is an artistic approximation of the stills, not sampled colour. The
  first, darker attempt read too green and too black in renders, so it was
  lightened.
- **Hangar interiors.** `Daedalus_HangarInterior` is a warm emissive material
  on the faces inside the measured bay openings.
- **Bow light strip.** `Daedalus_LightWhite` on the back face of the measured
  horizontal bow slot.
- **Windows.** The 30 baseline window boxes keep their positions as a
  separate cool-white emissive object in `DaedalusLights.glb`. The baseline
  dark hatch squares and vent slats are not in the original model, so they were
  removed at the user's request. Their dark squares did not match the stills.
- **Bow silo hatches.** Each of the 16 raised hatch plates (measured around the
  door down to the deck level) is painted amber-orange `#c98f35` in COLOR_0. It
  is plain paint with no emission, and baked AO still applies. The shade was
  matched by eye to U11, because the image was not available as a file.
- **Asgard beam weapons.**
  - Bow pair: the model has two domes per side on the lower side ledge of the
    bow, exactly where U12 labels the Asgard beam turret. The outer dome of each
    side is the Asgard turret; the inner one stays a railgun proposal.
  - Ventral pair: there is no dedicated emitter on the model. The mount snaps to
    the chamfered front of the block between the hangar pods (between its two
    capsules). That face points forward and down, like the beams in U9.
- **Engines.** Nozzle faces use darker metal. The glow is a separate effect
  (`DaedalusEngineGlow.glb`) sized from the measured outlets.
- **Not done.**
  - Lilac and red point lights are not placed; they would need a separate
    light-effect layer.
  - The forward bow rods are not modelled. Exact canon colours are unknown, and the
  production textures are not available.

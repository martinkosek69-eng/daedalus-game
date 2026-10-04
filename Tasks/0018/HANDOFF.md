# 0018 — Map renderer/input handoff

Status: READY_FOR_REVIEW. Base `6c8db7b`, branch `codex/solar-flight`.
Helper changed only `Solar/GalaxyMapView.h`, `Solar/GalaxyMapView.cpp` and this
task's progress/handoff. No app, build, editor call or Git operation was run.

## Frozen integration interface

`FGalaxyBodyView`, `FGalaxySystemView`, `FMapAction` and `FGalaxyMapView` match
the brief. Approved additions are body `ParentId`, `bKnownPosition=true` and
system `Regions` containing `FGalaxyRegionView` (Name, Geometry, CentreMetres,
InnerMetres, OuterMetres, Color). Regions are only annotated estimated guides,
never physical target bodies or collision geometry.

`FMapAction::EType` is None/Browse/Navigate/Inspect. Its members are Type,
SystemIndex and BodyIndex. Root alone applies these commands. An empty system
has BodyIndex=INDEX_NONE. Browse is emitted for system/body selection and
duration changes. Navigation and explicitly labelled test relocation are
disabled for unavailable systems or bodies without known positions.

Root handles M open/close, cursor/input mode and suppression of flight controls
while browsing. Pass actual mouse cursor/delta, pressed edge, held right/middle
buttons and wheel/Home to Input. Draw follows Input each frame. Hit rectangles
and marker caches come from the most recent Draw. Root passes its runtime Roboto
font; text is rasterized at actual pixel size through FSlateFontInfo.

The search box sets `bSearchFocused`. Root feeds ASCII typed text/backspace via
`SetSearchQuery`; matching folds Czech accented names to plain letters. Search
also matches body ID and translated kind. Wheel over the body list scrolls;
wheel over the map zooms. Kind filters select all/planets/moons/other. Parent
names annotate moon rows when ParentId is supplied.

## Presentation behavior

34100 deterministic illustrative samples produce a genuine perspective barred
spiral with four blue arms, warmer bar/bulge, thin disk, sparse halo and subdued
dust lanes. The distribution is an illustration, not a claimed measured star
catalogue. Samples are depth sorted and batched by Canvas; no per-star actors.
Zoomed samples fade and retain bounded pixel sizes instead of becoming giant
point stars. Right drag orbits, middle drag pans, wheel zooms exponentially,
Home resets the galaxy. Shora/Z boku buttons choose top/side views; pitch allows
both faces of the disk within ±89 degrees. A left list gives independent access
to six systems even when their whole-galaxy markers overlap.

The camera separates its galactic light-year anchor from a local metre offset.
Body focus subtracts body/pivot local metres before conversion to light-years,
preserving small-scale detail at faraway galactic positions. Whole-system focus
uses known planet extents; body detail focus uses known physical radius. Unknown
radius remains an explicit marker, with the database saying radius unknown.
Unknown position is omitted from projection/orbits and labelled Poloha neurčena;
its focus, navigation and inspection actions are blocked.

At local zoom, known-body materials supplied by root render round textured
planet discs through K2_DrawMaterial. Root owns their UObject lifetime and
material projection/lighting. Body markers remain selectable; sparse labels
avoid most overlaps and the selected body retains a highlighted ring. Parent
orbit guides are schematic circles through the current catalogue radius, clearly
labelled orientační; they do not update canonical orbits or ship state.

Annular regions render inner/outer local circles. Geometry names containing
"spher" render three great-circle envelope guides, suitable for uncertain Oort
boundaries. These are faint annotated lines, never opaque clouds. Region labels
should include source uncertainty in their Name supplied by root.

Navigation display uses only root-provided FNavigationMetrics and flags:
centre-to-centre distance, current ETA, full-impulse ETA and required speed for
1 hour/day/week. No hyperdrive, energy consumption or movement is implemented
by the view. Waiting source assets are explicitly shown as Čeká na podklady.

## Probe coordinates

All following positions use logical pixels with
`S=min(viewportWidth/1280, viewportHeight/720)`, minimum 0.55. Multiply by S for
native coordinates. W/H below are viewport dimensions divided by S.

- System row i: x20..244, y134+45*i through y+40.
- System focus: x23..241, y424..451.
- Search: xW-326..W-24, y145..172. Filters y176..203.
- Database rows: xW-326..W-24, y210+24*row through y+22.
- Body detail focus: xW-326..W-24, yH-240..H-213.
- Duration hour/day/week: xW-326, W-225, W-126 respectively, yH-102..H-75.
- Navigation: xW-326..W-24, yH-68..H-41.
- Explicit test relocation: same x, yH-36..H-9.
- Galaxy/top/side buttons: xW-436, W-294, W-201, y20..47.
- Map plot: x266..W-346, y90..H-65; use its centre for orbit/zoom probes.

`MarkerScreenPosition` returns only a currently projected visible system marker.
`GetPivotLY`, `GetCameraDistanceLY`, `GetCameraPitchDegrees` support root checks.

## Actual verification and remaining gates

Static review compared used Canvas/Vector APIs with local Unreal 5.8 source.
It caught the engine's null-texture triangle assertion: glow triangles now use
the Engine-owned white texture exposed by a plain FCanvasTileItem, without a
new RenderCore module dependency. No rooted/UObject graph mutations or hidden
canonical state writes occur. Compile and render success is not claimed.

Root must build, test actual mouse/key handling and body search, verify selection
leaves ship location unchanged, test top/side/deep zoom and unknown-position
blocking, inspect real 4K whole-galaxy/planet/database screenshots and assess
Canvas performance. No actual source assets or renderer output was made by this
helper. Future orbital dynamics, additional galaxies and scrolling a system
list longer than the current six require separate work; this view is a prototype.

## Map provenance

Morphological reference: ESA/Gaia's [Guide to our galaxy](https://www.esa.int/Science_Exploration/Space_Science/Gaia/Guide_to_our_galaxy)
describes the disk, spiral arms, bulge/halo and the Sun's approximately 26000 ly
galactocentric distance. The [2025 Gaia map](https://www.esa.int/ESA_Multimedia/Images/2025/01/Milky_Way_map_by_Gaia_labelled)
is itself an artist's impression and highlights continuing revisions to the
bar/arm geometry. The renderer's four logarithmic arms, point distribution,
palette and fictional system positions are authored illustration choices;
they do not reproduce Gaia measured stars or assert an exact Galactic model.
No ESA image, texture or dataset was downloaded or redistributed by this task.

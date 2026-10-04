# Solar realism source catalog

This is research and source data for the singleplayer game. It does not change
the runtime catalog or claim an integrated scene. Coordinates, unknown values
and appearance choices must remain separate when Codex integrates the data.

## Coverage

The 2026-10-04 snapshots contain 460 rows of the full accessible
[JPL satellite orbital table](https://ssd.jpl.nasa.gov/sats/elem/): **459 unique
planetary moons**, including provisional names. Puck has two solutions; the
newer epoch is selected and the older orbit retained as an alternative.
Counts: Earth 1, Mars 2, Jupiter 115, Saturn 291, Uranus 29, Neptune 16,
Pluto 5. These are coverage of this particular table, not a promise that every
new announcement appears immediately in JPL's published orbital table.

The [JPL physical table](https://ssd.jpl.nasa.gov/sats/phys_par/) provides
46 moon mean radii. Positive ellipsoids from the
[NAIF generic planetary constants kernel](https://naif.jpl.nasa.gov/pub/naif/generic_kernels/pck/pck00011.tpc)
support 25 additional volume-equivalent radii: **71 sizes known, 388 unknown**.
An ellipsoid-derived radius is `(axisA*axisB*axisC)^(1/3)`; the individual axes
and their provenance are retained. Historical reference shapes are approximate.
Do not silently turn an unknown radius into zero or a claimed measurement.

The bounded [JPL SBDB API](https://ssd-api.jpl.nasa.gov/doc/sbdb.html) snapshot
contains **29 selected dwarf planets, asteroids and comets**, including all five
commonly designated dwarf planets, mission targets and nine named periodic
comets. Its 14 moon records add **9 unique small-body moons** after excluding
the five Pluto duplicates. Most additional moon positions are unknown because
the selected API has incomplete elements. Preserve all original references and
sigma values. No million-object asteroid download is required for sparse scenery.
The selected minor-body set supports 24 radii and leaves 5 unknown. Of the nine
additional moons only Dysnomia has a supported radius here; all nine positions
remain unknown because no complete selected source-epoch orbit is supplied.

One reviewed measurement supplement replaces the older Eris radius with
1163 +/- 6 km from occultation, and adds Dysnomia's modeled 350 +/- 57.5 km
radius. The [NASA-hosted Holler et al. paper](https://ntrs.nasa.gov/api/citations/20210012932/downloads/21-49.pdf)
reports these earlier measurements; the previous Eris value remains an
alternative. Other missing measurements are not invented. The
[planet physical summary](https://ssd.jpl.nasa.gov/planets/phys_par.html) can
contain older models, particularly for irregular Haumea; retain source dates.

## Coordinates and runtime integration

`Art/Space/SolarCatalog/satellites.json` uses stable `sol.*` IDs; legacy
24 moon IDs are retained and further moons use their stable JPL satellite code.
`relativePositionMetres` is parent-centred **right-handed ecliptic J2000**.
`positionMetres` is null until the parent is placed. The 29 minor bodies instead
have heliocentric source-epoch positions. Epochs are native to each record,
including 2000, 2020 and 2025; this is **not one coherent current-date ephemeris**.

The parser solves the two-body ellipse at the stated epoch. Moon equatorial
and Laplace frames use published poles; missing parent poles use NAIF polynomial
terms without periodic nutation. This is useful for illustrative layout, not a
navigation accuracy claim. Do not propagate mean elements blindly for decades.
For a common accurate epoch, acquire suitable Horizons/SPICE ephemerides in a
separate reviewed step. The current game has a rotated layout, Earth at origin
and a left-handed engine: integration needs one explicit coordinate transform,
parent translation and precision-preserving SI coordinates. Never copy these
vectors directly into current engine coordinates or mix source epochs unnoticed.

The coordinator owns the **one** canonical runtime catalog. These research
files are import inputs, not a second mutable gameplay state. Missing sizes,
textures and positions require explicit presentation policy: an unknown marker,
or a labeled schematic visual. Such choices must not become physical facts.

## Sparse space, dust and rings

`distributions.json` separates quoted approximate extent from authored sampling
envelopes. The [main asteroid belt](https://science.nasa.gov/solar-system/asteroids/)
lies between Mars and Jupiter; local samples must leave large empty distances.
[Jupiter Trojans](https://science.nasa.gov/mission/lucy/science/) cluster near
L4/L5, roughly 60 degrees ahead and behind Jupiter. Their phase depends on Jupiter.
No measured density or collision population is fabricated.

The [Kuiper belt](https://science.nasa.gov/solar-system/kuiper-belt/facts/)
core is approximately 30-50 AU, with inclined/eccentric scattered populations
reaching much farther. The [Oort population](https://science.nasa.gov/solar-system/oort-cloud/facts/)
is inferred: quoted inner-boundary and outer-extent ranges express uncertainty,
not photographed shell walls. Use distant navigation overlays and sparse local
objects; avoid a visible opaque cloud surrounding the system.

[Zodiacal light](https://www.nasa.gov/missions/serendipitous-juno-spacecraft-detections-shatter-ideas-about-origin-of-zodiacal-light/)
is faint sunlight scattered by diffuse dust. The
[NASA/PDS dust instrument context](https://pds.nasa.gov/data/uly-j-gas-5-sky-maps-v1.0/uly_5001/document/dust/dustinst.htm)
describes grains probed by scattered/thermal light. Flight motion particles may
be a deliberately exaggerated readability aid, separate from astronomical dust.
[Comet activity](https://science.nasa.gov/solar-system/comets/) depends on heating:
coma and tail must not appear as permanent dense spheres around every nucleus.

`rings.json` records **31 ring components**, including the low-density Cassini
division, and the numerically unspecified Zeta ring. Radii are from each planet's
centre. Broad ring edges, narrow reference radii, width ranges and upper limits
are different fields; missing edges stay unknown. The
[Jupiter](https://nssdc.gsfc.nasa.gov/planetary/factsheet/jupringfact.html),
[Saturn](https://nssdc.gsfc.nasa.gov/planetary/factsheet/satringfact.html),
[Uranus](https://nssdc.gsfc.nasa.gov/planetary/factsheet/uranringfact.html) and
[Neptune](https://science.nasa.gov/neptune/neptune-facts/) sources describe
different ring structures. Dark, faint dust rings need different appearance from
Saturn's brighter ice. Summary geometry does not resolve every ringlet or arc.

## Reproduction and checks

Run `python Tools/Fetch-SolarMinorCatalog.py --offline` to reproduce normalized
JSON from checked-in public snapshots and reviewed numeric supplements.
Without `--offline`, table snapshots are refreshed and missing curated SBDB
queries fetched sequentially. Delete/replace a specific API snapshot record
intentionally to refresh it; ordinary offline verification never needs network.

The parser checks table widths, unique body IDs, positive axes, elliptic element
domains, Kepler residual and periapsis/apoapsis distance bounds. Final review also
checks finite numbers, exact counts, preserved legacy IDs and repeatable hashes.
No Unreal visual, streaming, collision or GPU performance test is implied by
successful source normalization. Those remain coordinator integration work.

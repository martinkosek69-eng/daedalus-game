# 0014 handoff

Status: READY_FOR_REVIEW, not ACCEPTED. No Git operation performed by helper.

Changed only assigned new paths:

- `Tools/Fetch-SolarMinorCatalog.py`: stdlib sequential/cached downloader,
  table/kernel parsers, offline normalization, Kepler positions and reviewed
  scientific/environment supplements. No game data/assets/app mutation.
- `Art/Space/SolarCatalog/**`: three full public table snapshots, derived NAIF
  poles/95 positive reference ellipsoids, 29 raw bounded API responses, reviewed
  Eris/Dysnomia numeric supplement, five normalized outputs and source credits.
- `Docs/SOLAR_REALISM.md`: source links, actual coverage, coordinate/uncertainty
  contract, sparse space/ring/dust behavior and integration limitations.
- This task's progress/handoff files.

Normalized output counts:

| File | Records | Supported radii | Source positions |
| --- | ---: | ---: | ---: |
| satellites.json | 459 | 71 | 459 parent-relative at native epoch |
| minor-bodies.json | 29 | 24 | 29 heliocentric at native epoch |
| minor-body-satellites.json | 9 | 1 | 0; explicitly unknown |
| rings.json | 31 | component geometry fields | parent equatorial planes |
| distributions.json | 7 | not applicable | population extents, not body placements |

Checks: `python Tools/Fetch-SolarMinorCatalog.py --offline` twice, identical output
SHA-256 across all five normalized files; all JSON finite; unique IDs across 497
combined body definitions; positive known radii; 24 legacy moon IDs preserved;
source table width assertions, orbital domain/Kepler residual/distance bounds.
No engine or visual verification is claimed.

Coordinator import requirements:

1. Preserve the one canonical runtime catalog. Source vectors are right-handed
   ecliptic J2000, not the current rotated engine frame. Explicitly transform
   axes/handedness and translate moon vectors by parent location in SI doubles.
2. Native source epochs differ. Retain epochs/status, use only a declared
   illustrative fixed layout, or obtain common-epoch Horizons/SPICE positions
   before making a scientifically accurate time simulation claim.
3. Unknown radius/position/texture remains null. Use an explicit unknown marker
   or labeled authored schematic policy, never fake a measured size or terrain.
   All extra small-body moons need placement research or an authored-phase policy.
4. Sampling density/LOD/brightness/particle sizes are presentation decisions;
   sparse belt garnish is not extra authoritative persistent catalog bodies.
   Oort ranges are uncertainty on boundaries, not dense shell intervals.
5. Broad ring edges versus narrow reference radius/width/upper limit have
   different schemas. Zeta numeric geometry is unknown. No complete ringlet/arc
   reconstruction is claimed. Dust-ring optical depth differs greatly from Saturn.

No existing SolarSystem asset or runtime system.json was edited. No apps were
started, user's applications untouched, and no build/commit/push performed.

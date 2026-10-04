# Solar catalog provenance and reuse

These are public scientific data snapshots and original normalized JSON, not
private machine logs, imported photographs or a full asteroid database.
Retrieved dates and original payload SHA-256 are embedded in the table/kernel
snapshots. SBDB query URLs, service signatures, references and retrieval times
are embedded in `small-body-api.json`. Numerical supplements and environment
summaries carry their source URLs and review date.

Credit: NASA/JPL Solar System Dynamics; JPL NAIF; NASA Goddard/NSSDCA planetary
factsheets curated by David R. Williams; cited original investigators for physical
measurements; NASA/PDS dust context. Preserve these credits when redistributing
derived data. Do not imply NASA/JPL endorsement or use agency logos as game brands.

- [NASA media/use guidance](https://www.nasa.gov/nasa-brand-center/images-and-media/)
  describes generally reusable NASA material and third-party exceptions. These
  files extract scientific facts; they do not assert that every linked third-party
  paper or image has a blanket Creative Commons licence.
- [NAIF rules](https://naif.jpl.nasa.gov/naif/rules.html) permit using kernels,
  including commercial use. `planet-poles.json` is explicitly our derived summary,
  not an altered file presented as the official kernel. No toolkit is bundled.
- [JPL API documentation](https://ssd-api.jpl.nasa.gov/doc/sbdb.html) defines field
  units and API access. Requests in the reusable parser remain sequential and
  bounded; the checked-in snapshots permit offline operation.
- The [NASA-hosted Holler et al. paper](https://ntrs.nasa.gov/api/citations/20210012932/downloads/21-49.pdf)
  has an Elsevier copyright notice. Only the cited numerical Eris/Dysnomia radius
  facts are transcribed; the paper, text and figures are not redistributed.

`satellite-physical-table.json`, `satellite-orbital-table.json` and
`planet-physical-table.json` preserve raw row cells, references and uncertainty
notation. `planet-poles.json` preserves selected coefficients and positive
ellipsoid axes. `small-body-api.json` preserves all 29 bounded query responses,
including five duplicate Pluto satellite records excluded from normalized output.

Normalized deliverables: `satellites.json`, `minor-bodies.json`,
`minor-body-satellites.json`, `rings.json`, `distributions.json`.
Rebuild with `Tools/Fetch-SolarMinorCatalog.py --offline`; source inputs must
remain alongside it. `supplemental-measurements.json` is an explicitly reviewed
numeric transcription, not automatically fetched research prose.

No new surface texture is provided. `texture:null` means unmapped in this project.
Unknown radius/orbit/uncertainty is not a licence to claim fictional surface
detail as observed. See `Docs/SOLAR_REALISM.md` for epoch/frame and runtime limits.

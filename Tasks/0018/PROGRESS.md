# 0018 — Interactive galaxy-map view

Status: READY_FOR_REVIEW

Implemented the isolated renderer/input class and agreed catalogue presentation
types. Root owns the catalogue, UObject lifetimes, canonical navigation/flight
commands, source assets, real application integration, builds and publication.

The view emits Browse/Navigate/Inspect actions and never changes actual ship
state. It renders an illustrative 3D barred spiral, true catalogue positions,
local planet detail, region/orbit guides and a searchable body database. Unknown
positions remain explicit database records without fabricated map locations.

Static API/source review completed; actual C++ compilation, packaged mouse
interaction, performance and native 4K images remain root acceptance checks.

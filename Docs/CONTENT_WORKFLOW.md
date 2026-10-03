# Content contract

## New system / galaxy

Add a unique galaxy/system ID and system-local body definitions to canonical
JSON. Validate references and numbers. Galaxies can have separate strategic
coordinate origins; avoid claiming exact intergalactic travel calculations until
their transform model is implemented. Authored set pieces use separate scene
assets associated with IDs, not one enormous permanently loaded universe map.

## New ship

Deliver one unique ship definition (physical dimensions, capacities, weapon ID),
editablе .blend, approved export, dependent textures, source/rights record, and
render/size checks. +X forward, +Z up; Blender metre units and applied transforms.
Engine import must be independently measured. Model dimensions are not inferred
from file name. Mount/engine/interaction sockets get documented IDs before use.
Engine art uses Git LFS; handoff includes source and imported dependencies, not
just the LFS pointer. Assign the binary owner before concurrent work.

The current original cruiser is a reusable synthetic geometry fixture, generated
by Tools/Create-FoundationAssets.py and imported by Prepare-FoundationContent.py.
It has no third-party models/textures. Publication does not establish a new
license for unrelated franchise content; the old prototype assets remain outside
this public repository.

## Location / mission / behavior

Locations specify permanent identity, system, local position and kind. Surface
and interior detailed scenes can be added without rewriting ship persistence.
Missions/factions/equipment are extension work, not fabricated complete systems.
Before adding them specify state ownership, commands/events, save changes and
acceptance checks. Do not encode mission truth in a UI widget or mesh visibility.

## Review

Check definitions, IDs, units, source assets, dependencies, collision approximation,
loading/unloading and performance in actual standalone builds. Compare state
before/after travel and save/load. Repeatedly revisiting regions must not retain
every region's visuals. Actor counts and memory trends are meaningful checks;
successful catalog parsing alone is not large-world performance validation.

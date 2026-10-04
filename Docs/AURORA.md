# Aurora-class — source, research and playable flight lab

The GitHub library model is source commita193736 in
`claude/aurora-ancient-warship`. It is an Ancient/Lantean Stargate Atlantis
warship, not the similarly named ships in Star Trek or Star Citizen.

## Research and assumptions (2026-10-04)

Aurora is an advanced Lantean warship with shields, drones, control interfaces,
stasis facilities and a hyperdrive. Appearances include Aurora, Orion and Tria.
Sources: [SGCommand class reference](https://stargate.fandom.com/wiki/Aurora-class_battleship),
[Aurora episode](https://www.gateworld.net/atlantis/s2/aurora/),
[Helia/Tria](https://gateworld.net/wiki/Helia).
The Tria story includes travel at0.999c after a hyperdrive failure. This is a
particular ship/exceptional operating condition, not proof every hull always
reaches that speed. Original interstellar drives and modified intergalactic
ones must also not be conflated. Advanced Ancient technology does not mean it
automatically exceeds every later Asgard enhancement of a304 in every system.

Exact screen dimensions vary; online size estimates conflict. **3500m is the
provided model's provisional working scale, not a verified canon measurement.**
The game uses the model at that scale relative to the existing600m Daedalus.
No unsupported mass, power, weapon counts or manoeuvre measurements are invented.
All drive/steering numbers below are explicit game balancing choices.

| Flight lab setting | Daedalus | Aurora |
| --- | ---: | ---: |
| Working hull length |600m |3500m |
| Conservative collision sphere |360m |1950m |
| Harbour cap |150m/s |120m/s |
| Normal impulse cap |250km/s |350km/s |
| Full sublight cap |250000km/s |299492.666km/s (0.999c) |
| Time to normal/full cap from rest |2.5s |2s |
| Turn limit at full speed |10.8°/s |6.8°/s |
| Turn limit at rest |15.66°/s |10.2°/s |
| Maximum low-speed bank |36° |26° |

Both use the same fixed120Hz assisted flight model, no sideways drift, gradual
pitch/bank boundaries and speed-dependent manoeuvring. This preserves the
requested readable ship control rather than modelling relativistic forces or
time dilation. Aurora feels larger but remains responsive through the stronger
drive. Its conservative clearance sphere must cover the actual exported hull.

## Asset preparation

Original source attribution is in Art/Ships/Aurora/SOURCE.md (Martin/Printables,
CC BY geometry; Stargate design attribution preserved). Original .blend is
unchanged. Tools/Inspect-AuroraScene.py reads the actual saved scene; no GUI
session is assumed. Tools/Prepare-Aurora.py cleans numerically degenerate STL
edges, creates an atlas and bakes the original procedural paint through Cycles.
Base colour8192×8192, ORM/emission/normal4096×4096. Materials are three source
slots; the separate blue orb retains its original constant emissive material.
The baked atlas changes neither the silhouette nor the copper/steel layout.
Fine charts inevitably have finite texture density;4K output is not unlimited
surface detail at arbitrarily close distances.

Prepare-AuroraContent.py imports only Aurora into its own folder, with normal
green conversion, opaque lit materials, full texture resolution and the complete
mesh (Nanite disabled, no generated reduced-detail hull). The Blender preview
uses a different lighting rig; in-game lighting is the shared star/fill rig.
No ship-specific artificial studio lights or photographed backgrounds are added.

Saved Aurora does not contain separate animated hangar doors or engine modules.
They are not silently invented. Daedalus's latest H doors, beacons and engine
effects remain operational. Aurora window/orb emission is part of the source
look. HUD shield/weapon/energy sections remain placeholders; Aurora identifies
drones/pulse weapons without claiming counts or fabricated mount positions.

## Controls and boundaries

P opens the clickable pause menu. Choose Přepnout loď, then Daedalus or Aurora.
Default remains Daedalus. P or Pokračovat ve hře resumes. W/S,A/D,Q/E,R,
Shift+R and map controls are unchanged. The camera adjusts to each hull size;
HUD uses its actual projected silhouette and correct name. Switching preserves
position, heading, speed, throttle, clock and navigation; speed above the new
drive's cap slows gradually. Swaps work while the menu keeps the flight paused;
they are blocked during map, inside a new
hull clearance sphere or while attitude exceeds the new ship's allowed turn/
bank limits. Finish the turn/move away and try again. Changes do not touch the
foundation save schema; this remains the isolated singleplayer flight lab.

Menu also offers exit and read-only graphics settings. Save/load entries are
explicitly unavailable until a flight-lab save contract is designed. The separate
foundation persistence system remains unchanged. UI never pretends to save a
flight. ESC is planned as the menu key for the final game; currently it closes
the menu/map, and outside them retains the earlier quit shortcut.

Authoritative tuning: Game/Daedalus/Content/Data/Solar/ships.json. Legacy
flight.json tuning is retained for compatibility checks; its world start
settings remain active, while ship profiles now come from the catalog.
The adapter owns presentation references; simulation catalog owns no meshes.
An additional hull needs its reviewed presentation adapter and catalog entry.

Rebuild: Prepare-Aurora.py through saved-source background Blender; Invoke-
SolarFlight.ps1 AuroraAssets (or Assets for both recipes), Build/Test/Package;
Invoke-AuroraProbe.ps1 tests the separate package at actual3840×2160.

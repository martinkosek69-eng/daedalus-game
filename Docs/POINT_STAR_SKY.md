# Point-star sky baseline

User explicitly requested generated points like the original web version,
not a photograph or bitmap sky. Runtime uses the existing8920HYG catalogue,
equatorial-to-ecliptic conversion, magnitude/colour and analytic masked points.
The magnitude curve is the web celestial-effects.mjs curve:
clamp(10^(-0.16*(mag-1)),0.09,1.3). No random invented star positions, atmospheric
twinkling, doubled photographic stars, blurred nebulae or bloom halos.
The historic M_Sky source is retained but its sphere is invisible. Galaxy map
is a separate presentation and retains its authored3D structure.

Primary references informing this human-view artistic baseline:
- [NASA: Why stars twinkle](https://apod.nasa.gov/rjn/apod/ap000725.html):
  distant stars are unresolved points; atmospheric twinkling does not apply in space.
- [NASA: Why space is black](https://starchild.gsfc.nasa.gov/docs/StarChild/questions/question52.html).
- [NASA Night Sky Network: The Great Rift](https://nightsky.jpl.nasa.gov/news/444/):
  long-exposure photos show more colour/detail than the naked eye.
- [NASA: Stars in space images](https://cosmicopia.gsfc.nasa.gov/qa_star.html):
  apparent visibility depends on exposure and brightness of foreground objects.

Visible-star exposure is an artistic compromise for reading the ship and sky
together, not a calibrated retinal or camera simulation. Finite pixel resolution
still applies. HYG is a Sol observer reference even in the five authored systems.

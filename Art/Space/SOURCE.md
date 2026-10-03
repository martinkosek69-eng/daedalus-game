# Solar scene sources

Earth day/night/cloud maps and Sun surface: Solar System Scope / INOVE,
https://www.solarsystemscope.com/textures/ , CC BY 4.0,
https://creativecommons.org/licenses/by/4.0/ . Existing 2k files recovered from
the user's original prototype; imported as engine textures. Colors/shading are
adapted for the Unreal flight laboratory; clouds are a static texture layer.

Stars: David Nash / Astronexus HYG 4.1, CC BY-SA 4.0. The original prototype's
unchanged 8,920-entry subset and provenance are Content/Data/Solar/stars-hyg.json
and stars-source.json. That data remains CC BY-SA 4.0:
https://github.com/astronexus/HYG-Database and
https://creativecommons.org/licenses/by-sa/4.0/ . Equatorial J2000 positions
are converted into ecliptic directions; magnitude/color are compressed for a
monitor. No scintillation, parallax or positional proper-motion simulation.

SolarSphere geometry is an original generic UV sphere, not a downloaded planet
model. Celestial rendering preserves angular size while projecting very distant
bodies closer to the camera; authoritative positions/radii stay in real metres.
Sun/Earth are fixed reference bodies for flight testing, not an orbital ephemeris.

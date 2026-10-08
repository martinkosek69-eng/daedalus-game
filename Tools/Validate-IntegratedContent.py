"""One read-only commandlet for reviewed planets and latest ship bindings."""
import runpy
from pathlib import Path

for script in ('Validate-PlanetQuality.py', 'Validate-ShipPresentation.py'):
    runpy.run_path(str(Path(__file__).parent / script), run_name='__main__')

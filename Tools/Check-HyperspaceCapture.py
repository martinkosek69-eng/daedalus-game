"""Check native capture and aperture visibility (not artistic likeness).

Usage: python Tools/Check-HyperspaceCapture.py PATH_TO_CAPTURE
Requires Pillow and NumPy, as does the local reference-analysis workflow.
The fixed ROI belongs to SolarHyperspace's exterior rehearsal camera.
"""
import argparse
import json
from pathlib import Path
import numpy as np
from PIL import Image

parser = argparse.ArgumentParser()
parser.add_argument('capture', type=Path)
args = parser.parse_args()
report = json.loads((args.capture / 'result.json').read_text(encoding='utf-8-sig'))
assert report['passed'] and report['flightStateUnchanged'], 'Runtime capture failed'
assert (report['width'], report['height']) == (3840, 2160)
for index in range(report['frames']):
    with Image.open(args.capture / f'{index:04}.png') as im:
        assert im.size == (3840, 2160), f'Non-native frame {index}'

def aperture(index):
    with Image.open(args.capture / f'{index:04}.png') as im:
        return np.asarray(im.convert('RGB').crop((800, 250, 2050, 1700)), dtype=np.int16)

base = aperture(0)
counts = {}
for index in (24, 38, 103):
    frame = aperture(index)
    light = (frame[:, :, 1] - base[:, :, 1] > 60) & (frame[:, :, 1] > 90)
    counts[str(index)] = int(light.sum())
assert counts['24'] > 150, 'Opening is missing, possibly cold PSO compilation'
assert counts['38'] > 10000, 'Full aperture is missing, possibly cold PSO compilation'
assert counts['103'] < counts['38'] * .03, 'Aperture remains after closure'
output = {'passed': True, 'nativeFrames': report['frames'], 'width': 3840,
          'height': 2160, 'apertureBrightPixelCounts': counts,
          'checks': ['opening visible', 'full aperture visible', 'closure clears'],
          'artisticLikenessVerified': False}
(args.capture / 'visual-check.json').write_text(json.dumps(output, indent=2) + '\n', encoding='utf-8')
print(json.dumps(output))

"""Check final 0030 frame integrity/source consistency, not aesthetic likeness.

Run with Python containing Pillow and NumPy. Encoded movie decoding is a
separate ffmpeg check; this tool must not claim the movie has been decoded.
"""
import argparse
import hashlib
import json
from pathlib import Path
import numpy as np
from PIL import Image

p=argparse.ArgumentParser()
p.add_argument('--root',type=Path,required=True)
a=p.parse_args()
folder=a.root/'.local/hyper/blender0030'
blend=a.root/'Art/Effects/Hyperspace/Blender/Daedalus_GreenRupture.blend'
sha=hashlib.sha256(blend.read_bytes()).hexdigest()
frames=folder/'frames'/sha[:12]
assert (frames/'source-sha256.txt').read_text().strip()==sha
reload=json.loads((folder/'reload-check.json').read_text())
assert reload['blendSHA256']==sha and not reload['missingImages'] and not reload['unpackedFileImages']
for i in range(1,109):
    with Image.open(frames/f'{i:04}.png') as image:
        assert image.size==(3840,2160), f'Wrong native dimensions, frame {i}'
        image.verify()

def roi(i):
    with Image.open(frames/f'{i:04}.png') as image:
        return np.asarray(image.convert('RGB').crop((1650,200,3200,1750)),dtype=np.int16)

baseline=roi(1)
counts={}
for i in (20,44,61,90,108):
    pixels=roi(i)
    counts[str(i)]=int(((pixels[:,:,1]-baseline[:,:,1]>55)&(pixels[:,:,1]>85)).sum())
assert counts['20']>500, 'Opening is missing'
assert counts['44']>counts['20']*3, 'Aperture does not open fully'
assert counts['61']>10000, 'Aperture disappears during crossing'
assert counts['90']<counts['44']*.02 and counts['108']<counts['44']*.02, 'Aperture fails to close'
report={'passed':True,'nativeFrames':108,'resolution':[3840,2160],'fps':24,
        'sourceSHA256':sha,'apertureBrightPixelCounts':counts,
        'allPNGIntegrityChecksPassed':True,'packedDependenciesVerified':True,
        'artisticLikenessAccepted':False,'unrealIntegration':False}
(folder/'final-check.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print(json.dumps(report))

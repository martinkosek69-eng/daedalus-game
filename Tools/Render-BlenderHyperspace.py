"""Render saved task0030 scene, resuming complete native PNG frames safely."""
import argparse
import hashlib
import json
import struct
import sys
from pathlib import Path
import bpy

p=argparse.ArgumentParser()
p.add_argument('--root',type=Path,required=True)
p.add_argument('--frames',default='1:108')
p.add_argument('--audit-only',action='store_true')
a=p.parse_args(sys.argv[sys.argv.index('--')+1:])
scene=bpy.context.scene
assert (scene.render.resolution_x,scene.render.resolution_y,scene.render.resolution_percentage)==(3840,2160,100)
assert not scene.render.use_motion_blur and not scene.camera.data.dof.use_dof
missing=[im.name for im in bpy.data.images if im.source=='FILE' and not im.packed_file
         and not Path(bpy.path.abspath(im.filepath)).is_file()]
assert not missing, f'Missing images: {missing}'
report={'blendReloadPassed':True,'blender':bpy.app.version_string,'resolution':[3840,2160],
        'fps':scene.render.fps,'engine':scene.render.engine,'missingImages':missing,
        'unpackedFileImages':[im.name for im in bpy.data.images if im.source=='FILE' and not im.packed_file],
        'meshes':len([o for o in scene.objects if o.type=='MESH']),
        'animatedMembranes':len([o for o in scene.objects if o.name.startswith('Tear ') and o.data.shape_keys]),
        'renderedFrames':[]}
out=a.root/'.local/hyper/blender0030'
out.mkdir(parents=True,exist_ok=True)
source_hash=hashlib.sha256(Path(bpy.data.filepath).read_bytes()).hexdigest()
report['blendSHA256']=source_hash
(out/'reload-check.json').write_text(json.dumps(report,indent=2)+'\n')
if not a.audit_only:
    if ':' in a.frames:
        start,end=map(int,a.frames.split(':'))
        frames=range(start,end+1)
    else:
        frames=[int(x) for x in a.frames.split(',')]
    folder=out/'frames-final'
    folder.mkdir(exist_ok=True)
    stamp=folder/'source-sha256.txt'
    if stamp.exists():
        assert stamp.read_text().strip()==source_hash, 'Scene changed; use a fresh frame folder before rendering'
    else:
        assert not list(folder.glob('*.png')), 'Unidentified older frames: use a fresh folder'
        stamp.write_text(source_hash+'\n')
    for frame in frames:
        assert scene.frame_start<=frame<=scene.frame_end
        path=folder/f'{frame:04}.png'
        path.parent.mkdir(exist_ok=True)
        valid=False
        if path.exists():
            with path.open('rb') as f:header=f.read(24)
            valid=(header[:8]==b'\x89PNG\r\n\x1a\n' and struct.unpack('>II',header[16:24])==(3840,2160)
                   and path.read_bytes()[-8:]==b'IEND\xaeB`\x82')
        if not valid:
            scene.frame_set(frame)
            scene.render.filepath=str(path)
            bpy.ops.render.render(write_still=True)
        report['renderedFrames'].append(frame)
        print(f'FRAME_COMPLETE {frame}',flush=True)
    (out/'render-check.json').write_text(json.dumps(report,indent=2)+'\n')
print('BLENDER_HYPERSPACE_CHECK_PASS',flush=True)

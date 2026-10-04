"""Read-only Blender validation of reviewed fictional system deliveries.
Run: blender --background --factory-startup --python Tools/Validate-SystemAssets.py
Never rewrites source scenes or operates an existing Blender GUI.
"""
import bpy, json, math
from pathlib import Path
ROOT=Path(__file__).resolve().parent.parent
REPORT=[]
for name in ('Asterion','Velara','Nivara','Caelum','Morava'):
    folder=ROOT/'Art/Space/Systems'/name
    data=json.loads((folder/'system.json').read_text(encoding='utf-8-sig'))
    rows=data['bodies'];by_id={r['id']:r for r in rows}
    assert len(rows)==len(by_id)
    assert len([r for r in rows if r['kind']=='planet']) in range(4,8)
    assert len([r for r in rows if r['kind']=='moon'])>=5
    assert data['primaryStarId'] in by_id and by_id[data['primaryStarId']]['positionMetres']==[0,0,0]
    for row in rows:
        assert math.isfinite(row['radiusMetres']) and row['radiusMetres']>0
        assert len(row['positionMetres'])==3 and all(math.isfinite(v) and abs(v)<=1e15 for v in row['positionMetres'])
        assert all(math.isfinite(v) and v>0 for v in row['shapeScale'])
        if row['parentId']:
            parent=by_id[row['parentId']]
            guard=max(parent['radiusMetres']*max(parent['shapeScale']),parent.get('ringOuterMetres',0))
            assert math.dist(row['positionMetres'],parent['positionMetres'])>guard+row['radiusMetres']*max(row['shapeScale']),row['id']
        if 'ringOuterMetres' in row:assert row['radiusMetres']<row['ringInnerMetres']<row['ringOuterMetres']
    images={r[k] for r in rows for k in ('texture','cloudTexture','nightTexture','ringTexture') if r.get(k)}
    for image in sorted(images):
        decoded=bpy.data.images.load(str(folder/image),check_existing=False)
        width,height=decoded.size[:]
        assert width>=2048 and (width==2*height or image.endswith('_ring.png')),(name,image,width,height)
        # Access decoded pixels, beyond only trusting a header.
        assert len(decoded.pixels)>0 and all(math.isfinite(v) for v in decoded.pixels[:4])
        bpy.data.images.remove(decoded)
    glbs=list((folder/'Models').glob('*.glb'))
    for glb in glbs:
        bpy.ops.wm.read_factory_settings(use_empty=True)
        bpy.ops.import_scene.gltf(filepath=str(glb))
        meshes=[o for o in bpy.context.scene.objects if o.type=='MESH']
        assert meshes
        radius=max((o.matrix_world@v.co).length for o in meshes for v in o.data.vertices)
        assert abs(radius-1)<.01,(glb,radius)
    bpy.ops.wm.open_mainfile(filepath=str(folder/(name+'.blend')))
    assert 'SM_UnitSphere' in bpy.data.objects and 'SchematicPreview' in bpy.data.collections
    missing=[i.filepath for i in bpy.data.images if i.source=='FILE' and i.filepath and not Path(bpy.path.abspath(i.filepath)).is_file()]
    assert not missing,(name,missing)
    REPORT.append(dict(system=data['id'],bodies=len(rows),textures=len(images),models=len(glbs),blendReopened=True))
print('SYSTEM_ASSETS_PASS '+json.dumps(REPORT))

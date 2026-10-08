"""Read-only source/export integrity and game-scale checks, outside UE."""
import hashlib,json,struct,math
from pathlib import Path
from PIL import Image
root=Path(__file__).resolve().parents[1]; folder=root/'Art/Ships/Aurora'
audit=json.loads((folder/'GAME_ASSET.json').read_text())
for file,key in [('Aurora_Class.blend','sourceSHA256'),('Aurora.glb','gameSHA256')]:
    assert hashlib.sha256((folder/file).read_bytes()).hexdigest()==audit[key],file
raw=(folder/'Aurora.glb').read_bytes(); n=struct.unpack_from('<I',raw,12)[0]
doc=json.loads(raw[20:20+n]);data=raw[28+n:]
assert len(doc['meshes'])==1 and len(doc['materials'])==3
def values(i):
    a=doc['accessors'][i];v=doc['bufferViews'][a['bufferView']]
    dims={'SCALAR':1,'VEC2':2,'VEC3':3}[a['type']]
    fmt='<'+{5123:'H',5125:'I',5126:'f'}[a['componentType']]*dims
    size=struct.calcsize(fmt);start=v.get('byteOffset',0)+a.get('byteOffset',0)
    return (struct.unpack_from(fmt,data,start+k*v.get('byteStride',size)) for k in range(a['count']))
low=[math.inf]*3;high=[-math.inf]*3;radius=0;triangles=0
for p in doc['meshes'][0]['primitives']:
    assert 'NORMAL' in p['attributes'] and 'TEXCOORD_0' in p['attributes']
    for v in values(p['attributes']['POSITION']):
        assert all(math.isfinite(x) for x in v)
        radius=max(radius,math.sqrt(sum(x*x for x in v)))
        for i,x in enumerate(v):low[i]=min(low[i],x);high[i]=max(high[i],x)
    assert all(all(math.isfinite(x) for x in uv) for uv in values(p['attributes']['TEXCOORD_0']))
    triangles+=doc['accessors'][p['indices']]['count']//3
assert abs(high[0]-low[0]-3500)<.01,(low,high)
assert triangles==audit['triangles'],(triangles,audit)
catalog=json.loads((root/'Game/Daedalus/Content/Data/Solar/ships.json').read_text(encoding='utf-8'))
aurora=next(s for s in catalog['ships'] if s['id']=='aurora')
assert aurora['radiusMetres']>=radius,('collision sphere fails to cover actual mesh',radius)
for role,info in audit['maps'].items():
    with Image.open(folder/'Textures'/info['file']) as image:assert list(image.size)==info['size'] and min(image.size)>=4096
assert audit['cleanedNonManifoldEdges']==0
result={'passed':True,'triangles':triangles,'lengthMetres':3500,'maxVertexRadiusMetres':round(radius,3),
    'collisionRadiusMetres':aurora['radiusMetres'],'removedNearZeroSourceFaces':audit['degenerateFaces']-audit['cleanedDegenerateFaces'],
    'remainingNumericalNearZeroFaces':audit['cleanedDegenerateFaces'],'materials':3,'colourMap':8192,'otherMaps':4096}
p=root/'.local/aurora/source-validation.json';p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(result,indent=2))
print('AURORA_SOURCE_PASS',json.dumps(result))

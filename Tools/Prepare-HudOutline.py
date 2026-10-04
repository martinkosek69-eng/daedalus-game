import json, struct, math
from pathlib import Path
from PIL import Image, ImageDraw

root=Path(__file__).resolve().parents[1]
b=(root/'Art/Ships/Daedalus/Daedalus.glb').read_bytes()
n=struct.unpack_from('<I',b,12)[0]; doc=json.loads(b[20:20+n]); data=b[28+n:]
def accessor(i):
    a=doc['accessors'][i]; v=doc['bufferViews'][a['bufferView']]
    count={'SCALAR':1,'VEC3':3}[a['type']]
    fmt={5123:'H',5125:'I',5126:'f'}[a['componentType']]*count
    size=struct.calcsize('<'+fmt); start=v.get('byteOffset',0)+a.get('byteOffset',0)
    return [struct.unpack_from('<'+fmt,data,start+k*v.get('byteStride',size)) for k in range(a['count'])]
primitives=doc['meshes'][0]['primitives']
parts=[(accessor(p['attributes']['POSITION']),accessor(p['indices'])) for p in primitives]
points=[(-v[2],-v[0]) for vs,ids in parts for v in vs]
lo=[min(p[i] for p in points) for i in range(2)]; hi=[max(p[i] for p in points) for i in range(2)]
scale=300/(hi[1]-lo[1]); W=math.ceil((hi[0]-lo[0])*scale)+4; H=304
mask=Image.new('1',(W,H)); draw=ImageDraw.Draw(mask)
for vs,ids in parts:
    coords=[((-v[2]-lo[0])*scale+2,(-v[0]-lo[1])*scale+2) for v in vs]
    for i in range(0,len(ids),3):draw.polygon([coords[ids[k][0]] for k in (i,i+1,i+2)],fill=1)
pix=mask.load(); edges={}
for y in range(H):
    for x in range(W):
        if not pix[x,y]:continue
        if y==0 or not pix[x,y-1]:edges[(x,y)]=(x+1,y)
        if x==W-1 or not pix[x+1,y]:edges[(x+1,y)]=(x+1,y+1)
        if y==H-1 or not pix[x,y+1]:edges[(x+1,y+1)]=(x,y+1)
        if x==0 or not pix[x-1,y]:edges[(x,y+1)]=(x,y)
loops=[]
while edges:
    p=next(iter(edges)); loop=[]
    while p in edges:
        loop.append(p);p=edges.pop(p)
    loops.append(loop)
outline=max(loops,key=len)
def reduce(pts):
    if len(pts)<3:return pts
    ax,ay=pts[0]; bx,by=pts[-1]; den=math.hypot(bx-ax,by-ay)
    distances=[abs((bx-ax)*(ay-y)-(ax-x)*(by-ay))/max(den,1e-9) for x,y in pts]
    i=max(range(len(pts)),key=distances.__getitem__)
    return reduce(pts[:i+1])[:-1]+reduce(pts[i:]) if distances[i]>1.1 else [pts[0],pts[-1]]
outline=reduce(outline)
path='M'+' L'.join(f'{x},{y}' for x,y in outline)+' Z'
(root/'Tasks/0024/Preview/outline.json').write_text(json.dumps({'path':path,'width':W,'height':H,'projectionLower':lo,'projectionScale':scale}),encoding='utf-8')
print('Projected actual Daedalus hull:',len(outline),'outline points')

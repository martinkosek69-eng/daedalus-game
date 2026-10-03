"""Recover prototype Daedalus: background Blender, from repository-owned source.
Run: blender --background --python Tools/Prepare-DaedalusSource.py
Outputs are overwriteable derived assets; Original remains untouched.
"""
import bpy, json, math, hashlib
import numpy as np
from pathlib import Path
from mathutils import Vector, Matrix
from mathutils.bvhtree import BVHTree

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / 'Art/Ships/Daedalus'
TMP = ROOT / '.local/solar-art'
TMP.mkdir(parents=True, exist_ok=True)
SOURCE = OUT / 'Original/daedalus.glb'
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=str(SOURCE))
scene = bpy.context.scene
scene.unit_settings.system = 'METRIC'
scene.unit_settings.scale_length = 1.0
parts = [o for o in scene.objects if o.type == 'MESH']
assert sum(len(o.data.polygons) for o in parts) == 222330
# Importer maps source -Z to Blender +Y. Rotate +Y to engine +X.
for o in parts:
    o.matrix_world = Matrix.Rotation(-math.pi/2, 4, 'Z') @ o.matrix_world
bpy.ops.object.select_all(action='DESELECT')
for o in parts: o.select_set(True)
bpy.context.view_layer.objects.active = parts[0]
bpy.ops.object.join()
ship = bpy.context.object
ship.name = 'SM_Daedalus'
bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
ship.data.name = 'Daedalus_OriginalHull'
ship.data.materials.clear()
mesh = ship.data
mesh.update()
assert abs(ship.dimensions.x - 600) < .01

# Source-space runtime shader sampled into portable linear vertex colours.
# Fine derivative-based seams are omitted; source geometry supplies edge detail.
v = np.empty(len(mesh.vertices)*3,dtype=np.float32)
mesh.vertices.foreach_get('co', v)
v = v.reshape(-1,3)
loopvi=np.empty(len(mesh.loops),dtype=np.int32)
mesh.loops.foreach_get('vertex_index',loopvi)
p=v[loopvi]/600
p=np.stack((-p[:,1],p[:,2],-p[:,0]),axis=1)
n=np.empty(len(mesh.loops)*3,dtype=np.float32)
mesh.corner_normals.foreach_get('vector',n)
n=n.reshape(-1,3)
n=np.stack((-n[:,1],n[:,2],-n[:,0]),axis=1)
a=np.abs(n)
uv=np.where((a[:,1]>np.maximum(a[:,0],a[:,2]))[:,None],p[:,[0,2]],np.where((a[:,0]>a[:,2])[:,None],p[:,[2,1]],p[:,[0,1]]))
grid=uv*np.array([27,43]);grid[:,0]+=np.mod(np.floor(grid[:,1]),2)*.37
h=np.sin(np.floor(grid)@np.array([127.1,311.7]))*43758.5453
panel=.76+(h-np.floor(h))*.22
wear=np.sin(p[:,0]*583)*np.sin(p[:,2]*769)*np.sin(p[:,1]*677)*.012
c=np.repeat((panel+wear)[:,None],3,axis=1)
c[p[:,1]<-.018]*=.69
c[p[:,2]>.33]*=np.array([.58,.61,.65])
c[(abs(p[:,0])<.010)&(p[:,2]<.23)&(p[:,2]>-.32)]*=.55
c[(abs(p[:,0])>.18)&(abs(p[:,0])<.295)&(p[:,2]<.015)&(abs(n[:,2])>.7)&(p[:,1]<.016)]*=.28
c[(p[:,1]>.04)&(abs(n[:,1])<.35)]*=.64
rgba=np.ones((len(mesh.loops),4),dtype=np.float32);rgba[:,:3]=np.clip(c,0,1)
attr=mesh.color_attributes.new(name='COLOR_0',type='FLOAT_COLOR',domain='CORNER')
attr.data.foreach_set('color',rgba.ravel())
mesh.color_attributes.active_color=attr

def srgb(h):
    q=np.array([int(h[i:i+2],16)/255 for i in (0,2,4)])
    return tuple(np.where(q<=.04045,q/12.92,((q+.055)/1.055)**2.4))
def material(name,color,metal,rough,emission=None,strength=0,vertex=False):
    m=bpy.data.materials.new(name);m.use_nodes=True
    rgb=srgb(color);m.diffuse_color=(*rgb,1)
    bs=m.node_tree.nodes.get('Principled BSDF')
    bs.inputs['Base Color'].default_value=(*rgb,1)
    bs.inputs['Metallic'].default_value=metal;bs.inputs['Roughness'].default_value=rough
    if emission:
        bs.inputs['Emission Color'].default_value=(*srgb(emission),1)
        bs.inputs['Emission Strength'].default_value=strength
    if vertex:
        vc=m.node_tree.nodes.new('ShaderNodeVertexColor');vc.layer_name='COLOR_0'
        mix=m.node_tree.nodes.new('ShaderNodeMixRGB');mix.blend_type='MULTIPLY';mix.inputs[0].default_value=1
        mix.inputs[2].default_value=(*rgb,1)
        m.node_tree.links.new(vc.outputs['Color'],mix.inputs[1]);m.node_tree.links.new(mix.outputs[0],bs.inputs['Base Color'])
    return m
mats=[material('Daedalus_Armor','929899',.48,.68,vertex=True),material('Daedalus_Recess','1b2226',.65,.59),material('Daedalus_Trim','666d70',.6,.54),material('Daedalus_Glass','182b32',.3,.23,'93bfca',1.3)]
for m in mats:mesh.materials.append(m)
for poly in mesh.polygons:poly.material_index=0
# Geometry fittings reproduce the web livery's surface ray casts and dimensions.
bvh=BVHTree.FromObject(ship,bpy.context.evaluated_depsgraph_get())
verts=[];faces=[];mi=[]
def cv(p): return Vector((-p[2],-p[0],p[1]))
def ray(o,d):
    hit,normal,index,dist=bvh.ray_cast(cv(o),cv(d),2000)
    if hit is None:return None
    return hit,Vector((-normal.y,normal.z,-normal.x))
def cube(center, dims, mat):
    # dims in source XYZ converted to engine XYZ
    d=Vector((dims[2],dims[0],dims[1]))*.5;start=len(verts)
    verts.extend([tuple(center+Vector((sx*d.x,sy*d.y,sz*d.z))) for sx,sy,sz in [(-1,-1,-1),(-1,-1,1),(-1,1,-1),(-1,1,1),(1,-1,-1),(1,-1,1),(1,1,-1),(1,1,1)]])
    for f in [(0,4,6,2),(1,3,7,5),(0,1,5,4),(2,6,7,3),(0,2,3,1),(4,5,7,6)]:faces.append(tuple(start+i for i in f));mi.append(mat)
def plate(x,z,w,d,mat,lift=.00035):
    h=ray((x*600,600,z*600),(0,-1,0))
    if h is None or h[1].y<.55:return False
    cube(h[0]+Vector((0,0,lift*600)),(w*600,.00045*600,d*600),mat);return True
stats={'launchHatches':0,'ventSlats':0,'windows':0}
for side in [-1,1]:
    for row in range(7):
        for col in range(2):
            x=side*(.054+col*.018);z=-.19+row*.031
            if plate(x,z,.013,.019,1):plate(x,z,.0095,.015,2,.0008);stats['launchHatches']+=1
    for row in range(18):
        if plate(side*.247,.08+row*.008,.054,.003,1):stats['ventSlats']+=1
    for row in range(24):
        for y in [.046,.052]:
            h=ray((side*.16*600,y*600,(-.015+row*.006)*600),(-side,0,0))
            if h is None or abs(h[0].y)>.10*600 or abs(h[1].x)<.4:continue
            cube(h[0]+cv((side*.0004*600,0,0)),(.00065*600,.00135*600,.0028*600),3);stats['windows']+=1
    for i in range(9):
        h=ray(((side*.245+(i-4)*.004)*600,-.006*600,-600),(0,0,1))
        if h:
            cube(h[0]+cv((0,0,-.0004*600)),(.0018*600,.001*600,.0007*600),3);stats['windows']+=1
fmesh=bpy.data.meshes.new('RuntimeLiveryFittings');fmesh.from_pydata(verts,[],faces);fmesh.update()
for m in mats:fmesh.materials.append(m)
for poly,idx in zip(fmesh.polygons,mi):poly.material_index=idx
fa=fmesh.color_attributes.new(name='COLOR_0',type='FLOAT_COLOR',domain='CORNER')
fa.data.foreach_set('color',np.ones(len(fmesh.loops)*4,dtype=np.float32))
fmesh.color_attributes.active_color=fa
fo=bpy.data.objects.new('RuntimeLiveryFittings',fmesh);scene.collection.objects.link(fo)
bpy.ops.object.select_all(action='DESELECT');ship.select_set(True);fo.select_set(True);bpy.context.view_layer.objects.active=ship;bpy.ops.object.join()
# Triangulate added cube faces; original triangles are unchanged.
mod=ship.modifiers.new('Triangulate fittings','TRIANGULATE');bpy.ops.object.modifier_apply(modifier=mod.name)
ship['source_creator']='Astrofossil';ship['source_url']='https://www.thingiverse.com/thing:2256025'
ship['license']='CC BY-NC 4.0';ship['forward_axis']='+X';ship['up_axis']='+Z';ship['length_metres']=600.0
# GLTF exporter only recognizes direct Principled constant base + vertex colour.
# Keep rich Blender node preview, temporarily constant base for export.
blend=OUT/'Daedalus.blend';glb=OUT/'Daedalus.glb'
bpy.ops.wm.save_as_mainfile(filepath=str(blend),compress=True)
armor=mats[0];bs=armor.node_tree.nodes.get('Principled BSDF')
for link in list(bs.inputs['Base Color'].links):armor.node_tree.links.remove(link)
bs.inputs['Base Color'].default_value=(*srgb('929899'),1)
bpy.ops.export_scene.gltf(filepath=str(glb),export_format='GLB',use_selection=True,export_vertex_color='ACTIVE',export_materials='EXPORT',export_extras=True)
bpy.ops.wm.open_mainfile(filepath=str(blend))
ship=bpy.data.objects['SM_Daedalus']
assert abs(ship.dimensions.x-600)<.01
assert len([o for o in bpy.context.scene.objects if o.type=='MESH'])==1
report={'sourceSHA256':hashlib.sha256(SOURCE.read_bytes()).hexdigest(),'blendSHA256':hashlib.sha256(blend.read_bytes()).hexdigest(),'glbSHA256':hashlib.sha256(glb.read_bytes()).hexdigest(),'dimensionsMetres':list(ship.dimensions),'originalTriangles':222330,'finalTriangles':len(ship.data.polygons),'vertices':len(ship.data.vertices),'materialSlots':[m.name for m in ship.data.materials],'fittings':stats,'coordinateMapping':'source (x,y,z) -> Unreal/Blender (-z,-x,y)','checkedReopen':True}
(OUT/'asset-metadata.json').write_text(json.dumps(report,indent=2)+'\n')
# Preview scene belongs only to ignored outputs; source remains one editable mesh.
scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=24
scene.world=bpy.data.worlds.new('PreviewWorld');scene.world.use_nodes=True
scene.world.node_tree.nodes['Background'].inputs[0].default_value=(.035,.045,.065,1)
scene.world.node_tree.nodes['Background'].inputs[1].default_value=.35
for name,pos,power,size,color in [('Key',(300,-450,600),18000000,500,(.80,.89,1)),('Fill',(-250,350,300),10000000,500,(.55,.67,1)),('Rim',(-350,-50,100),6000000,250,(1,.80,.60))]:
    data=bpy.data.lights.new(name,'AREA');data.energy=power;data.shape='DISK';data.size=size;data.color=color
    light=bpy.data.objects.new(name,data);scene.collection.objects.link(light);light.location=pos;light.rotation_euler=(-light.location).to_track_quat('-Z','Y').to_euler()
camdata=bpy.data.cameras.new('PreviewCamera');cam=bpy.data.objects.new('PreviewCamera',camdata);scene.collection.objects.link(cam)
cam.location=(650,-740,620);cam.rotation_euler=(-cam.location).to_track_quat('-Z','Y').to_euler();camdata.type='ORTHO';camdata.ortho_scale=850;camdata.clip_end=5000;scene.camera=cam
scene.render.resolution_x=1400;scene.render.resolution_y=1050;scene.render.resolution_percentage=100
scene.render.filepath=str(TMP/'daedalus-preview.png');scene.view_settings.view_transform='AgX'
bpy.ops.render.render(write_still=True)
print('DAEDALUS_SOURCE_PASS',json.dumps(report))

# Original smooth 2 m diameter sphere, shared by Earth/Sun and atmosphere.
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.mesh.primitive_uv_sphere_add(segments=128, ring_count=64, radius=1)
sphere=bpy.context.object
sphere.name='SM_SolarSphere'
for face in sphere.data.polygons: face.use_smooth=True
space=ROOT/'Art/Space'
space.mkdir(parents=True,exist_ok=True)
bpy.ops.wm.save_as_mainfile(filepath=str(space/'SolarSphere.blend'),compress=True)
bpy.ops.export_scene.gltf(filepath=str(space/'SolarSphere.glb'),export_format='GLB',use_selection=True)
print('SOLAR_SPHERE_PASS')

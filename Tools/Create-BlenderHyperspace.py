"""Blender 5.2: editable, baked-keyframe spatial rupture rehearsal (task 0030).

Run with Blender -b --python this_file -- --root REPO [--still FRAME].
All geometry/materials are original; existing ship GLBs are read-only context.
No external texture or simulation cache is required by the packed .blend.
"""
import argparse
import json
import math
import random
import sys
from pathlib import Path

import bpy
from mathutils import Vector, noise

parser = argparse.ArgumentParser()
parser.add_argument('--root', type=Path, required=True)
parser.add_argument('--still', type=int)
args = parser.parse_args(sys.argv[sys.argv.index('--') + 1:])
root = args.root.resolve()
output = root / 'Art/Effects/Hyperspace/Blender'
renders = root / '.local/hyper/blender0030'
output.mkdir(parents=True, exist_ok=True)
renders.mkdir(parents=True, exist_ok=True)
bpy.ops.wm.read_factory_settings(use_empty=True)
scene = bpy.context.scene
scene.render.engine = 'BLENDER_EEVEE'
scene.render.resolution_x = 3840
scene.render.resolution_y = 2160
scene.render.resolution_percentage = 100
scene.render.fps = 24
scene.frame_start = 1
scene.frame_end = 108
scene.render.image_settings.file_format = 'PNG'
scene.render.image_settings.color_mode = 'RGB'
scene.render.image_settings.color_depth = '8'
scene.render.film_transparent = False
scene.render.use_motion_blur = False
scene.eevee.taa_render_samples = 48
scene.eevee.volumetric_tile_size = '2'
scene.eevee.volumetric_samples = 64
scene.eevee.volumetric_start = .1
scene.eevee.volumetric_end = 50
scene.view_settings.view_transform = 'AgX'
scene.view_settings.look = 'AgX - Medium High Contrast'
scene.world = bpy.data.worlds.new('Black space')
scene.world.use_nodes = True
scene.world.node_tree.nodes['Background'].inputs['Color'].default_value = (.00003, .00004, .00006, 1)
scene.world.node_tree.nodes['Background'].inputs['Strength'].default_value = .1
scene['task'] = '0030; appearance pending user review; offline Blender rehearsal'
scene['scale'] = '1 Blender unit represents 200 metres; ship length 3 units'
scene['timeline'] = '24 fps: tear 12-32, hold 32-63, collapse 64-88; ship bow-first'

def node(tree, kind, name=None):
    n = tree.nodes.new(kind)
    if name:
        n.name = name
        n.label = name
    return n

def mathnode(tree, operation, a, b=None):
    n = node(tree, 'ShaderNodeMath')
    n.operation = operation
    for i, val in enumerate((a, b)):
        if val is None:
            continue
        if isinstance(val, (float, int)):
            n.inputs[i].default_value = val
        else:
            tree.links.new(val, n.inputs[i])
    return n.outputs[0]

def ramp(tree, source, stops):
    n = node(tree, 'ShaderNodeValToRGB')
    tree.links.new(source, n.inputs[0])
    n.color_ramp.interpolation = 'EASE'
    for i, (position, colour) in enumerate(stops):
        e = n.color_ramp.elements[i] if i < 2 else n.color_ramp.elements.new(position)
        e.position = position
        e.color = colour
    return n.outputs[0]

def keys(obj, path, values):
    for frame, value in values:
        setattr(obj, path, value)
        obj.keyframe_insert(data_path=path, frame=frame)

def material(name):
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    m.node_tree.nodes.clear()
    return m, m.node_tree, node(m.node_tree, 'ShaderNodeOutputMaterial')

def animate_noise(n, first=0, last=1.5):
    n.noise_dimensions = '4D'
    n.inputs['W'].default_value = first
    n.inputs['W'].keyframe_insert('default_value', frame=1)
    n.inputs['W'].default_value = last
    n.inputs['W'].keyframe_insert('default_value', frame=108)

# Surface sheets carry true folds, while the shader adds sub-fold filaments.
sheet, tree, out = material('Rupture | thin luminous folded membrane')
tex = node(tree, 'ShaderNodeTexCoord')
mapping = node(tree, 'ShaderNodeVectorMath'); mapping.operation = 'MULTIPLY'
tree.links.new(tex.outputs['UV'], mapping.inputs[0])
mapping.inputs[1].default_value = (3.5, 1.0, 1.0)
n = node(tree, 'ShaderNodeTexNoise', 'Advect microstructure')
tree.links.new(mapping.outputs[0], n.inputs['Vector'])
n.inputs['Scale'].default_value = 8
n.inputs['Detail'].default_value = 5
n.inputs['Roughness'].default_value = .7
animate_noise(n, 0, .65)
colour = ramp(tree, n.outputs['Fac'], [(.22, (.005,.045,.032,1)),
    (.48, (.035,.26,.16,1)), (.62,(.24,.7,.49,1)), (.76,(.77,1,.9,1))])
emission = node(tree, 'ShaderNodeEmission')
tree.links.new(colour, emission.inputs['Color'])
emission.inputs['Strength'].default_value = 4.5
uv = node(tree, 'ShaderNodeSeparateXYZ'); tree.links.new(tex.outputs['UV'],uv.inputs[0])
across = mathnode(tree, 'SINE', mathnode(tree, 'MULTIPLY', uv.outputs['Y'], math.pi))
across = mathnode(tree, 'POWER', across, .55)
mask = mathnode(tree, 'MULTIPLY', across,
    ramp(tree, n.outputs['Fac'],[(.43,(0,0,0,1)),(.72,(.45,.45,.45,1))]))
mask = mathnode(tree,'MULTIPLY',mask,mathnode(tree,'SUBTRACT',1,uv.outputs['X']))
trans = node(tree,'ShaderNodeBsdfTransparent')
mix = node(tree,'ShaderNodeMixShader')
tree.links.new(mask,mix.inputs[0]);tree.links.new(trans.outputs[0],mix.inputs[1])
tree.links.new(emission.outputs[0],mix.inputs[2]);tree.links.new(mix.outputs[0],out.inputs['Surface'])
sheet.surface_render_method = 'DITHERED'

window = bpy.data.objects.new('WINDOW | animated opening / closure',None)
scene.collection.objects.link(window)
window.location = (2,0,0)
window.rotation_euler.z = .35
keys(window,'scale',[(1,(.001,)*3),(11,(.001,)*3),(17,(.08,.06,.25)),
    (23,(.5,.38,.72)),(32,(1,1,1)),(63,(1,1,1)),
    (73,(.8,.75,.88)),(83,(.15,.1,.35)),(89,(.001,)*3),(108,(.001,)*3)])

rng = random.Random(3030)
def point(i, j, angle, extent, width, seed, phase, thin):
    u, v = i/96, j/12
    radius = .42 + extent*u
    # Independent curls and taper, not a circular ring or uniformly radial rays.
    bend = .13*math.sin(u*7+seed) + .09*math.sin(u*17+seed*2)
    a = angle + bend + (v-.5)*width*(math.sin(math.pi*u)**.65)
    y, z = math.cos(a)*radius, math.sin(a)*radius
    turbulence = noise.noise_vector(Vector((y*1.9+seed,z*1.9,phase)))
    fold = math.sin(v*math.pi*5 + u*7+seed+phase)*.045
    fold += math.sin(v*math.pi*11-u*13+phase)*.018
    x = .16*math.sin(u*5+seed)+(fold + turbulence.x*.12)*math.sin(math.pi*u)
    taper = math.sin(math.pi*u)
    return (x + (seed%3-1)*.11, y+turbulence.y*.09*taper,
            z+turbulence.z*.09*taper)

for k in range(28):
    angle = k*2.399963+rng.uniform(-.16,.16)
    extent = rng.uniform(1.35,3.2) if k<10 else rng.uniform(.95,2.05)
    width = rng.uniform(.2,.42) if k<10 else rng.uniform(.15,.3)
    seed = rng.uniform(1,30)
    verts = [point(i,j,angle,extent,width,seed,0,k>=10) for i in range(97) for j in range(13)]
    faces = [(i*13+j,i*13+j+1,(i+1)*13+j+1,(i+1)*13+j) for i in range(96) for j in range(12)]
    mesh = bpy.data.meshes.new(f'Folded membrane {k:02}')
    mesh.from_pydata(verts,[],faces);mesh.update()
    obj = bpy.data.objects.new(f'Tear {k:02} | separate 3D sheet',mesh)
    scene.collection.objects.link(obj); obj.parent = window
    mesh.materials.append(sheet)
    for p in mesh.polygons: p.use_smooth=True
    uv_layer = mesh.uv_layers.new()
    for poly in mesh.polygons:
        for li in poly.loop_indices:
            vi=mesh.loops[li].vertex_index
            uv_layer.data[li].uv = (vi//13/96,vi%13/12)
    obj.shape_key_add(name='Basis')
    for key_index in range(1,4):
        key = obj.shape_key_add(name=f'Turbulence phase {key_index}')
        coords = [c for i in range(97) for j in range(13)
                  for c in point(i,j,angle,extent,width,seed,key_index*.8,k>=10)]
        key.data.foreach_set('co',coords)
        for frame,value in [(1,0),(key_index*22,1),((key_index+1)*22,0),(108,0)]:
            key.value=value;key.keyframe_insert('value',frame=frame)

# Thin curved fracture lips maintain crisp local contrast above the soft volume.
# Non-branching spline paths avoid the repeated angular spokes of rejected 0029.
lip,lt,lo=material('Rupture | fine mint fracture lips')
le=node(lt,'ShaderNodeEmission');le.inputs['Color'].default_value=(.15,.72,.39,1)
le.inputs['Strength'].default_value=2.5;lt.links.new(le.outputs[0],lo.inputs['Surface'])
for k in range(11):
    a=k*2.399963+rng.uniform(-.2,.2)
    extent=rng.uniform(1.4,2.8)
    seed=rng.uniform(0,40)
    data=bpy.data.curves.new(f'Fracture lip {k:02}','CURVE')
    data.dimensions='3D';data.bevel_depth=.012
    data.bevel_resolution=3;data.resolution_u=2
    spline=data.splines.new('POLY');spline.points.add(128)
    for i,p in enumerate(spline.points):
        u=i/128;r=.25+extent*u
        bend=.24*noise.noise(Vector((u*4,seed,1)))+.08*noise.noise(Vector((u*15,seed,2)))
        v=Vector((u*8,seed,.5))
        wobble=noise.noise_vector(v)*(.17*math.sin(math.pi*u))
        p.co=(.02+wobble.x,(math.cos(a+bend)*r)+wobble.y,(math.sin(a+bend)*r)+wobble.z,1)
        p.radius=max(.003,(1-u)**1.7*(.45+.45*noise.noise(Vector((u*32,seed,3)))))
    obj=bpy.data.objects.new(f'Fine fracture {k:02}',data);scene.collection.objects.link(obj)
    obj.parent=window;data.materials.append(lip)
    keys(data,'bevel_factor_end',[(1,0),(14+k%7,0),(32+k%5,1),(70,1),(87,0)])
    keys(obj,'rotation_euler',[(1,(0,0,0)),(52,(.04*math.sin(seed),0,0)),(108,(-.04*math.sin(seed),0,0))])

# A true emissive 3D volume fills the torn surface, rather than a painted disc.
volume, tree, out = material('Rupture | turbulent emissive interior volume')
tex=node(tree,'ShaderNodeTexCoord')
sub=node(tree,'ShaderNodeVectorMath');sub.operation='SUBTRACT'
tree.links.new(tex.outputs['Generated'],sub.inputs[0]);sub.inputs[1].default_value=(.5,.5,.5)
warp=node(tree,'ShaderNodeTexNoise');warp.inputs['Scale'].default_value=5;warp.inputs['Detail'].default_value=4
tree.links.new(tex.outputs['Generated'],warp.inputs['Vector'])
offset=node(tree,'ShaderNodeVectorMath');offset.operation='SUBTRACT';tree.links.new(warp.outputs['Color'],offset.inputs[0]);offset.inputs[1].default_value=(.5,.5,.5)
stretch=node(tree,'ShaderNodeVectorMath');stretch.operation='SCALE';tree.links.new(offset.outputs[0],stretch.inputs[0]);stretch.inputs['Scale'].default_value=.7
distort=node(tree,'ShaderNodeVectorMath');distort.operation='ADD';tree.links.new(sub.outputs[0],distort.inputs[0]);tree.links.new(stretch.outputs[0],distort.inputs[1])
length=node(tree,'ShaderNodeVectorMath');length.operation='LENGTH';tree.links.new(distort.outputs[0],length.inputs[0])
n=node(tree,'ShaderNodeTexNoise');tree.links.new(tex.outputs['Generated'],n.inputs['Vector'])
n.inputs['Scale'].default_value=32;n.inputs['Detail'].default_value=5;animate_noise(n,1,1.45)
falloff=ramp(tree,length.outputs['Value'],[(.10,(1,1,1,1)),(.43,(0,0,0,1))])
wisps=ramp(tree,n.outputs['Fac'],[(.46,(0,0,0,1)),(.70,(1,1,1,1))])
density=mathnode(tree,'MULTIPLY',falloff, mathnode(tree,'MULTIPLY',wisps,.7))
vol=node(tree,'ShaderNodeVolumePrincipled');tree.links.new(density,vol.inputs['Density'])
vol.inputs['Color'].default_value=(.12,.6,.36,1)
vol.inputs['Emission Color'].default_value=(.45,1,.72,1)
hot=ramp(tree,length.outputs['Value'],[(.03,(1,1,1,1)),(.25,(0,0,0,1))])
lightfield=mathnode(tree,'ADD',mathnode(tree,'MULTIPLY',density,10),mathnode(tree,'MULTIPLY',hot,16))
tree.links.new(lightfield,vol.inputs['Emission Strength'])
tree.links.new(vol.outputs[0],out.inputs['Volume'])
bpy.ops.mesh.primitive_uv_sphere_add(segments=64,ring_count=32,radius=1)
core=bpy.context.object;core.name='Interior | bounded luminous volume'
core.parent=window;core.scale=(.85,2.6,2.8);core.data.materials.append(volume)

# Bright folded central shells obscure the hull only once it crosses the surface.
inner, tree, out=material('Rupture | pearlescent central folds')
tex=node(tree,'ShaderNodeTexNoise');tex.inputs['Scale'].default_value=6
tex.inputs['Detail'].default_value=6;animate_noise(tex,0,.8)
em=node(tree,'ShaderNodeEmission')
tree.links.new(ramp(tree,tex.outputs['Fac'],[(.2,(.035,.13,.09,1)),
    (.55,(.31,.8,.56,1)),(.75,(.72,1,.85,1))]),em.inputs['Color'])
em.inputs['Strength'].default_value=6
tree.links.new(em.outputs[0],out.inputs['Surface'])
bpy.ops.mesh.primitive_uv_sphere_add(segments=160,ring_count=96,radius=1)
shell=bpy.context.object;shell.name='Interior | crumpled bright throat';shell.parent=window
for v in shell.data.vertices:
    original=v.co.copy()
    n=noise.fractal(original*5,1,2,4)
    v.co*=1+n*.15
shell.scale=(.14,.89,1.04)
shell.hide_render=True
shell.data.materials.append(inner)
for p in shell.data.polygons:p.use_smooth=True
keys(shell,'rotation_euler',[(1,(0,0,0)),(108,(.12,.07,.15))])

# Import reviewed ship unchanged; all transforms below apply only to this scene.
ship=bpy.data.objects.new('DAEDALUS | presentation motion only',None)
scene.collection.objects.link(ship)
ship_objects=[]
for filename in ['Daedalus.glb','DaedalusAddOns.glb','DaedalusLights.glb','DaedalusEngineGlow.glb']:
    before=set(bpy.data.objects)
    bpy.ops.import_scene.gltf(filepath=str(root/'Art/Ships/Daedalus'/filename))
    added=set(bpy.data.objects)-before
    for obj in added:
        if obj.parent is None:obj.parent=ship
        if obj.type=='LIGHT':obj.data.energy*=.005**2
        ship_objects.append(obj)
ship.scale=(.005,)*3
keys(ship,'location',[(1,(-7,0,0)),(28,(-6.8,0,0)),(42,(-5.8,0,0)),
    (54,(-2.2,0,0)),(66,(3.7,0,0)),(76,(10,0,0)),(108,(30,0,0))])
# World-space half-space clipping on scene-local material copies gives a clean
# bow-first crossing. It never edits source GLBs or game materials.
copies={}
for obj in ship_objects:
    if obj.type!='MESH':continue
    for slot in obj.material_slots:
        source=slot.material
        if not source or not source.use_nodes:continue
        if source not in copies:
            m=source.copy();m.name=source.name+' | rehearsal clip'
            tree=m.node_tree
            for shader in tree.nodes:
                if shader.type=='BSDF_PRINCIPLED' and 'Emission Strength' in shader.inputs:
                    strength=shader.inputs['Emission Strength']
                    for link in list(strength.links):tree.links.remove(link)
                    strength.default_value=.8 if any(word in source.name.lower() for word in ['light','glow','inner']) else 0
            out=next(n for n in tree.nodes if n.type=='OUTPUT_MATERIAL' and n.is_active_output)
            if not out.inputs['Surface'].links:continue
            old=out.inputs['Surface'].links[0].from_socket
            geo=node(tree,'ShaderNodeNewGeometry');distance=node(tree,'ShaderNodeVectorMath')
            distance.operation='DOT_PRODUCT'
            tree.links.new(geo.outputs['Position'],distance.inputs[0])
            distance.inputs[1].default_value=(math.cos(.35),math.sin(.35),0)
            passed=mathnode(tree,'GREATER_THAN',distance.outputs['Value'],2*math.cos(.35))
            mix=node(tree,'ShaderNodeMixShader');trans=node(tree,'ShaderNodeBsdfTransparent')
            tree.links.new(passed,mix.inputs[0]);tree.links.new(old,mix.inputs[1])
            tree.links.new(trans.outputs[0],mix.inputs[2]);tree.links.new(mix.outputs[0],out.inputs['Surface'])
            m.surface_render_method='DITHERED';copies[source]=m
        slot.material=copies[source]

def light(name,kind,location,power,colour,size=1):
    data=bpy.data.lights.new(name,kind);data.energy=power;data.color=colour
    obj=bpy.data.objects.new(name,data);scene.collection.objects.link(obj);obj.location=location
    if kind=='AREA':data.shape='DISK';data.size=size
    obj.rotation_euler=(Vector((-2,0,0))-obj.location).to_track_quat('-Z','Y').to_euler()
    return obj
light('Cold distant key','AREA',(-5,-4,10),1100,(.76,.85,1),8)
light('Soft hull separation','AREA',(-8,7,2),600,(.16,.28,.5),8)
local=light('Window illuminates approaching hull','POINT',(1.5,0,0),0,(.2,1,.57))
local.data.shadow_soft_size=1.5
keys(local.data,'energy',[(1,0),(12,0),(32,600),(65,600),(88,0),(108,0)])

# Sharp world-space stars, no background photograph and no depth-of-field blur.
star,tree,out=material('Distant star points')
em=node(tree,'ShaderNodeEmission');em.inputs['Color'].default_value=(.65,.8,1,1)
em.inputs['Strength'].default_value=1.4;tree.links.new(em.outputs[0],out.inputs['Surface'])
verts=[];faces=[]
for i in range(450):
    x,y,z=rng.uniform(-32,32),35,rng.uniform(-20,24)
    r=rng.uniform(.009,.027)
    base=len(verts);verts.extend([(x-r,y,z-r),(x+r,y,z-r),(x,y,z+r)])
    faces.append((base,base+1,base+2))
mesh=bpy.data.meshes.new('Distant star geometry');mesh.from_pydata(verts,[],faces)
obj=bpy.data.objects.new('Sparse sharp starfield',mesh);scene.collection.objects.link(obj);mesh.materials.append(star)

cam_data=bpy.data.cameras.new('Cinematic exterior 4K')
cam=bpy.data.objects.new('Cinematic exterior 4K',cam_data);scene.collection.objects.link(cam)
cam.location=(-14,-18,8)
cam.rotation_euler=(Vector((-.7,0,0))-cam.location).to_track_quat('-Z','Y').to_euler()
cam_data.lens=47;cam_data.clip_end=250;cam_data.dof.use_dof=False;scene.camera=cam

# Restrained optical glare added to real bright surfaces; no scene-wide blur.
comp=bpy.data.node_groups.new('Rupture optical finish','CompositorNodeTree')
comp.interface.new_socket(name='Image',in_out='OUTPUT',socket_type='NodeSocketColor')
scene.compositing_node_group=comp
layers=node(comp,'CompositorNodeRLayers')
glare=node(comp,'CompositorNodeGlare')
glare.inputs['Type'].default_value='Fog Glow'
glare.inputs['Quality'].default_value='High'
glare.inputs['Threshold'].default_value=2.0
glare.inputs['Strength'].default_value=.15
glare.inputs['Size'].default_value=.25
comp.links.new(layers.outputs['Image'],glare.inputs['Image'])
dest=node(comp,'NodeGroupOutput');comp.links.new(glare.outputs['Image'],dest.inputs['Image'])

scene.render.filepath=str(renders/'frames'/'')+'/'
(renders/'frames').mkdir(exist_ok=True)
scene.frame_set(44)
bpy.ops.file.pack_all()
bpy.ops.wm.save_as_mainfile(filepath=str(output/'Daedalus_GreenRupture.blend'),compress=True)
metadata={'blender':bpy.app.version_string,'renderEngine':scene.render.engine,
    'resolution':[3840,2160],'fps':24,'frames':[1,108],
    'membranes':28,'animatedShapeKeys':84,'selfContainedTextures':True,
    'reference':'Tasks/0029/REFERENCE_REVIEW.md; user-supplied series excerpts',
    'gameIntegration':False,'userAppearanceAccepted':False}
(output/'scene.json').write_text(json.dumps(metadata,indent=2)+'\n',encoding='utf-8')
if args.still is not None:
    scene.frame_set(args.still)
    scene.render.filepath=str(renders/f'review-{args.still:04}.png')
    bpy.ops.render.render(write_still=True)
print('BLENDER_HYPERSPACE_SAVED',str(output/'Daedalus_GreenRupture.blend'))

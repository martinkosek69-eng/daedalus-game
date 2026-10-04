"""Blender background: preserve saved Aurora look, bake PBR maps and export GLB.

Run with the saved Art/Ships/Aurora/Aurora_Class.blend. Source is never changed.
Original procedural paint/materials are evaluated by Cycles, not guessed.
"""
import bpy, json, hashlib, bmesh, math, os
from pathlib import Path
from mathutils import Vector
root=Path(__file__).resolve().parents[1]; out=root/'Art/Ships/Aurora'
tex=out/'Textures'; tex.mkdir(exist_ok=True)
scene=bpy.context.scene
cache=root/'.local/aurora/OptixCache'; cache.mkdir(parents=True,exist_ok=True)
os.environ['OPTIX_CACHE_PATH']=str(cache)
scene.render.use_file_extension=True
scene.render.image_settings.file_format='PNG'
scene.render.engine='CYCLES'; scene.cycles.samples=1
scene.render.bake.use_clear=True; scene.render.bake.margin=12
scene.render.bake.target='IMAGE_TEXTURES'
prefs=bpy.context.preferences.addons['cycles'].preferences
try:
    prefs.compute_device_type='OPTIX'; prefs.get_devices()
    for d in prefs.devices: d.use=d.type!='CPU'
    if any(d.use for d in prefs.devices): scene.cycles.device='GPU'
except Exception: pass
ship=bpy.data.objects['SM_Aurora']; orb=bpy.data.objects['GEO_BlueOrb']
for img in bpy.data.images:
    if img.source=='FILE':
        img.filepath=str(tex/Path(bpy.path.abspath(img.filepath).replace('\\','/')).name)
        img.reload()
bm=bmesh.new(); bm.from_mesh(ship.data)
audit={'vertices':len(bm.verts),'faces':len(bm.faces),
       'nonManifoldEdges':sum(not e.is_manifold for e in bm.edges),
       'degenerateFaces':sum(f.calc_area()<1e-9 for f in bm.faces)}
bm.free()
# STL-derived collinear faces carry no surface area. Dissolve their zero-length
# edges before unwrapping, preserving the visible shape and per-face paint.
bm=bmesh.new(); bm.from_mesh(ship.data)
bmesh.ops.dissolve_degenerate(bm,dist=1e-6,edges=list(bm.edges))
audit['cleanedDegenerateFaces']=sum(f.calc_area()<1e-9 for f in bm.faces)
audit['cleanedNonManifoldEdges']=sum(not e.is_manifold for e in bm.edges)
bm.to_mesh(ship.data); bm.free(); ship.data.update()
bpy.ops.object.select_all(action='DESELECT'); ship.select_set(True)
bpy.context.view_layer.objects.active=ship
if not ship.data.uv_layers:
    ship.data.uv_layers.new(name='GameAtlas')
    bpy.ops.object.mode_set(mode='EDIT'); bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project(angle_limit=math.radians(66),island_margin=.001,area_weight=1,correct_aspect=True)
    bpy.ops.object.mode_set(mode='OBJECT')
materials=list(ship.data.materials)
original={}
for m in materials:
    bs=next(n for n in m.node_tree.nodes if n.type=='BSDF_PRINCIPLED')
    original[m.name]={}
    for key in ['Base Color','Metallic','Roughness','Emission Color','Emission Strength']:
        s=bs.inputs[key]
        original[m.name][key]=(s.links[0].from_socket if s.is_linked else tuple(s.default_value) if hasattr(s.default_value,'__len__') else float(s.default_value))
    original[m.name]['output']=next(n for n in m.node_tree.nodes if n.type=='OUTPUT_MATERIAL')
    original[m.name]['bsdf']=bs
def input_link(nt,src,dest):
    if hasattr(src,'node'): nt.links.new(src,dest)
    else: dest.default_value=src
maps={}
for role,size in [('BaseColor',8192),('ORM',4096),('Emission',4096),('Normal',4096)]:
    img=bpy.data.images.new('T_Aurora_'+role,width=size,height=size,alpha=False)
    img.colorspace_settings.name='sRGB' if role in ('BaseColor','Emission') else 'Non-Color'
    for m in materials:
        nt=m.node_tree; src=original[m.name]
        for n in nt.nodes: n.select=False
        target=nt.nodes.new('ShaderNodeTexImage'); target.image=img; target.select=True; nt.nodes.active=target
        if role=='Normal': nt.links.new(src['bsdf'].outputs['BSDF'],src['output'].inputs['Surface']); continue
        em=nt.nodes.new('ShaderNodeEmission')
        if role=='BaseColor': input_link(nt,src['Base Color'],em.inputs['Color'])
        elif role=='ORM':
            comb=nt.nodes.new('ShaderNodeCombineXYZ'); comb.inputs['X'].default_value=1
            input_link(nt,src['Roughness'],comb.inputs['Y']); input_link(nt,src['Metallic'],comb.inputs['Z'])
            nt.links.new(comb.outputs[0],em.inputs['Color'])
        else:
            input_link(nt,src['Emission Color'],em.inputs['Color']); input_link(nt,src['Emission Strength'],em.inputs['Strength'])
        nt.links.new(em.outputs[0],src['output'].inputs['Surface'])
    print('AURORA_BAKE_START',role,size,flush=True)
    bpy.ops.object.bake(type='NORMAL' if role=='Normal' else 'EMIT')
    img.filepath_raw=str(tex/(img.name+'.png')); img.file_format='PNG'; img.save(); maps[role]=img
    print('AURORA_BAKE_SAVED',role,flush=True)
# Replace only the temporary export scene material graph with portable glTF PBR.
for m in materials:
    nt=m.node_tree; nt.nodes.clear(); bs=nt.nodes.new('ShaderNodeBsdfPrincipled'); o=nt.nodes.new('ShaderNodeOutputMaterial')
    nt.links.new(bs.outputs[0],o.inputs['Surface'])
    if 'DomeGlow' in m.name:
        # The separately joined orb is not part of the hull atlas bake.
        bs.inputs['Base Color'].default_value=original[m.name]['Base Color']
        bs.inputs['Emission Color'].default_value=original[m.name]['Emission Color']
        bs.inputs['Emission Strength'].default_value=original[m.name]['Emission Strength']
        bs.inputs['Roughness'].default_value=.15
        continue
    nodes={}
    for role,img in maps.items(): n=nt.nodes.new('ShaderNodeTexImage'); n.image=img; nodes[role]=n
    nt.links.new(nodes['BaseColor'].outputs['Color'],bs.inputs['Base Color'])
    split=nt.nodes.new('ShaderNodeSeparateColor'); nt.links.new(nodes['ORM'].outputs['Color'],split.inputs[0])
    nt.links.new(split.outputs['Green'],bs.inputs['Roughness']); nt.links.new(split.outputs['Blue'],bs.inputs['Metallic'])
    normal=nt.nodes.new('ShaderNodeNormalMap'); nt.links.new(nodes['Normal'].outputs['Color'],normal.inputs['Color'])
    nt.links.new(normal.outputs[0],bs.inputs['Normal'])
    nt.links.new(nodes['Emission'].outputs['Color'],bs.inputs['Emission Color']); bs.inputs['Emission Strength'].default_value=1
    if 'Glass' in m.name: m.surface_render_method='DITHERED' # Opaque glowing panes in the lab; no refraction artefacts.
bpy.ops.object.select_all(action='DESELECT'); ship.select_set(True); orb.select_set(True)
bpy.context.view_layer.objects.active=ship
# Bake orb's local transform into the combined single imported hull.
bpy.ops.object.join(); ship.name='SM_Aurora'
bpy.ops.wm.save_as_mainfile(filepath=str(root/'.local/aurora/Aurora_Game.blend'))
bpy.ops.export_scene.gltf(filepath=str(out/'Aurora.glb'),export_format='GLB',use_selection=True,
    export_apply=True,export_yup=True,export_normals=True,export_texcoords=True,export_materials='EXPORT')
ship.data.calc_loop_triangles()
audit.update({'triangles':len(ship.data.loop_triangles),'dimensionsMetres':list(ship.dimensions),
    'uvLayers':list(ship.data.uv_layers.keys()),'materials':[m.name for m in ship.data.materials],
    'sourceSHA256':hashlib.sha256((out/'Aurora_Class.blend').read_bytes()).hexdigest(),
    'gameSHA256':hashlib.sha256((out/'Aurora.glb').read_bytes()).hexdigest(),
    'sourceNotModified':True,'maps':{r:{'size':list(i.size),'file':Path(i.filepath_raw).name} for r,i in maps.items()}})
(out/'GAME_ASSET.json').write_text(json.dumps(audit,indent=2)+'\n')
print('AURORA_EXPORT_PASS',json.dumps(audit),flush=True)

"""Read the saved source scene; isolated Blender background inspection only."""
import bpy, json
from pathlib import Path
root = Path(__file__).resolve().parents[1]
report = {'blender': bpy.app.version_string, 'objects': [], 'materials': []}
for o in bpy.data.objects:
    if o.type != 'MESH': continue
    report['objects'].append({'name': o.name, 'vertices': len(o.data.vertices),
        'polygons': len(o.data.polygons), 'dimensions': list(o.dimensions),
        'matrix': [list(r) for r in o.matrix_world], 'uvs': list(o.data.uv_layers.keys()),
        'attributes': [(a.name,a.domain,a.data_type) for a in o.data.attributes],
        'modifiers': [(m.name,m.type) for m in o.modifiers]})
for m in bpy.data.materials:
    report['materials'].append({'name':m.name, 'nodes': [
        {'name':n.name,'type':n.bl_idname,
         'image':getattr(getattr(n,'image',None),'filepath',None),
         'inputs':{s.name:list(s.default_value) if hasattr(s.default_value,'__len__') and not isinstance(s.default_value,str) else s.default_value
                   for s in n.inputs if hasattr(s,'default_value')},
         'attribute':getattr(n,'attribute_name',None),
         'links':[(l.from_node.name,l.from_socket.name,l.to_socket.name) for l in n.inputs for l in l.links]}
        for n in m.node_tree.nodes] if m.use_nodes else []})
p=root/'.local/aurora/scene-inspection.json'; p.parent.mkdir(parents=True,exist_ok=True)
p.write_text(json.dumps(report,indent=2,default=str)); print('AURORA_INSPECTED',p)

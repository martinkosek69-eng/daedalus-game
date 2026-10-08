"""Latest 0008 multi-piece ship import and explicit animation/light manifest.

Interchange bakes mesh nodes into the hull frame. Light coordinates are instead
resolved from glTF node transforms; conversion to UE cm happens exactly once.
"""
import hashlib
import json
import struct
from pathlib import Path


def document(path):
    raw = path.read_bytes()
    assert raw[:4] == b'glTF', path
    return json.loads(raw[20:20 + struct.unpack_from('<I', raw, 12)[0]])


def prepare(root, unreal, tools, source_materials, effect_materials):
    root = Path(root)
    source_dir = root / 'Art/Ships/Daedalus'
    manifest = {'version': 2, 'meshes': [], 'doors': [], 'pointLights': [], 'sources': {}}

    def pieces(filename, destination):
        source = source_dir / filename
        doc = document(source)
        manifest['sources'][filename] = hashlib.sha256(source.read_bytes()).hexdigest()
        task = unreal.AssetImportTask()
        task.filename = str(source)
        task.destination_path = destination
        task.automated = True
        task.replace_existing = True
        task.save = True
        tools.import_asset_tasks([task])
        meshes = [unreal.load_asset(p) for p in task.imported_object_paths]
        meshes = [m for m in meshes if isinstance(m, unreal.StaticMesh)]
        expected = {n['name']: n for n in doc['nodes'] if 'mesh' in n}
        assert len(meshes) == len(expected), (filename, task.imported_object_paths)
        found = {}
        for mesh in meshes:
            # A one-mesh scene uses its filename as asset name in Interchange.
            name = next(iter(expected)) if len(expected) == 1 else mesh.get_name()
            assert name in expected and name not in found, (filename, name, expected.keys())
            found[name] = mesh
            nanite = mesh.get_editor_property('nanite_settings')
            nanite.set_editor_property('enabled', False)
            mesh.set_editor_property('nanite_settings', nanite)
        return doc, found, expected

    def lights(doc, kind):
        definitions = doc.get('extensions', {}).get('KHR_lights_punctual', {}).get('lights', [])
        identity = [[int(i == j) for j in range(4)] for i in range(4)]
        def visit(index, parent=identity):
            node = doc['nodes'][index]
            if 'matrix' in node:
                local = [[node['matrix'][j * 4 + i] for j in range(4)] for i in range(4)]
            else:
                x, y, z, w = node.get('rotation', [0, 0, 0, 1])
                sx, sy, sz = node.get('scale', [1, 1, 1])
                tx, ty, tz = node.get('translation', [0, 0, 0])
                local = [[(1-2*(y*y+z*z))*sx, 2*(x*y-z*w)*sy, 2*(x*z+y*w)*sz, tx],
                         [2*(x*y+z*w)*sx, (1-2*(x*x+z*z))*sy, 2*(y*z-x*w)*sz, ty],
                         [2*(x*z-y*w)*sx, 2*(y*z+x*w)*sy, (1-2*(x*x+y*y))*sz, tz], [0, 0, 0, 1]]
            world = [[sum(parent[i][k] * local[k][j] for k in range(4)) for j in range(4)] for i in range(4)]
            position = [world[i][3] for i in range(3)]
            ref = node.get('extensions', {}).get('KHR_lights_punctual', {}).get('light')
            if ref is not None:
                light = definitions[ref]
                assert light['type'] == 'point' and light['range'] > 0
                manifest['pointLights'].append({
                    'name': node['name'], 'kind': kind,
                    'positionCentimetres': [position[0] * 100, position[2] * 100, position[1] * 100],
                    'colourLinear': light.get('color', [1, 1, 1]),
                    'candela': light.get('intensity', 1), 'radiusCentimetres': light['range'] * 100})
            for child in node.get('children', []):
                visit(child, world)
        for index in doc['scenes'][doc.get('scene', 0)]['nodes']:
            visit(index)

    doc, meshes, nodes = pieces('DaedalusAddOns.glb', '/Game/Ships/Daedalus/Details')
    for name, mesh in meshes.items():
        source_materials.assign_materials(root, unreal, tools, unreal.MaterialEditingLibrary, mesh, 'DaedalusAddOns.glb')
        extras = nodes[name].get('extras', {})
        if 'doorHalf' in extras:
            axis = extras['openAxis']  # Blender source coordinates, not glTF.
            distance = extras['openDistanceMetres']
            assert extras['doorHalf'] in ('upper', 'lower') and distance > 0
            manifest['doors'].append({'mesh': mesh.get_path_name(), 'bay': extras['bay'],
                'half': extras['doorHalf'], 'openOffsetCentimetres': [axis[0] * distance * 100, -axis[1] * distance * 100, axis[2] * distance * 100]})
        else:
            manifest['meshes'].append(mesh.get_path_name())
    assert len(manifest['doors']) == 4 and len(manifest['meshes']) == 1

    doc, meshes, _ = pieces('DaedalusLights.glb', '/Game/Ships/Daedalus/Effects')
    assert set(meshes) == {'Daedalus_DetailLights', 'Daedalus_WindowLights'}
    for name, mesh in meshes.items():
        effect_materials(mesh, 'DaedalusLights.glb', True)
        if name == 'Daedalus_WindowLights':
            manifest['windowMesh'] = mesh.get_path_name()
        else:
            manifest['meshes'].append(mesh.get_path_name())
    lights(doc, 'hangar')

    doc, meshes, nodes = pieces('DaedalusBeacons.glb', '/Game/Ships/Daedalus/Effects')
    assert len(meshes) == 1
    name, mesh = next(iter(meshes.items()))
    source_materials.assign_materials(root, unreal, tools, unreal.MaterialEditingLibrary, mesh, 'DaedalusBeacons.glb', beacon=True)
    extras = nodes[name]['extras']
    manifest['beacon'] = {'mesh': mesh.get_path_name(), 'periodSeconds': extras['blinkPeriodSeconds'],
        'onSeconds': extras['onSeconds'], 'emissionOn': extras['emissionOn'], 'emissionOff': extras['emissionOff']}
    lights(document(source_dir / 'DaedalusEngineGlow.glb'), 'engine')
    assert len(manifest['pointLights']) == 10
    assert len(manifest['meshes']) == 2  # No obsolete duplicate turret barrels.
    for filename in ('Daedalus.glb', 'DaedalusEngineGlow.glb'):
        manifest['sources'][filename] = hashlib.sha256((source_dir / filename).read_bytes()).hexdigest()
    output = root / 'Game/Daedalus/Content/Data/Solar/ship-details.json'
    output.write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
    print('SHIP_PRESENTATION_PASS', len(manifest['doors']), 'doors', len(manifest['pointLights']), 'lights')
    return meshes

"""Create an original fixture once; preserve saved edits on subsequent runs."""
import bpy
from pathlib import Path

root = Path(__file__).resolve().parent.parent
source = root / "Art" / "Ships" / "Foundation"
source.mkdir(parents=True, exist_ok=True)
blend = source / "FoundationCruiser.blend"
if blend.exists():
    bpy.ops.wm.open_mainfile(filepath=str(blend))
else:
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    bpy.context.scene.unit_settings.system = "METRIC"
    bpy.context.scene.unit_settings.scale_length = 1.0
    material = bpy.data.materials.new("FoundationAlloy")
    material.diffuse_color = (0.12, 0.38, 0.7, 1)
    material.use_nodes = True
    material.node_tree.nodes["Principled BSDF"].inputs["Base Color"].default_value = material.diffuse_color
    parts = [((0, 0, 0), (120, 28, 14)), ((-15, -23, 0), (55, 18, 10)),
             ((-15, 23, 0), (55, 18, 10)), ((20, 0, 10), (35, 18, 6))]
    for index, (position, dimensions) in enumerate(parts):
        bpy.ops.mesh.primitive_cube_add(size=1, location=position)
        obj = bpy.context.object
        obj.name = f"FoundationCruiser_{index}"
        obj.dimensions = dimensions
        obj.data.materials.append(material)
        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    bpy.ops.object.select_all(action="SELECT")
    bpy.context.view_layer.objects.active = bpy.data.objects["FoundationCruiser_0"]
    bpy.ops.object.join()
    bpy.context.object.name = "SM_FoundationCruiser"
    bpy.context.object.location = (0, 0, 0)
    bpy.ops.wm.save_as_mainfile(filepath=str(blend))
obj = bpy.data.objects["SM_FoundationCruiser"]
assert bpy.context.scene.unit_settings.scale_length == 1.0
assert abs(obj.dimensions.x - 120) < 0.001
bpy.ops.object.select_all(action="DESELECT")
obj.select_set(True)
bpy.context.view_layer.objects.active = obj
bpy.ops.export_scene.gltf(filepath=str(source / "FoundationCruiser.glb"), export_format="GLB", use_selection=True)
bpy.ops.wm.open_mainfile(filepath=str(blend))
assert abs(bpy.data.objects["SM_FoundationCruiser"].dimensions.x - 120) < 0.001
print("DAEDALUS_SOURCE_ASSET_PASS")

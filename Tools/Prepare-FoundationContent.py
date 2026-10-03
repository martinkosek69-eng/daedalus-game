"""Unreal editor commandlet. Import owned foundation source and save blank map."""
import unreal
from pathlib import Path
import json
import struct

root = Path(unreal.Paths.project_dir()).resolve().parent.parent
source = root / "Art" / "Ships" / "Foundation" / "FoundationCruiser.glb"
task = unreal.AssetImportTask()
task.filename = str(source)
task.destination_path = "/Game/Ships/Foundation"
task.destination_name = "SM_FoundationCruiser"
task.automated = True
task.replace_existing = True
task.save = True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
meshes = [unreal.load_asset(p) for p in task.imported_object_paths]
meshes = [m for m in meshes if isinstance(m, unreal.StaticMesh)]
assert len(meshes) == 1, "Expected one original static mesh"
mesh = meshes[0]
extent = mesh.get_bounds().box_extent
# GLB metre -> engine centimetre conversion and forward axis must survive import.
assert abs(extent.x * 2 - 12000) < 5, str(extent)
if mesh.get_path_name() != "/Game/Ships/Foundation/SM_FoundationCruiser.SM_FoundationCruiser":
    assert unreal.EditorAssetLibrary.rename_asset(mesh.get_path_name(), "/Game/Ships/Foundation/SM_FoundationCruiser")
# Interchange reimport preserves existing material overrides. Reconcile the
# owned flat fixture's source color explicitly, then verify the saved dependency.
glb = source.read_bytes()
assert glb[:4] == b"glTF" and glb[16:20] == b"JSON"
document = json.loads(glb[20:20 + struct.unpack_from("<I", glb, 12)[0]])
assert len(document["materials"]) == 1
color = document["materials"][0]["pbrMetallicRoughness"]["baseColorFactor"]
assert len(color) == 4 and all(0 <= c <= 1 for c in color)
material = mesh.get_material(0)
assert isinstance(material, unreal.MaterialInstanceConstant)
assert material.get_path_name().startswith("/Game/Ships/Foundation/")
# UE 5.8's setter returns false even after applying the value; verify readback.
unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(material, "BaseColorFactor", unreal.LinearColor(*color))
unreal.MaterialEditingLibrary.update_material_instance(material)
assert unreal.EditorAssetLibrary.save_loaded_asset(material)
actual = unreal.MaterialEditingLibrary.get_material_instance_vector_parameter_value(material, "BaseColorFactor")
assert all(abs(a - b) < 0.0001 for a, b in zip((actual.r, actual.g, actual.b, actual.a), color))
editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not unreal.EditorAssetLibrary.does_asset_exist("/Game/Maps/Foundation"):
    assert editor.new_level("/Game/Maps/Foundation")
    assert editor.save_current_level()
print("DAEDALUS_CONTENT_PASS")

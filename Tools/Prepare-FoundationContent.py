"""Unreal editor commandlet. Import owned foundation source and save blank map."""
import unreal
from pathlib import Path

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
editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not unreal.EditorAssetLibrary.does_asset_exist("/Game/Maps/Foundation"):
    assert editor.new_level("/Game/Maps/Foundation")
    assert editor.save_current_level()
print("DAEDALUS_CONTENT_PASS")

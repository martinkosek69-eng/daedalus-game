import unreal, json
from pathlib import Path
root=Path(__file__).resolve().parent.parent/'.local'
destination='/Game/__EnvironmentAudit__'
if unreal.EditorAssetLibrary.does_directory_exist(destination):
    raise RuntimeError('Probe destination already exists; refusing to overwrite it')
try:
    task=unreal.AssetImportTask()
    task.set_editor_property('filename',str(root/'audit-probe.glb'))
    task.set_editor_property('destination_path',destination)
    task.set_editor_property('automated',True)
    task.set_editor_property('save',True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    assets=list(unreal.EditorAssetLibrary.list_assets(destination,recursive=True,include_folder=False))
    meshes=[]
    for name in assets:
        asset=unreal.EditorAssetLibrary.load_asset(name)
        if isinstance(asset,unreal.StaticMesh): meshes.append(name)
    if not meshes: raise RuntimeError('No static mesh was imported')
    (root/'audit-import-result.json').write_text(json.dumps({'importedAssets':assets,'staticMeshes':meshes}),encoding='utf8')
    unreal.log('ENVIRONMENT_AUDIT_IMPORT_OK '+json.dumps({'assets':len(assets),'meshes':len(meshes)}))
finally:
    if unreal.EditorAssetLibrary.does_directory_exist(destination):
        if not unreal.EditorAssetLibrary.delete_directory(destination):
            raise RuntimeError('Audit import folder cleanup failed')

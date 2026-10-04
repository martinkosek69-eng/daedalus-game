"""Rebuild only original hyperspace effect materials; never reimport ships."""
from pathlib import Path
import hashlib
import unreal

root=Path(unreal.Paths.project_dir()).resolve().parent.parent
src=root/'Art/Effects/Hyperspace'
folder='/Game/Hyperspace/Materials'
lib=unreal.MaterialEditingLibrary
tools=unreal.AssetToolsHelpers.get_asset_tools()
for name,shader,translucent in [('M_HyperWindow','Window.ush',True),('M_HyperTunnel','Tunnel.ush',False)]:
    mat=unreal.load_asset(folder+'/'+name)
    if not mat: mat=tools.create_asset(name,folder,unreal.Material,unreal.MaterialFactoryNew())
    assert mat
    lib.delete_all_material_expressions(mat)
    mat.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property('two_sided',True)
    mat.set_editor_property('blend_mode',unreal.BlendMode.BLEND_TRANSLUCENT if translucent else unreal.BlendMode.BLEND_OPAQUE)
    if translucent: mat.set_editor_property('translucency_pass',unreal.MaterialTranslucencyPass.MTP_BEFORE_DOF)
    uv=lib.create_material_expression(mat,unreal.MaterialExpressionTextureCoordinate)
    inputs=[('UV',uv)]
    for key,value in [('PhaseTime',0),('Radius',1),('Strength',1)]:
        node=lib.create_material_expression(mat,unreal.MaterialExpressionScalarParameter)
        node.set_editor_property('parameter_name',key)
        node.set_editor_property('default_value',value)
        inputs.append((key,node))
    code=(src/'Noise.ush').read_text()+'\n'+(src/shader).read_text()
    custom=lib.create_material_expression(mat,unreal.MaterialExpressionCustom)
    custom.set_editor_property('code',code)
    custom.set_editor_property('output_type',unreal.CustomMaterialOutputType.CMOT_FLOAT4)
    fields=[]
    for key,node in inputs:
        field=unreal.CustomInput();field.set_editor_property('input_name',key);fields.append(field)
    custom.set_editor_property('inputs',fields)
    for key,node in inputs: assert lib.connect_material_expressions(node,'',custom,key)
    rgb=lib.create_material_expression(mat,unreal.MaterialExpressionComponentMask)
    for channel in ['r','g','b']: rgb.set_editor_property(channel,True)
    assert lib.connect_material_expressions(custom,'',rgb,'')
    assert lib.connect_material_property(rgb,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    if translucent:
        alpha=lib.create_material_expression(mat,unreal.MaterialExpressionComponentMask)
        alpha.set_editor_property('r',False);alpha.set_editor_property('g',False);alpha.set_editor_property('a',True)
        assert lib.connect_material_expressions(custom,'',alpha,'')
        assert lib.connect_material_property(alpha,'',unreal.MaterialProperty.MP_OPACITY)
    lib.recompile_material(mat)
    unreal.EditorAssetLibrary.set_metadata_tag(mat,'HyperspaceRecipe',hashlib.sha256(code.encode()).hexdigest())
    assert unreal.EditorAssetLibrary.save_loaded_asset(mat)
    unreal.log('HYPER_MATERIAL_SAVED '+mat.get_path_name())
unreal.log('HYPER_MATERIALS_PASS')

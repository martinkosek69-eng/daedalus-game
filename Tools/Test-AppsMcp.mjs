import { spawn } from 'node:child_process';
import readline from 'node:readline';
import assert from 'node:assert/strict';
import { writeFile } from 'node:fs/promises';
import path from 'node:path';
const server = process.argv[2];
const child = spawn(process.execPath, [server], { windowsHide: true, stdio: ['pipe','pipe','pipe'] });
let id=0;
const pending=new Map();
readline.createInterface({input:child.stdout}).on('line', line=>{
  const response=JSON.parse(line), item=pending.get(response.id);
  if(item){ clearTimeout(item.timer); pending.delete(response.id); item.resolve(response); }
});
child.stderr.on('data', b=>process.stderr.write(b));
function request(method,params={}){
  return new Promise((resolve,reject)=>{
    const requestId=++id;
    const timer=setTimeout(()=>reject(new Error('MCP test timeout')),180000);
    pending.set(requestId,{resolve,reject,timer});
    child.stdin.write(JSON.stringify({jsonrpc:'2.0',id:requestId,method,params})+'\n');
  });
}
async function call(name,args={}){
  const reply=await request('tools/call',{name,arguments:args});
  assert.ok(!reply.error, JSON.stringify(reply));
  assert.equal(reply.result.isError,false,JSON.stringify(reply.result));
  return JSON.parse(reply.result.content[0].text);
}
try {
  await request('initialize',{protocolVersion:'2025-03-26',capabilities:{},clientInfo:{name:'environment-audit',version:'1'}});
  child.stdin.write(JSON.stringify({jsonrpc:'2.0',method:'notifications/initialized'})+'\n');
  const tools=await request('tools/list');
  const build=await call('build_unreal_editor');
  console.log(JSON.stringify({build}));
  const probe=path.resolve('.local/audit-probe.blend').replaceAll('\\','/');
  const model=path.resolve('.local/audit-probe.glb').replaceAll('\\','/');
  const render=path.resolve('.local/audit-probe.png').replaceAll('\\','/');
  const code=`import bpy\nbpy.data.objects['Cube'].name='EnvironmentAuditProbe'\nbpy.ops.wm.save_as_mainfile(filepath=${JSON.stringify(probe)})\nbpy.ops.export_scene.gltf(filepath=${JSON.stringify(model)},export_format='GLB')\nbpy.context.scene.render.engine='BLENDER_EEVEE'\nbpy.context.scene.render.resolution_x=256\nbpy.context.scene.render.resolution_y=256\nbpy.context.scene.render.resolution_percentage=100\nbpy.context.scene.render.filepath=${JSON.stringify(render)}\nbpy.ops.render.render(write_still=True)\n`;
  const create=await call('blender_python',{code});
  const reopen=await call('blender_scene_info',{blend_file:probe});
  assert.match(reopen.stdout,/EnvironmentAuditProbe/);
  const failure=await request('tools/call',{name:'blender_python',arguments:{code:"raise RuntimeError('expected_audit_error')"}});
  assert.equal(failure.result.isError,true);
  const result={tools:tools.result.tools.map(x=>x.name),build,blenderCreateExportRender:create,blenderReload:reopen,blenderErrorReporting:true};
  await writeFile('.local/audit-apps-result.json',JSON.stringify(result,null,2));
  console.log(JSON.stringify({tools:result.tools,blenderSaved:true,blenderReloaded:true,glbExport:true,render:true,errorReporting:true}));
}finally{for(const item of pending.values()) clearTimeout(item.timer); child.stdin.end();child.kill();}

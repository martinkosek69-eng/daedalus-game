import fs from 'node:fs';
import path from 'node:path';
import {fileURLToPath} from 'node:url';
import {createHash} from 'node:crypto';
import assert from 'node:assert/strict';
const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..');
const dir=path.join(root,'Art/Ships/Daedalus');
const json=name=>JSON.parse(fs.readFileSync(path.join(dir,name),'utf8'));
const meta=json('asset-metadata.json');
for(const [name,key] of [['Original/daedalus.glb','sourceSHA256'],['Daedalus.blend','blendSHA256'],['Daedalus.glb','glbSHA256'],['DaedalusEngineGlow.glb','engineGlowGlbSHA256'],['DaedalusLights.glb','lightsGlbSHA256'],['DaedalusAddOns.glb','addOnsGlbSHA256'],['DaedalusTurrets.glb','turretsGlbSHA256']]){
  assert.equal(createHash('sha256').update(fs.readFileSync(path.join(dir,name))).digest('hex'),meta[key],name+' hash');
}
const binaries=new WeakMap();
function glb(name,expectedImages=0){
  const b=fs.readFileSync(path.join(dir,name));
  assert.equal(b.readUInt32LE(0),0x46546c67);assert.equal(b.readUInt32LE(4),2);assert.equal(b.readUInt32LE(8),b.length);
  const j=JSON.parse(b.toString('utf8',20,20+b.readUInt32LE(12)));
  assert.equal(j.images?.length??0,expectedImages,name+' image count');
  assert.equal(j.buffers.length,1);assert(!j.buffers[0].uri,'external buffer');
  const binaryHeader=20+b.readUInt32LE(12);
  assert.equal(b.readUInt32LE(binaryHeader+4),0x004e4942,'GLB binary chunk');
  const binary=b.subarray(binaryHeader+8);
  assert(binary.length>=j.buffers[0].byteLength);
  binaries.set(j,binary);
  for(const image of j.images??[]){
    assert(!image.uri,'external image');assert.equal(image.mimeType,'image/png');
    const view=j.bufferViews[image.bufferView];assert.equal(view.buffer,0);
    const embedded=binary.subarray(view.byteOffset??0,(view.byteOffset??0)+view.byteLength);
    const standalone=fs.readFileSync(path.join(dir,'Textures',image.name+'.png'));
    assert.equal(createHash('sha256').update(embedded).digest('hex'),createHash('sha256').update(standalone).digest('hex'),name+' embedded / standalone image mismatch');
  }
  return j;
}
const hull=glb('Daedalus.glb',3),glow=glb('DaedalusEngineGlow.glb'),lights=glb('DaedalusLights.glb'),addons=glb('DaedalusAddOns.glb',3),turrets=glb('DaedalusTurrets.glb');
assert.equal(hull.meshes.length,1);assert.equal(lights.meshes.length,1);assert.equal(glow.meshes.length,6);
let triangles=0;
for(const p of hull.meshes[0].primitives){
  assert.equal(p.mode??4,4);assert('COLOR_0' in p.attributes);assert('TEXCOORD_0' in p.attributes);
  assert.equal(hull.accessors[p.attributes.COLOR_0].count,hull.accessors[p.attributes.POSITION].count);
  assert.equal(hull.accessors[p.attributes.TEXCOORD_0].count,hull.accessors[p.attributes.POSITION].count);
  triangles+=hull.accessors[p.indices].count/3;
}
assert.equal(triangles,222330);assert.equal(triangles,meta.finalTriangles);
assert.deepEqual(hull.materials.map(m=>m.name).sort(),[...meta.materialSlots].sort());
assert.equal(meta.originalAstrofossilGeometryChanged,false);
const minimum=[Infinity,Infinity,Infinity],maximum=[-Infinity,-Infinity,-Infinity];
for(const primitive of hull.meshes[0].primitives){
  const a=hull.accessors[primitive.attributes.POSITION];
  for(let i=0;i<3;i++){minimum[i]=Math.min(minimum[i],a.min[i]);maximum[i]=Math.max(maximum[i],a.max[i]);}
}
const dimensions=[maximum[0]-minimum[0],maximum[2]-minimum[2],maximum[1]-minimum[1]];
assert(dimensions.every((v,i)=>Math.abs(v-meta.dimensionsMetres[i])<.001),'actual hull dimensions / axis mismatch');
assert.equal(hull.nodes.filter(n=>n.mesh!==undefined).length,1);
assert(!hull.nodes[0].translation&&!hull.nodes[0].rotation&&!hull.nodes[0].scale,'hull frame must be baked');
for(const [key,filename] of Object.entries(meta.textures.files)){
  const png=fs.readFileSync(path.join(dir,filename));
  assert.equal(createHash('sha256').update(png).digest('hex'),meta.textures.sha256[key]);
  assert.equal(png.subarray(0,8).toString('hex'),'89504e470d0a1a0a');
  assert.equal(png.readUInt32BE(16),2048);assert.equal(png.readUInt32BE(20),2048);
}
for(const model of [hull,addons]){
  for(const material of model.materials.filter(m=>m.extras?.textured)){
    for(const [info,expected] of [[material.normalTexture,'T_Daedalus_Plating_Normal'],[material.pbrMetallicRoughness.baseColorTexture,'T_Daedalus_Plating_BaseColor'],[material.pbrMetallicRoughness.metallicRoughnessTexture,'T_Daedalus_Plating_ORM']]){
      assert.equal(info.texCoord??0,0);
      assert.equal(model.images[model.textures[info.index].source].name,expected);
    }
  }
}
assert.equal(addons.meshes.length,1);assert.equal(addons.nodes.length,1);
assert.equal(addons.nodes[0].name,'Daedalus_AddOns');
for(const primitive of addons.meshes[0].primitives){assert('COLOR_0' in primitive.attributes);assert('TEXCOORD_0' in primitive.attributes);}
const mounts=json('WEAPON_MOUNTS.json'),engines=json('ENGINE_MOUNTS.json');
assert.equal(mounts.version,1);assert.equal(engines.version,1);
assert.equal(mounts.units,'metres');assert.equal(engines.units,'metres');
assert.equal(mounts.mounts.length,62);assert.equal(engines.outlets.length,6);
const ids=new Set(),counts={};
function vector(v){assert(Array.isArray(v)&&v.length===3&&v.every(Number.isFinite));}
function frame(f,u){vector(f);vector(u);assert(Math.abs(Math.hypot(...f)-1)<1e-5);assert(Math.abs(Math.hypot(...u)-1)<1e-5);assert(Math.abs(f.reduce((s,v,i)=>s+v*u[i],0))<1e-5);}
for(const m of mounts.mounts){
  assert(typeof m.mountID==='string'&&!ids.has(m.mountID));ids.add(m.mountID);
  vector(m.centerMetres);frame(m.forwardAxis,m.upAxis);
  if(m.group==='bow_rods'){
    assert(['ROD_01_P','ROD_02_S'].includes(m.mountID));
    const rod=meta.addOns.rods[m.mountID==='ROD_01_P'?0:1];
    assert(m.centerMetres.every((v,i)=>Math.abs(v-rod.tipMetres[i])<.001),'rod tip / add-on mismatch');
    assert(m.centerMetres[0]>300&&m.centerMetres[0]<332);
    assert(Math.abs(m.centerMetres[1])===29.5&&m.centerMetres[2]===-14);
  }else{
    assert(m.centerMetres.every((v,i)=>Math.abs(v)<meta.dimensionsMetres[i]/2+10),'mount outside hull bounds');
  }
  assert(m.referenceURL&&m.confidence);counts[m.group]=(counts[m.group]??0)+1;
}
assert.deepEqual(counts,mounts.groupCounts);
assert.deepEqual(counts,meta.mountGroups);
assert.equal(turrets.meshes.length,1);assert.equal(turrets.nodes.length,38);
const turretIds=new Set();
for(const node of turrets.nodes){
  assert.equal(node.mesh,0);const id=node.extras.mountID;
  assert.equal(node.name,'TURRET_'+id);assert(!turretIds.has(id));turretIds.add(id);
  const mount=mounts.mounts.find(m=>m.mountID===id);
  assert(mount&&['dorsal_railguns','ventral_railguns'].includes(mount.group));
  vector(node.translation);const position=[node.translation[0],-node.translation[2],node.translation[1]];
  assert(position.every((v,i)=>Math.abs(v-mount.centerMetres[i])<.001),'turret / mount mismatch');
  const rotation=node.rotation??[0,0,0,1];
  assert(rotation.length===4&&rotation.every(Number.isFinite));
  assert(Math.abs(Math.hypot(...rotation)-1)<1e-5);
}
assert.equal(turretIds.size,counts.dorsal_railguns+counts.ventral_railguns);
for(const m of engines.outlets){
  assert(!ids.has(m.id));ids.add(m.id);vector(m.centreMetres);frame(m.outwardAxis,m.upAxis);
  assert(m.apertureDiameterMetres>0&&m.lipOuterDiameterMetres>=m.apertureDiameterMetres);
  const n=glow.nodes.filter(n=>n.name===m.glowObject);assert.equal(n.length,1);
  assert.equal(n[0].extras.outlet_id,m.id);
  // glTF Y-up to project Z-up: (x,y,z) -> (x,-z,y).
  const t=n[0].translation,p=[t[0],-t[2],t[1]];
  assert(p.every((v,i)=>Math.abs(v-m.centreMetres[i])<.001),'glow / mount mismatch');
}
console.log(JSON.stringify({passed:true,hullTriangles:triangles,hullMaterials:hull.materials.length,engines:6,weaponAndBayMounts:62,turretNodes:38,platingTextures:3,embeddedImagesMatchDeliveredPNGs:true,externalImages:0}));

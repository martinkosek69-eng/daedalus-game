import fs from 'node:fs';
import path from 'node:path';
import {fileURLToPath} from 'node:url';
import {createHash} from 'node:crypto';
import assert from 'node:assert/strict';
const root=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..');
const dir=path.join(root,'Art/Ships/Daedalus');
const json=name=>JSON.parse(fs.readFileSync(path.join(dir,name),'utf8'));
const meta=json('asset-metadata.json');
for(const [name,key] of [['Original/daedalus.glb','sourceSHA256'],['Daedalus.blend','blendSHA256'],['Daedalus.glb','glbSHA256'],['DaedalusEngineGlow.glb','engineGlowGlbSHA256'],['DaedalusLights.glb','lightsGlbSHA256']]){
  assert.equal(createHash('sha256').update(fs.readFileSync(path.join(dir,name))).digest('hex'),meta[key],name+' hash');
}
function glb(name){
  const b=fs.readFileSync(path.join(dir,name));
  assert.equal(b.readUInt32LE(0),0x46546c67);assert.equal(b.readUInt32LE(4),2);assert.equal(b.readUInt32LE(8),b.length);
  const j=JSON.parse(b.toString('utf8',20,20+b.readUInt32LE(12)));
  assert.equal(j.images?.length??0,0,'undelivered image dependency');
  assert.equal(j.buffers.length,1);assert(!j.buffers[0].uri,'external buffer');
  return j;
}
const hull=glb('Daedalus.glb'),glow=glb('DaedalusEngineGlow.glb'),lights=glb('DaedalusLights.glb');
assert.equal(hull.meshes.length,1);assert.equal(lights.meshes.length,1);assert.equal(glow.meshes.length,6);
let triangles=0;
for(const p of hull.meshes[0].primitives){assert.equal(p.mode??4,4);assert('COLOR_0' in p.attributes);triangles+=hull.accessors[p.indices].count/3;}
assert.equal(triangles,222330);assert.equal(triangles,meta.finalTriangles);
assert.deepEqual(hull.materials.map(m=>m.name).sort(),[...meta.materialSlots].sort());
const mounts=json('WEAPON_MOUNTS.json'),engines=json('ENGINE_MOUNTS.json');
assert.equal(mounts.version,1);assert.equal(engines.version,1);
assert.equal(mounts.units,'metres');assert.equal(engines.units,'metres');
assert.equal(mounts.mounts.length,58);assert.equal(engines.outlets.length,6);
const ids=new Set(),counts={};
function vector(v){assert(Array.isArray(v)&&v.length===3&&v.every(Number.isFinite));}
function frame(f,u){vector(f);vector(u);assert(Math.abs(Math.hypot(...f)-1)<1e-5);assert(Math.abs(Math.hypot(...u)-1)<1e-5);assert(Math.abs(f.reduce((s,v,i)=>s+v*u[i],0))<1e-5);}
for(const m of mounts.mounts){
  assert(typeof m.mountID==='string'&&!ids.has(m.mountID));ids.add(m.mountID);
  vector(m.centerMetres);frame(m.forwardAxis,m.upAxis);
  assert(m.centerMetres.every((v,i)=>Math.abs(v)<meta.dimensionsMetres[i]/2+10),'mount outside hull bounds');
  assert(m.referenceURL&&m.confidence);counts[m.group]=(counts[m.group]??0)+1;
}
assert.deepEqual(counts,mounts.groupCounts);
for(const m of engines.outlets){
  assert(!ids.has(m.id));ids.add(m.id);vector(m.centreMetres);frame(m.outwardAxis,m.upAxis);
  assert(m.apertureDiameterMetres>0&&m.lipOuterDiameterMetres>=m.apertureDiameterMetres);
  const n=glow.nodes.filter(n=>n.name===m.glowObject);assert.equal(n.length,1);
  assert.equal(n[0].extras.outlet_id,m.id);
  // glTF Y-up to project Z-up: (x,y,z) -> (x,-z,y).
  const t=n[0].translation,p=[t[0],-t[2],t[1]];
  assert(p.every((v,i)=>Math.abs(v-m.centreMetres[i])<.001),'glow / mount mismatch');
}
console.log(JSON.stringify({passed:true,hullTriangles:triangles,hullMaterials:hull.materials.length,engines:6,weaponAndBayMounts:58,externalImages:0}));

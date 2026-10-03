import readline from 'node:readline';
import { spawn } from 'node:child_process';
import { mkdir, writeFile, readFile, unlink } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
import { randomUUID } from 'node:crypto';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
let config = {};
try { config = JSON.parse(await readFile(path.join(root, '.local', 'toolchain.json'), 'utf8')); }
catch (error) { if (error.code !== 'ENOENT') throw error; }
const blender = process.env.DAEDALUS_BLENDER || config.blender;
const studio = process.env.DAEDALUS_STUDIO || config.studio;
const engine = process.env.DAEDALUS_ENGINE || config.engine;
const project = path.join(root, 'Game', 'Daedalus', 'Daedalus.uproject');
const work = path.join(root, '.local', 'mcp-work');
// Local overrides also work when a running AI client still has its old environment.
const childEnv = { ...process.env, ...(config.environment || {}) };
function run(file, args, timeout = 300000) {
  return new Promise((resolve, reject) => {
    const child = spawn(file, args, { cwd: work, env: childEnv, windowsHide: true, windowsVerbatimArguments: /cmd\.exe$/i.test(file), stdio: ['ignore', 'pipe', 'pipe'] });
    let stdout = '', stderr = '';
    const append = (old, chunk) => (old + chunk.toString()).slice(-100000);
    child.stdout.on('data', b => { stdout = append(stdout, b); });
    child.stderr.on('data', b => { stderr = append(stderr, b); });
    const timer = setTimeout(() => { child.kill(); reject(new Error('Tool timed out; inspect child processes before retrying.')); }, timeout);
    child.on('error', e => { clearTimeout(timer); reject(e); });
    child.on('close', code => { clearTimeout(timer); resolve({ exitCode: code, stdout, stderr }); });
  });
}

const tools = [
  { name: 'blender_scene_info', description: 'Read a .blend scene in a background Blender process, or inspect a factory startup scene. Does not inspect an already open GUI session.', inputSchema: { type: 'object', properties: { blend_file: { type: 'string' } }, additionalProperties: false }, annotations: { readOnlyHint: true, destructiveHint: false, openWorldHint: false } },
  { name: 'blender_python', description: 'Run Python with the official bpy API in background Blender. Optionally load a .blend file. To persist edits the script must explicitly save a .blend file. This executes supplied code; review filesystem writes before use.', inputSchema: { type: 'object', properties: { code: { type: 'string' }, blend_file: { type: 'string' } }, required: ['code'], additionalProperties: false }, annotations: { readOnlyHint: false, destructiveHint: true, openWorldHint: true } },
  { name: 'visual_studio_toolchain_info', description: 'Read the installed Visual Studio MSBuild version and configured application paths.', inputSchema: { type: 'object', properties: {}, additionalProperties: false }, annotations: { readOnlyHint: true, destructiveHint: false, openWorldHint: false } },
  { name: 'build_unreal_editor', description: 'Build the existing Daedalus C++ editor target with the installed Visual Studio toolchain. Writes generated build files and logs. Close the Unreal editor before a full build.', inputSchema: { type: 'object', properties: {}, additionalProperties: false }, annotations: { readOnlyHint: false, destructiveHint: false, openWorldHint: false } }
];

async function call(name, args) {
  await mkdir(work, { recursive: true });
  if ((name.startsWith('blender_') && !blender) || (name === 'build_unreal_editor' && !engine) || (name === 'visual_studio_toolchain_info' && !studio)) throw new Error('Configure .local/toolchain.json using Tools/toolchain.example.json before calling this tool.');
  if (name === 'visual_studio_toolchain_info') {
    return { studio, blender, engine, project, ...(await run(path.join(studio, 'MSBuild\\Current\\Bin\\MSBuild.exe'), ['-version', '-nologo'])) };
  }
  if (name === 'build_unreal_editor') {
    const batch = path.join(engine, 'Engine\\Build\\BatchFiles\\Build.bat');
    const command = `""${batch}" DaedalusEditor Win64 Development -Project="${project}" -WaitMutex -NoHotReloadFromIDE -MaxParallelActions=1"`;
    return run('C:\\Windows\\System32\\cmd.exe', ['/d', '/s', '/c', command]);
  }
  if (name === 'blender_scene_info' || name === 'blender_python') {
    if (args.blend_file && (typeof args.blend_file !== 'string' || !path.isAbsolute(args.blend_file) || !args.blend_file.toLowerCase().endsWith('.blend'))) throw new Error('blend_file must be an absolute .blend path.');
    let code = args.code;
    if (name === 'blender_scene_info') code = 'import bpy, json\nprint("CODEX_SCENE_JSON=" + json.dumps({"version":bpy.app.version_string,"filepath":bpy.data.filepath,"objects":[{"name":o.name,"type":o.type} for o in bpy.context.scene.objects]}))\n';
    if (typeof code !== 'string' || !code.trim()) throw new Error('Nonempty Python code is required.');
    const script = path.join(work, randomUUID() + '.py');
    await writeFile(script, code, 'utf8');
    const cli = ['--background', '--factory-startup'];
    if (args.blend_file) cli.push(args.blend_file);
    cli.push('--python-exit-code', '1', '--python', script);
    try { return await run(blender, cli); } finally { await unlink(script).catch(() => {}); }
  }
  throw new Error('Unknown tool: ' + name);
}

function send(value) { process.stdout.write(JSON.stringify(value) + '\n'); }
async function dispatch(message) {
  if (!Object.hasOwn(message, 'id')) return;
  try {
    let result;
    if (message.method === 'initialize') result = { protocolVersion: message.params?.protocolVersion || '2025-03-26', capabilities: { tools: {} }, serverInfo: { name: 'daedalus-local-apps', version: '1.0.0' } };
    else if (message.method === 'ping') result = {};
    else if (message.method === 'tools/list') result = { tools };
    else if (message.method === 'tools/call') {
      const output = await call(message.params?.name, message.params?.arguments || {});
      result = { content: [{ type: 'text', text: JSON.stringify(output) }], isError: output.exitCode !== 0 };
    } else { send({ jsonrpc: '2.0', id: message.id, error: { code: -32601, message: 'Method not found' } }); return; }
    send({ jsonrpc: '2.0', id: message.id, result });
  } catch (error) {
    if (message.method === 'tools/call') send({ jsonrpc: '2.0', id: message.id, result: { isError: true, content: [{ type: 'text', text: String(error.message || error) }] } });
    else send({ jsonrpc: '2.0', id: message.id, error: { code: -32603, message: String(error.message || error) } });
  }
}
const input = readline.createInterface({ input: process.stdin, crlfDelay: Infinity });
let queue = Promise.resolve();
input.on('line', line => {
  queue = queue.then(async () => { let message; try { message = JSON.parse(line); } catch { send({ jsonrpc: '2.0', id: null, error: { code: -32700, message: 'Invalid JSON' } }); return; } await dispatch(message); });
});

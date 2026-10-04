"""Planet quality: shared master materials, planet texture settings and per-body instances.

Unreal commandlet helper loaded by Prepare-SolarSystemMaterials.prepare(). Presentation only: it
reads canonical catalog rows and never changes IDs, sizes, positions or flight data.

- Masters in /Game/PlanetQuality/Materials: M_PlanetSurface (rocky/icy/ocean worlds), M_PlanetGas
  (no rock relief), M_PlanetStar (emissive, limb darkening, no planetary lighting) and
  M_PlanetClouds (separate translucent cloud shell).
- Per-body instances keep the stable runtime paths /Game/Solar/Materials/M_Body_<key> and
  M_Cloud_<key> (always cooked, loaded by path at runtime).
- All planet maps use TEXTUREGROUP_Project01 (planet group in DefaultDeviceProfiles.ini: linear
  mips, 16K limit) and sampling with analytic longitude/latitude UVs, seam-safe gradients.
- Generated micro detail (noise) only appears where a source texel spans more than one screen
  pixel; on real bodies it is an artistic supplement, never a geographic claim.
"""
import json
import math

MASTERS = '/Game/PlanetQuality/Materials'
TEXTURES = '/Game/PlanetQuality/Textures'
BODY_MATERIALS = '/Game/Solar/Materials'
SPHERES = {'SM_PlanetSphere_256': 'Art/Space/PlanetQuality/Meshes/PlanetSphere_256.glb',
           'SM_PlanetSphere_512': 'Art/Space/PlanetQuality/Meshes/PlanetSphere_512.glb'}
SPHERE_FOLDER = '/Game/Solar/Models'
CLOUD_SHELL_SCALE = 1.0015         # runtime cloud shell radius / body radius (SolarSystem.cpp)
DEFAULTS = {'Black': ('Art/Space/PlanetQuality/Defaults/pq_black.png', 'color'),
            'Clear': ('Art/Space/PlanetQuality/Defaults/pq_clear.png', 'mask'),
            'FlatLand': ('Art/Space/PlanetQuality/Defaults/pq_flat_land.png', 'mask')}
GAS_GIANT_RADIUS = 15.0e6          # metres; larger catalog planets render as gas/ice giants

# ---------------------------------------------------------------- HLSL (Custom node bodies)
LIBRARY = r'''
struct PQ
{
    float Hash(float3 p)
    {
        p = frac(p * 0.3183099 + 0.1);
        p *= 17.0;
        return frac(p.x * p.y * p.z * (p.x + p.y + p.z));
    }
    // Value noise in [0,1] with analytic gradient (Quilez).
    float4 Noise(float3 x)
    {
        float3 i = floor(x), f = frac(x);
        float3 u = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);
        float3 du = 30.0 * f * f * (f * (f - 2.0) + 1.0);
        float a = Hash(i), b = Hash(i + float3(1, 0, 0)), c = Hash(i + float3(0, 1, 0)), d = Hash(i + float3(1, 1, 0));
        float e = Hash(i + float3(0, 0, 1)), g = Hash(i + float3(1, 0, 1)), h = Hash(i + float3(0, 1, 1)), k = Hash(i + float3(1, 1, 1));
        float k1 = b - a, k2 = c - a, k3 = e - a, k4 = a - b - c + d, k5 = a - c - e + h, k6 = a - b - e + g;
        float k7 = -a + b + c - d + e - g - h + k;
        return float4(a + k1 * u.x + k2 * u.y + k3 * u.z + k4 * u.x * u.y + k5 * u.y * u.z + k6 * u.z * u.x + k7 * u.x * u.y * u.z,
                      du * float3(k1 + k4 * u.y + k6 * u.z + k7 * u.y * u.z,
                                  k2 + k5 * u.z + k4 * u.x + k7 * u.z * u.x,
                                  k3 + k6 * u.x + k5 * u.y + k7 * u.x * u.y));
    }
};
PQ F;
// Unit direction in body space; equirectangular coordinates matching SolarSphere.glb UVs
// (u = 0.5 at +X, north up). Gradients wrap at the longitude seam so mips stay correct.
float3 n = normalize(P);
float lat = asin(clamp(n.z, -1.0, 1.0));
float2 uv = float2(0.5 - atan2(n.y, n.x) * 0.1591549431, 0.5 - lat * 0.3183098862);
uv = lerp(uv, UV0, MeshUV);
float2 dx = ddx(uv), dy = ddy(uv);
dx.x -= round(dx.x); dy.x -= round(dy.x);
float pix = max(length(fwidth(n)), 1e-7);
'''

SURFACE = LIBRARY + r'''
float3 L = normalize(Lw), V = normalize(Vw);
float cl = max(cos(lat), 0.02);
float rxy = length(n.xy);
float3 E = rxy > 1e-5 ? float3(n.y, -n.x, 0.0) / rxy : float3(0.0, -1.0, 0.0);
float3 North = cross(E, n);
float3 albedo = Day.SampleGrad(DaySampler, uv, dx, dy).rgb;

// Data relief: 0 = water, land = 16 + 239*sqrt(h/8848 m). Central differences at the larger of the
// texel and pixel footprint; water neighbours reuse the centre height (no false coastal cliffs).
float2 st = max(1.0 / ReliefSize.xy, max(abs(dx), abs(dy)));
float ec = Relief.SampleGrad(ReliefSampler, uv, dx, dy).r * 255.0;
float eE = Relief.SampleGrad(ReliefSampler, uv + float2(st.x, 0.0), dx, dy).r * 255.0;
float eW = Relief.SampleGrad(ReliefSampler, uv - float2(st.x, 0.0), dx, dy).r * 255.0;
float eN = Relief.SampleGrad(ReliefSampler, uv - float2(0.0, st.y), dx, dy).r * 255.0;
float eS = Relief.SampleGrad(ReliefSampler, uv + float2(0.0, st.y), dx, dy).r * 255.0;
float water = (1.0 - saturate((ec - 3.0) / 9.0)) * WaterMask;
float hc = pow(saturate((ec - 16.0) / 239.0), 2.0) * 8848.0;
float hE = eE < 9.0 ? hc : pow(saturate((eE - 16.0) / 239.0), 2.0) * 8848.0;
float hW = eW < 9.0 ? hc : pow(saturate((eW - 16.0) / 239.0), 2.0) * 8848.0;
float hN = eN < 9.0 ? hc : pow(saturate((eN - 16.0) / 239.0), 2.0) * 8848.0;
float hS = eS < 9.0 ? hc : pow(saturate((eS - 16.0) / 239.0), 2.0) * 8848.0;
float sx = (hE - hW) / (2.0 * st.x * 6.2831853 * RadiusMetres * cl);
float sy = (hN - hS) / (2.0 * st.y * 3.1415927 * RadiusMetres);
float3 N = normalize(n - (sx * E + sy * North) * ReliefScale);

// Artistic micro detail below the source texel size; fades out when not magnified or sub-pixel.
float mag = (3.1415927 / DaySize.y) / pix;
float fade = saturate((mag - 1.0) * 0.5) * DetailStrength * (1.0 - water);
if (fade > 0.001)
{
    float freq = 2.0 * DaySize.y / 3.1415927, amp = 1.0, detail = 0.0;
    float3 grad = 0;
    [unroll] for (int o = 0; o < 3; ++o)
    {
        float w = saturate(1.5 - freq * pix * 1.5);
        float4 v = F.Noise(n * freq + o * 17.31);
        detail += (v.x - 0.5) * amp * w;
        grad += v.yzw * amp * w;
        freq *= 2.07; amp *= 0.5;
    }
    grad -= n * dot(grad, n);
    N = normalize(N - grad * fade * 0.35);
    albedo *= 1.0 + detail * fade * DetailAlbedo;
}

float ndlG = dot(n, L), ndl = dot(N, L), ndv = saturate(dot(n, V));
float sunVis = smoothstep(-0.04, 0.05, ndlG);
float3 sun = SunColor.rgb;
// Cloud shadow: sample the cloud layer where the sun ray to this point crosses the shell.
float2 Lt = float2(dot(L, E), dot(L, North));
float tanZ = sqrt(saturate(1.0 - ndlG * ndlG)) / max(ndlG, 0.25);
float2 off = Lt / max(length(Lt), 1e-5) * CloudHeight * tanZ;
float2 uvs = uv + float2(off.x / (6.2831853 * cl), -off.y / 3.1415927);
float cover = saturate(Clouds.SampleGrad(CloudsSampler, uvs, dx * 2.0, dy * 2.0).r * CloudOpacity);
float shadow = 1.0 - CloudShadow * cover;
float3 color = albedo * (saturate(ndl) * sunVis * shadow * Brightness * sun + Ambient);

// Ocean: GGX sun glint (broad glitter + sharp core) and Fresnel sky reflection on flat water.
float3 H = normalize(L + V);
float ndh = saturate(dot(n, H));
float fres = 0.02 + 0.98 * pow(1.0 - saturate(dot(H, V)), 5.0);
float a1 = 0.0144, a2 = 0.000625;
float d1 = a1 / (3.1415927 * pow(ndh * ndh * (a1 - 1.0) + 1.0, 2.0));
float d2 = a2 / (3.1415927 * pow(ndh * ndh * (a2 - 1.0) + 1.0, 2.0));
float spec = (d1 + d2 * 0.08) * fres / (4.0 * max(ndv, 0.1)) * saturate(ndlG);
color += sun * spec * water * OceanGlint * sunVis * shadow;
// Reflected sky radiance is a small fraction of direct sunlight (same scale as the haze light).
color += AtmosphereColor.rgb * 0.1 * (0.02 + 0.98 * pow(1.0 - ndv, 5.0)) * water * SkyReflection * saturate(ndlG + 0.1);

// Aerial perspective: thicker path towards the limb, lit by the day side.
float haze = saturate((1.0 - exp(-HazeDepth / max(ndv, 0.03))) * HazeStrength);
float hazeLight = saturate(ndlG * 0.9 + 0.12) * smoothstep(-0.15, 0.1, ndlG);
color = lerp(color, AtmosphereColor.rgb * 0.25 * hazeLight * Brightness * sun, min(haze, 0.6));

color += Night.SampleGrad(NightSampler, uv, dx, dy).rgb * NightStrength * (1.0 - smoothstep(-0.10, 0.03, ndlG));
return color;
'''

GAS = LIBRARY + r'''
float3 L = normalize(Lw), V = normalize(Vw);
float3 albedo = Day.SampleGrad(DaySampler, uv, dx, dy).rgb;
// Artistic band-aligned turbulence below the texel size (stretched along longitude).
float mag = (3.1415927 / DaySize.y) / pix;
float fade = saturate((mag - 1.0) * 0.5) * DetailStrength;
if (fade > 0.001)
{
    float freq = 2.0 * DaySize.y / 3.1415927, amp = 1.0, detail = 0.0;
    [unroll] for (int o = 0; o < 3; ++o)
    {
        float w = saturate(1.5 - freq * pix * 1.5);
        detail += (F.Noise(float3(n.xy * freq * 0.35, n.z * freq * 2.0) + o * 23.7).x - 0.5) * amp * w;
        freq *= 2.13; amp *= 0.5;
    }
    albedo *= 1.0 + detail * fade * DetailAlbedo;
}
float ndl = dot(n, L), ndv = saturate(dot(n, V));
float sunVis = smoothstep(-0.03, 0.06, ndl);
// Minnaert limb darkening of a deep cloud deck; no solid relief.
float lit = pow(saturate(ndl), Limb) * pow(max(ndv, 0.02), Limb - 1.0);
float3 color = albedo * (lit * sunVis * Brightness * SunColor.rgb + Ambient);
float haze = saturate((1.0 - exp(-HazeDepth / max(ndv, 0.03))) * HazeStrength);
color = lerp(color, AtmosphereColor.rgb * 0.35 * saturate(ndl * 0.9 + 0.12) * sunVis * Brightness * SunColor.rgb, haze);
color += Night.SampleGrad(NightSampler, uv, dx, dy).rgb * NightStrength * (1.0 - smoothstep(-0.10, 0.03, ndl));
return color;
'''

STAR = LIBRARY + r'''
float3 V = normalize(Vw);
float3 c = Day.SampleGrad(DaySampler, uv, dx, dy).rgb;
float mag = (3.1415927 / DaySize.y) / pix;
float fade = saturate((mag - 1.0) * 0.5) * DetailStrength;
if (fade > 0.001)
{
    // Artistic granulation below the texel size.
    float freq = 2.0 * DaySize.y / 3.1415927, amp = 1.0, detail = 0.0;
    [unroll] for (int o = 0; o < 3; ++o)
    {
        float w = saturate(1.5 - freq * pix * 1.5);
        detail += (F.Noise(n * freq + o * 11.3).x - 0.5) * amp * w;
        freq *= 2.09; amp *= 0.5;
    }
    c *= 1.0 + detail * fade * DetailAlbedo;
}
// Quadratic photospheric limb darkening; stars emit, no planetary lighting.
float mu = saturate(dot(n, V)), m = 1.0 - mu;
return c * Tint.rgb * Intensity * saturate(1.0 - LimbA * m - LimbB * m * m);
'''

CLOUD_COVER = LIBRARY + r'''
float c = Clouds.SampleGrad(CloudsSampler, uv, dx, dy).r;
// Artistic erosion of partially covered edges below the source texel size.
float mag = (3.1415927 / CloudSize) / pix;
float fade = saturate((mag - 1.0) * 0.5) * CloudDetail;
if (fade > 0.001)
{
    float freq = 2.0 * CloudSize / 3.1415927, amp = 0.5, v = 0.0;
    [unroll] for (int o = 0; o < 3; ++o)
    {
        float w = saturate(1.5 - freq * pix * 1.5);
        v += (F.Noise(n * freq + o * 31.7).x - 0.5) * amp * w;
        freq *= 2.11; amp *= 0.55;
    }
    c = saturate(c + v * fade * 6.4 * c * (1.0 - c));
}
return saturate(c * CloudOpacity);
'''

CLOUD_COLOR = r'''
float3 n = normalize(P), L = normalize(Lw);
float ndl = dot(n, L);
float day = smoothstep(-0.10, 0.06, ndl);
float3 tint = lerp(float3(1.0, 0.62, 0.42), float3(1.0, 1.0, 1.0), saturate(ndl / 0.18 + 0.25));
float thick = lerp(0.78, 1.0, A);
return CloudColor.rgb * thick * (saturate(ndl * 0.92 + 0.08) * day * tint * Brightness * SunColor.rgb + Ambient);
'''

SCALARS = {
    'surface': dict(MeshUV=0, RadiusMetres=6.371e6, ReliefScale=0, WaterMask=0, DetailStrength=0, DetailAlbedo=.12,
                    CloudHeight=CLOUD_SHELL_SCALE - 1, CloudShadow=0, CloudOpacity=1, OceanGlint=0, SkyReflection=0,
                    HazeDepth=.035, HazeStrength=0, NightStrength=0, Brightness=.96, Ambient=.026),
    'gas': dict(MeshUV=0, DetailStrength=0, DetailAlbedo=.10, Limb=1.08, HazeDepth=.05, HazeStrength=0, NightStrength=0,
                Brightness=.96, Ambient=.026),
    'star': dict(MeshUV=0, DetailStrength=0, DetailAlbedo=.10, Intensity=1, LimbA=.47, LimbB=.23),
    'clouds': dict(MeshUV=0, CloudSize=8192, CloudDetail=0, CloudOpacity=1, Brightness=.96, Ambient=.006),
}
VECTORS = {
    'surface': dict(SunColor=(1, 1, 1, 1), AtmosphereColor=(.3, .55, 1, 1), ReliefSize=(4, 4, 0, 0), DaySize=(4096, 2048, 0, 0)),
    'gas': dict(SunColor=(1, 1, 1, 1), AtmosphereColor=(.6, .6, .6, 1), DaySize=(4096, 2048, 0, 0)),
    'star': dict(Tint=(2.1, 2.1, 2.1, 1), DaySize=(2048, 1024, 0, 0)),
    'clouds': dict(SunColor=(1, 1, 1, 1), CloudColor=(.95, .96, .97, 1)),
}
TEXTURE_PARAMS = {'surface': {'Day': 'Black', 'Relief': 'FlatLand', 'Clouds': 'Clear', 'Night': 'Black'},
                  'gas': {'Day': 'Black', 'Night': 'Black'}, 'star': {'Day': 'Black'}, 'clouds': {'Clouds': 'Clear'}}
MASTER_NAMES = {'surface': 'M_PlanetSurface', 'gas': 'M_PlanetGas', 'star': 'M_PlanetStar', 'clouds': 'M_PlanetClouds'}


MANIFEST = 'Art/Space/PlanetQuality/planets.json'
SOURCE_KEYS = ('day', 'night', 'clouds', 'relief')
NUMBER_KEYS = {'reliefScale': (0, 20), 'oceanGlint': (0, 10), 'cloudShadow': (0, 1), 'cloudOpacity': (0, 2),
               'cloudDetail': (0, 2), 'detail': (0, 2), 'nightStrength': (0, 4)}


def load_manifest(root, catalog_ids=None):
    """Validated planet-quality manifest. Missing file -> legacy presentation (compatible fallback)."""
    path = root / MANIFEST
    if not path.is_file():
        return dict(version=1, rollout=False, bodies={})
    data = json.loads(path.read_text(encoding='utf-8-sig'))
    assert data.get('version') == 1 and isinstance(data.get('rollout', False), bool), 'planets.json version/rollout'
    bodies = data.get('bodies', {})
    assert isinstance(bodies, dict)
    for ident, entry in bodies.items():
        assert catalog_ids is None or ident in catalog_ids, ('planets.json names an unknown body', ident)
        unknown = set(entry) - set(SOURCE_KEYS) - set(NUMBER_KEYS)
        assert not unknown, (ident, unknown)
        for key in SOURCE_KEYS:
            if key in entry:
                source = root / entry[key]
                assert entry[key].startswith('Art/Space/') and '..' not in entry[key] and source.is_file(), (ident, key, entry[key])
        for key, (low, high) in NUMBER_KEYS.items():
            if key in entry:
                assert isinstance(entry[key], (int, float)) and low <= entry[key] <= high, (ident, key, entry[key])
    return data


def uses_planet_quality(row, manifest):
    return row['id'] in manifest['bodies'] or bool(manifest.get('rollout'))


def material_key(ident):
    return ident.removeprefix('sol.').replace('.', '_').replace('-', '_')


def body_type(row):
    if row.get('kind') == 'star':
        return 'star'
    if row.get('kind') == 'planet' and (row.get('radiusMetres') or 0) >= GAS_GIANT_RADIUS:
        return 'gas'
    return 'surface'


def srgb_to_linear(c):
    return c / 12.92 if c <= .04045 else ((c + .055) / 1.055) ** 2.4


def hex_linear(text, fallback=(.3, .55, 1)):
    if not text:
        return fallback
    return tuple(srgb_to_linear(int(text[i:i + 2], 16) / 255) for i in (1, 3, 5))


class Recipe:
    """Holds the commandlet helpers from Prepare-SolarContent.py."""

    def __init__(self, root, unreal, tools, lib, imported, node, custom, finish, material):
        self.root, self.unreal, self.tools, self.lib = root, unreal, tools, lib
        self.imported, self.node, self.custom, self.finish, self.material = imported, node, custom, finish, material
        self.textures, self.masters, self.defaults = {}, {}, {}
        self.report = []
        self.manifest = load_manifest(root)

    # ------------------------------------------------------------ textures
    def configure(self, asset, kind):
        """kind: 'color' (BC7 sRGB), 'linear' (BC7 linear) or 'mask' (BC4 single channel, linear)."""
        unreal = self.unreal
        compression = {'color': unreal.TextureCompressionSettings.TC_BC7, 'linear': unreal.TextureCompressionSettings.TC_BC7,
                       'mask': unreal.TextureCompressionSettings.TC_ALPHA}[kind]
        asset.set_editor_property('compression_settings', compression)
        asset.set_editor_property('srgb', kind == 'color')
        asset.set_editor_property('lod_group', unreal.TextureGroup.TEXTUREGROUP_PROJECT01)
        asset.set_editor_property('mip_gen_settings', unreal.TextureMipGenSettings.TMGS_FROM_TEXTURE_GROUP)
        asset.set_editor_property('max_texture_size', 0)
        asset.set_editor_property('address_x', unreal.TextureAddress.TA_WRAP)
        asset.set_editor_property('address_y', unreal.TextureAddress.TA_CLAMP)
        assert unreal.EditorAssetLibrary.save_loaded_asset(asset)
        self.report.append(('texture', asset.get_path_name(), kind, (asset.blueprint_get_size_x(), asset.blueprint_get_size_y())))
        return asset

    def texture(self, source, kind, name=None):
        """Import a planet-quality source into /Game/PlanetQuality/Textures with planet settings."""
        identity = source.relative_to(self.root).as_posix()
        if identity not in self.textures:
            asset = self.imported(source, TEXTURES, name or 'T_PQ_' + source.stem, self.unreal.Texture2D)
            self.textures[identity] = self.configure(asset, kind)
        return self.textures[identity]

    def default(self, name):
        if name not in self.defaults:
            path, kind = DEFAULTS[name]
            self.defaults[name] = self.texture(self.root / path, kind, name='T_PQ_Default' + name)
        return self.defaults[name]

    # ------------------------------------------------------------ masters
    def master(self, kind):
        if kind in self.masters:
            return self.masters[kind]
        unreal, node, custom = self.unreal, self.node, self.custom
        clouds = kind == 'clouds'
        mat = self.material_at(MASTER_NAMES[kind], clouds)
        world = node(mat, unreal.MaterialExpressionWorldPosition)
        local = node(mat, unreal.MaterialExpressionTransformPosition,
                     transform_source_type=unreal.MaterialPositionTransformSource.TRANSFORMPOSSOURCE_WORLD,
                     transform_type=unreal.MaterialPositionTransformSource.TRANSFORMPOSSOURCE_LOCAL)
        assert self.lib.connect_material_expressions(world, '', local, '')

        def to_local(expr, channel=''):
            t = node(mat, unreal.MaterialExpressionTransform,
                     transform_source_type=unreal.MaterialVectorCoordTransformSource.TRANSFORMSOURCE_WORLD,
                     transform_type=unreal.MaterialVectorCoordTransform.TRANSFORM_LOCAL)
            assert self.lib.connect_material_expressions(expr, channel, t, '')
            return t

        def scalar(name):
            return node(mat, unreal.MaterialExpressionScalarParameter, parameter_name=name, default_value=float(SCALARS[kind][name]))

        def vector(name):
            return node(mat, unreal.MaterialExpressionVectorParameter, parameter_name=name,
                        default_value=unreal.LinearColor(*VECTORS[kind][name]))

        def texobj(name):
            default = TEXTURE_PARAMS[kind][name]
            sampler = unreal.MaterialSamplerType.SAMPLERTYPE_ALPHA if DEFAULTS[default][1] == 'mask' else unreal.MaterialSamplerType.SAMPLERTYPE_COLOR
            return node(mat, unreal.MaterialExpressionTextureObjectParameter, parameter_name=name,
                        texture=self.default(default), sampler_type=sampler)

        inputs = {'P': (local, ''), 'UV0': (node(mat, unreal.MaterialExpressionTextureCoordinate), '')}
        sun = node(mat, unreal.MaterialExpressionVectorParameter, parameter_name='SunDirection', default_value=unreal.LinearColor(.7, -.7, 0, 0))
        if kind != 'star':
            inputs['Lw'] = (to_local(sun, 'RGB'), '')
        if kind != 'clouds':
            inputs['Vw'] = (to_local(node(mat, unreal.MaterialExpressionCameraVectorWS)), '')
        for name in TEXTURE_PARAMS[kind]:
            inputs[name] = (texobj(name), '')
        for name in SCALARS[kind]:
            if not clouds or name in ('MeshUV', 'CloudSize', 'CloudDetail', 'CloudOpacity'):
                inputs[name] = (scalar(name), '')
        for name in VECTORS[kind]:
            if not clouds:
                inputs[name] = (vector(name), '')
        if clouds:
            cover = custom(mat, CLOUD_COVER, inputs, unreal.CustomMaterialOutputType.CMOT_FLOAT1)
            assert self.lib.connect_material_property(cover, '', unreal.MaterialProperty.MP_OPACITY)
            color_inputs = {'P': (local, ''), 'Lw': inputs['Lw'], 'A': (cover, '')}
            for name in ('Brightness', 'Ambient'):
                color_inputs[name] = (scalar(name), '')
            for name in ('SunColor', 'CloudColor'):
                color_inputs[name] = (vector(name), '')
            self.finish(mat, custom(mat, CLOUD_COLOR, color_inputs))
        else:
            code = {'surface': SURFACE, 'gas': GAS, 'star': STAR}[kind]
            self.finish(mat, custom(mat, code, inputs))
        self.masters[kind] = mat
        self.report.append(('master', mat.get_path_name()))
        return mat

    def material_at(self, name, translucent):
        unreal, tools = self.unreal, self.tools
        path = MASTERS + '/' + name
        mat = unreal.load_asset(path)
        if mat is None:
            mat = tools.create_asset(name, MASTERS, unreal.Material, unreal.MaterialFactoryNew())
        assert isinstance(mat, unreal.Material), path
        # Rebuild the owned graph from scratch on each recipe run (masters are never hand edited).
        self.lib.delete_all_material_expressions(mat)
        mat.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
        mat.set_editor_property('blend_mode', unreal.BlendMode.BLEND_TRANSLUCENT if translucent else unreal.BlendMode.BLEND_OPAQUE)
        mat.set_editor_property('two_sided', False)
        return mat

    # ------------------------------------------------------------ instances
    def instance(self, name, master, folder=BODY_MATERIALS):
        unreal, lib = self.unreal, self.lib
        path = folder + '/' + name
        mi = unreal.load_asset(path)
        if mi is not None and not isinstance(mi, unreal.MaterialInstanceConstant):
            # Legacy per-body Material at the same stable path becomes a shared-master instance.
            assert unreal.EditorAssetLibrary.delete_asset(path), ('Cannot replace legacy material', path)
            mi = None
        if mi is None:
            mi = self.tools.create_asset(name, folder, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        lib.set_material_instance_parent(mi, master)
        lib.clear_all_material_instance_parameters(mi)
        return mi

    def apply(self, mi, kind, scalars=None, vectors=None, textures=None):
        lib, unreal = self.lib, self.unreal
        # The engine setters return False even on success (UE 5.8); verify by reading back.
        for name, value in (scalars or {}).items():
            assert name in SCALARS[kind], (kind, name)
            lib.set_material_instance_scalar_parameter_value(mi, name, float(value))
            assert abs(lib.get_material_instance_scalar_parameter_value(mi, name) - float(value)) <= 1e-4 * max(1, abs(value)), (mi.get_name(), name)
        for name, value in (vectors or {}).items():
            assert name in VECTORS[kind], (kind, name)
            lib.set_material_instance_vector_parameter_value(mi, name, unreal.LinearColor(*value))
            got = lib.get_material_instance_vector_parameter_value(mi, name)
            assert abs(got.r - value[0]) < 1e-3 * max(1, abs(value[0])) and abs(got.g - value[1]) < 1e-3 * max(1, abs(value[1])), (mi.get_name(), name)
        for name, asset in (textures or {}).items():
            assert name in TEXTURE_PARAMS[kind], (kind, name)
            lib.set_material_instance_texture_parameter_value(mi, name, asset)
            assert lib.get_material_instance_texture_parameter_value(mi, name) == asset, (mi.get_name(), name)
        lib.update_material_instance(mi)
        assert unreal.EditorAssetLibrary.save_loaded_asset(mi)
        self.report.append(('instance', mi.get_path_name(), kind))
        return mi

    # ------------------------------------------------------------ meshes
    def spheres(self):
        unreal = self.unreal
        for name, source in SPHERES.items():
            mesh = self.imported(self.root / source, SPHERE_FOLDER, name, unreal.StaticMesh)
            assert abs(mesh.get_bounds().box_extent.x - 100) < .01, (name, mesh.get_bounds())
            nanite = mesh.get_editor_property('nanite_settings')
            nanite.set_editor_property('enabled', False)
            mesh.set_editor_property('nanite_settings', nanite)
            assert unreal.EditorAssetLibrary.save_loaded_asset(mesh)
            self.report.append(('mesh', mesh.get_path_name()))

    # ------------------------------------------------------------ bodies
    def body(self, row, day, night=None, clouds=None, relief=None):
        """Create M_Body_<key> (and M_Cloud_<key>) for one catalog row from imported textures."""
        key = material_key(row['id'])
        kind = body_type(row)
        q = self.manifest['bodies'].get(row['id'], {})
        size = (day.blueprint_get_size_x(), day.blueprint_get_size_y())
        atmosphere = hex_linear(row.get('atmosphereColor'), (.6, .6, .6))
        textures = {'Day': day}
        if kind == 'star':
            tint = (2.1, 1.75, 1.25, 1) if row['id'] == 'sol.sun' else (2.1, 2.1, 2.1, 1)
            scalars = dict(DetailStrength=q.get('detail', .6))
            vectors = dict(Tint=tint, DaySize=(*size, 0, 0))
        elif kind == 'gas':
            scalars = dict(DetailStrength=q.get('detail', .5), HazeStrength=.6 if row.get('atmosphereColor') else 0)
            vectors = dict(AtmosphereColor=(*atmosphere, 1), DaySize=(*size, 0, 0))
            if night:
                textures['Night'] = night
                scalars['NightStrength'] = .85
        else:
            scalars = dict(RadiusMetres=row['radiusMetres'], DetailStrength=q.get('detail', .5),
                           HazeStrength=1 if row.get('atmosphereColor') else 0)
            vectors = dict(AtmosphereColor=(*atmosphere, 1), DaySize=(*size, 0, 0))
            if relief:
                textures['Relief'] = relief
                scalars.update(ReliefScale=q.get('reliefScale', 4), WaterMask=1, OceanGlint=q.get('oceanGlint', 3),
                               SkyReflection=1)
                vectors['ReliefSize'] = (relief.blueprint_get_size_x(), relief.blueprint_get_size_y(), 0, 0)
            if night:
                textures['Night'] = night
                scalars['NightStrength'] = q.get('nightStrength', .85)
            if clouds:
                textures['Clouds'] = clouds
                scalars.update(CloudShadow=q.get('cloudShadow', .55), CloudOpacity=q.get('cloudOpacity', 1))
        if row.get('meshSource'):
            scalars['MeshUV'] = 1
        mi = self.apply(self.instance('M_Body_' + key, self.master(kind)), kind, scalars, vectors, textures)
        cloud = None
        if clouds and kind == 'surface':
            cloud = self.apply(self.instance('M_Cloud_' + key, self.master('clouds')), 'clouds',
                               dict(CloudSize=clouds.blueprint_get_size_y(), CloudDetail=q.get('cloudDetail', .8),
                                    CloudOpacity=q.get('cloudOpacity', 1)), {}, {'Clouds': clouds})
        return mi, cloud

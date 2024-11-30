#version 450

//===STAGE IN===============================================
layout (location = 0) in StageLayout {
    vec3 color;
    vec3 normal;
    vec2 uv;
    vec3 worldPos;
} stageIn;
//==========================================================


//===GLOBAL=================================================
layout (set = 0, binding = 0) uniform SceneUBO {
    mat4 projection;
    mat4 view;
    vec3 position;
    float unused_0;
} scene;

//===MUST_MIRROR_gfx_types.h================================
#define BEET_MAX_LIGHT_COUNT 256
struct LightDesc {
    vec3 position;
    float unused_0;

    //GfxLight
    vec3 color;
    float radiusInner;
    float radiusOuter;
    float unused_1;
    float unused_2;
    float unused_3;
};

struct GfxDirectionalLight {
    vec3 direction;
    uint color;
};

layout (set = 0, binding = 1) uniform LightUBO {
    LightDesc lightDesc[BEET_MAX_LIGHT_COUNT];
    int lightDescCount;
    uint unused_0;
    uint unused_1;
    uint unused_2;
    GfxDirectionalLight directionalLight;
} lights;
//==========================================================

//===TEXTURES===============================================
layout (binding = 2) uniform sampler2D albedoMap;
layout (binding = 3) uniform sampler2D normalMap;
layout (binding = 4) uniform sampler2D aoMap;
layout (binding = 5) uniform sampler2D metallicMap;
layout (binding = 6) uniform sampler2D roughnessMap;
//==========================================================


//===CONSTANTS==============================================
const float PI = 3.14159265359;
//==========================================================

//===OUT====================================================
layout (location = 0) out vec4 outFragColor;
//==========================================================

//===FUNCTIONS==============================================
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, stageIn.uv).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(stageIn.worldPos);
    vec3 Q2 = dFdy(stageIn.worldPos);
    vec2 st1 = dFdx(stageIn.uv);
    vec2 st2 = dFdy(stageIn.uv);

    vec3 N = normalize(stageIn.normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


uint pack_vec4f_to_uint(const vec4 vec) {
    uint packedValue = 0;
    packedValue |= (uint(vec.x * 0xFF) << 24);
    packedValue |= (uint(vec.y * 0xFF) << 16);
    packedValue |= (uint(vec.z * 0xFF) << 8);
    packedValue |= (uint(vec.w * 0xFF) << 0);
    return packedValue;
}

vec4 unpack_uint_to_vec4f(uint packedValue) {
    return vec4(
        float((packedValue >> 24) & 0xFF) / 255.0f,
        float((packedValue >> 16) & 0xFF) / 255.0f,
        float((packedValue >> 8) & 0xFF) / 255.0f,
        float((packedValue >> 0) & 0xFF) / 255.0f
    );
}

void main()
{
//    GfxDirectionalLight directionalLight;
//    directionalLight.color = pack_vec4f_to_uint(vec4(207.0 / 255.0, 221.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0));
//    directionalLight.direction = vec3(0.25, 1, 0.25);


    vec3 albedo = pow(texture(albedoMap, stageIn.uv).rgb, vec3(2.2));
    float metallic = texture(metallicMap, stageIn.uv).r;
    float roughness = texture(roughnessMap, stageIn.uv).r;
    float ao = texture(aoMap, stageIn.uv).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(scene.position - stageIn.worldPos);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    // point lights
    for (int i = 0; i < lights.lightDescCount; ++i)
    {
        vec3 lightPos = lights.lightDesc[i].position.xyz;
        float lightRadiusOuter = lights.lightDesc[i].radiusOuter;
        float lightRadiusInner = lights.lightDesc[i].radiusInner;
        vec3 L = normalize(lightPos - stageIn.worldPos);
        float distance = length(lightPos - stageIn.worldPos);

        // Fade based on distance
        float fade = smoothstep(lightRadiusOuter, lightRadiusInner, distance);
        float attenuation = fade / (distance * distance);
        vec3 radiance = lights.lightDesc[i].color.xyz * attenuation;

        // Cook-Torrance BRDF
        vec3 H = normalize(V + L);
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // Prevent division by zero
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // directional light
    vec3 dirL = normalize(lights.directionalLight.direction);
    vec3 radianceDir = unpack_uint_to_vec4f(lights.directionalLight.color).rgb;

    float NdotL = max(dot(N, dirL), 0.0);
    vec3 H = normalize(V + dirL);
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, dirL, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, dirL), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    Lo += (kD * albedo / PI + specular) * radianceDir * NdotL;

    // Ambient lighting
    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0 / 2.2));

    outFragColor = vec4(color, 1.0);
}

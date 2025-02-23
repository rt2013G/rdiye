#version 420 core

// DEFINES
#define PI32 3.14159265359f
#define MAX_LIGHTS 16

struct point_light
{
    vec3 position;
    vec3 color;
};

in vs_out
{
    vec3 fragment_position;
    vec3 normal;
    vec2 tex_coords;
} vertex_output;

// RENDERING VARIABLES
uniform mat4 view;
uniform vec3 viewer_position;
uniform vec3 directional_light;
uniform point_light lights[MAX_LIGHTS];
uniform int light_count;
uniform sampler2D albedo_map;
uniform sampler2D normal_map;
uniform sampler2D metallic_map;
uniform sampler2D roughness_map;
uniform sampler2D ambient_occlusion_map;

// NOTE, TODO: temporary for gltf format
// r=occlusion, g=roughness, b=metalness
uniform int use_metallic_roughness;

// SHADOW VARIABLES
#define CASCADE_COUNT 3
uniform float near_plane_cascades[CASCADE_COUNT];
uniform float far_plane_cascades[CASCADE_COUNT];
uniform sampler2DArray shadow_map;

layout (std140) uniform light_space_matrices_ubo
{
    mat4 light_space_matrices[CASCADE_COUNT];
};

out vec4 frag_color;

// NOTE: copy pasted from LearnOGL site
// TODO: re-read how the TBN matrix works
vec3 GetNormalFromMap()
{
    vec3 tangent_normal = texture(normal_map, vertex_output.tex_coords).xyz * 2.0f - 1.0f;
    vec3 Q1 = dFdx(vertex_output.fragment_position);
    vec3 Q2 = dFdy(vertex_output.fragment_position);
    vec2 st1 = dFdx(vertex_output.tex_coords);
    vec2 st2 = dFdy(vertex_output.tex_coords);

    vec3 N = normalize(vertex_output.normal);
    vec3 T = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    vec3 result = normalize(TBN * tangent_normal);
    return(result);
}

float CalculateShadow(vec3 world_position)
{
    vec4 view_position = view * vec4(world_position, 1.0f);
    float depth_value = abs(view_position.z);

    int layer = CASCADE_COUNT;
    for(int i = 0; i < CASCADE_COUNT; i++)
    {
        if(depth_value < far_plane_cascades[i])
        {
            layer = i;
            break;
        }
    }
    if(layer == CASCADE_COUNT)
    {
        layer = CASCADE_COUNT - 1;
    }
    vec4 light_space_pos = light_space_matrices[layer] * vec4(world_position, 1.0f);
    vec3 projected_coords = light_space_pos.xyz / light_space_pos.w;
    projected_coords = projected_coords * 0.5f + 0.5f;
    float current_depth = projected_coords.z;
    if(current_depth > 1.0f)
    {
        return 0.0f;
    }

    //vec3 normal = normalize(vertex_output.normal);
    vec3 normal = GetNormalFromMap();
    float bias = max(0.05f * (1.0f - dot(normal, directional_light)), 0.005f);
    if(layer == CASCADE_COUNT - 1)
    {
        bias *= 1.0f / (far_plane_cascades[layer] * 0.5f);
    }
    else
    {
        bias *= 1.0f / ((far_plane_cascades[layer] - near_plane_cascades[layer]) * 0.5f);
    }

    float shadow = 0.0f;
    vec2 texel_size = 1.0f / vec2(textureSize(shadow_map, 0));
    for(int x = -1; x <= 1; x++)
    {
        for(int y = -1; y <= 1; y++)
        {
            float pcf_depth = texture(shadow_map, vec3(projected_coords.xy + vec2(x, y) * texel_size, layer)).r;
            shadow += (current_depth - bias) > pcf_depth ? 1.0f : 0.0f;
        }
    }
    shadow /= 9.0f;

    return(shadow);
}

float NormalDistributionGGX(vec3 normal, vec3 halfway, float roughness)
{
    // NOTE: based on observations by epic games the lighting seems more correct
    //       if we square the roughness first
    float a = roughness * roughness;
    float a2 = a * a;
    float n_dot_h = clamp(dot(normal, halfway), 0.0f, 1.0f);
    float n_dot_h2 = n_dot_h * n_dot_h;

    float denominator = (n_dot_h2 * (a2 - 1.0f) + 1.0f);
    denominator = PI32 * denominator * denominator;

    float result = a2 / denominator;
    return(result);
}

float GeometryShlickGGX(vec3 normal, vec3 view_direction, float roughness)
{
    float n_dot_v = clamp(dot(normal, view_direction), 0.0f, 1.0f);

    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;
    float denominator = n_dot_v * (1.0f - k) + k;
    float result = (n_dot_v / denominator);
    
    return(result);
}

float GeometrySmithShlick(vec3 normal, vec3 view, vec3 light, float roughness)
{
    float ggx1 = GeometryShlickGGX(normal, view, roughness);
    float ggx2 = GeometryShlickGGX(normal, light, roughness);
    float result = ggx1 * ggx2;

    return(result);
}

vec3 FresnelSchlick(float cos_theta, vec3 F0)
{
    vec3 result = F0 + (1.0f - F0) * pow(clamp(1.0f - cos_theta, 0.0f, 1.0f), 5.0f);
    return(result);
}

void main()
{
    // NOTE: albedo and AO textures are usually in srgb space, so we need to first
    //       convert them to linear space
    vec3 albedo = pow(texture(albedo_map, vertex_output.tex_coords).rgb, vec3(2.2f));
    vec3 normal = GetNormalFromMap();
    float metallic = texture(metallic_map, vertex_output.tex_coords).r;
    float roughness = texture(roughness_map, vertex_output.tex_coords).r;
    float AO = pow(texture(ambient_occlusion_map, vertex_output.tex_coords).r, 2.2f);
    if(use_metallic_roughness > 0)
    {
        vec4 tmp = texture(metallic_map, vertex_output.tex_coords);
        AO = tmp.r;
        roughness = tmp.g;
        metallic = tmp.b;
    }
    
    vec3 view = normalize(viewer_position - vertex_output.fragment_position);

    vec3 F0 = vec3(0.04f);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0f);
    for(int i = 0; i < light_count; i++)
    {
        vec3 light_direction = normalize(lights[i].position - vertex_output.fragment_position);
        vec3 halfway = normalize(light_direction + view);
        float cos_theta = clamp(dot(halfway, view), 0.0f, 1.0f);

        float distance = length(lights[i].position - vertex_output.fragment_position);
        float attenuation = 1.0f / (distance);
        vec3 radiance = lights[i].color * attenuation;

        float D = NormalDistributionGGX(normal, halfway, roughness);
        float G = GeometrySmithShlick(normal, view, light_direction, roughness);
        vec3 F = FresnelSchlick(cos_theta, F0);
        vec3 numerator = D * F * G;
        float n_dot_v = clamp(dot(normal, view), 0.0f, 1.0f);
        float n_dot_l = clamp(dot(normal, light_direction), 0.0f, 1.0f);
        float denominator = (4.0f * n_dot_v * n_dot_l) + 0.00001f;
        vec3 specular = numerator / denominator;

        vec3 k_specular = F;
        vec3 k_diffuse = vec3(1.0f) - k_specular;
        // NOTE: if the material is metallic, the diffuse is 0 since metals don't refract light
        k_diffuse *= (1.0f - metallic);
        Lo += ((k_diffuse * albedo / PI32) + specular) * radiance * n_dot_l;
    }
    vec3 ambient = vec3(0.03f) * albedo * AO;
    vec3 color = ambient + Lo;

    // TODO: add sunlight and re-add shadows

    // NOTE, IMPORTANT: ALL PBR CALCULATIONS MUST BE DONE IN LINEAR SPACE!!!
    //                  then convert to srgb space at the end
    // TODO: post processing
    // NOTE: HDR -> LDR, then gamma correct
    // NOTE: gamma = 2.2f, exposure = 0.2f
    color = vec3(1.0f) - exp(-color * 0.2f);
    color = pow(color, vec3(1.0f / 2.2f));
    frag_color = vec4(color, 1.0f);
}
#include "glad/glad.h"
// NOTE: include glad before glfw
#include "GLFW/glfw3.h"

#include "thirdparty/stb/stb_image.h"

#include "rd_lib.h"
#include "camera.h"
#include "rd_mesh.h"
#include "temp_data.h"
#include "shader.hpp"

struct game_state
{
    GLFWwindow *window;
    u16 window_width;
    u16 window_height;
    f64 last_time;
    f32 delta_time;
    b32 is_running;
    game_camera player_camera;
};

GLOBAL u16 default_window_width = 1920;
GLOBAL u16 default_window_height = 1080;
GLOBAL game_state state;
GLOBAL bool has_mouse_moved = false;
GLOBAL vec2 mouse_last_movement;

#define SHADOW_CASCADES_COUNT 3
GLOBAL i32 render_debug_quad_layer = SHADOW_CASCADES_COUNT;

GLOBAL b32 camera_mode_ortho = false;

#define POST_PROCESSING_ENABLED 1
GLOBAL f32 tonemapping_exposure = 0.5f;
GLOBAL b32 bloom_enabled = 1;

// 0 = no tonemapping, 1 = just hdr->ldr correction, 
// 2 = aces tonemapper
GLOBAL i32 tonemapper_choice = 2;

void ResizeCallback(GLFWwindow *window, i32 width, i32 height)
{
    glViewport(0, 0, width, height);
    state.window_width = width;
    state.window_height = height;
}

void MouseCallback(GLFWwindow *window, f64 pos_x, f64 pos_y)
{
    vec2 p = Vec2((f32)pos_x, (f32)pos_y);
    if (!has_mouse_moved)
    {
        mouse_last_movement = p;
        has_mouse_moved = true;
    }
    vec2 offset = Vec2(p.x - mouse_last_movement.x, 
                       mouse_last_movement.y - p.y);
    mouse_last_movement = p;
    ProcessCameraMouse(&state.player_camera, offset);
}

void ProcessInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        state.is_running = false;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        if(camera_mode_ortho)
        {
            MoveCamera(&state.player_camera, UP, state.delta_time); 
        }
        else
        {
            MoveCamera(&state.player_camera, FORWARD, state.delta_time);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        if(camera_mode_ortho)
        {
            MoveCamera(&state.player_camera, DOWN, state.delta_time);
        }
        else
        {
            MoveCamera(&state.player_camera, BACKWARD, state.delta_time);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        MoveCamera(&state.player_camera, LEFT, state.delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        MoveCamera(&state.player_camera, RIGHT, state.delta_time);
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        state.player_camera.settings.speed = 10.0f;
    }
    else
    {
       state.player_camera.settings.speed = 2.5f; 
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        {
            render_debug_quad_layer = 0;
        }
        else if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        {
            render_debug_quad_layer = 1;
        }
        else if(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        {
            render_debug_quad_layer = 2;
        }
        else
        {
            render_debug_quad_layer = SHADOW_CASCADES_COUNT;
        }

        if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            if(tonemapping_exposure > 0.0f)
            {
                tonemapping_exposure -= 0.001f;
            }
            else
            {
                tonemapping_exposure = 0.0f;
            }
            std::cout << "exposure: " << tonemapping_exposure << std::endl;
        }
        else if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            tonemapping_exposure += 0.001f;
            std::cout << "exposure: " << tonemapping_exposure << std::endl;
        }

        if(glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        {
            bloom_enabled = 0;
        }
        else
        {
            bloom_enabled = 1;
        }
    }
    if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        camera_mode_ortho = !camera_mode_ortho;
    }
    if(glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        if(glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
        {
            tonemapper_choice = 0;
        }
        else if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        {
            tonemapper_choice = 1;
        }
        else if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        {
            tonemapper_choice = 2;
        }
    }
}

void ScrollCallback(GLFWwindow *window, f64 offset_x, f64 offset_y)
{
    ProcessCameraScroll(&state.player_camera, (f32)offset_y);
}

GLuint LoadCubemap(std::string *face_names, u32 face_count) {
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
    i32 texture_width, texture_height, channel_count;
    u8 *data;
    for (u32 i = 0; i < face_count; i++) {
        std::string path = ASSETS_FOLDER + face_names[i];
        data = stbi_load(path.c_str(), &texture_width, &texture_height, &channel_count, 0);
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return(texture_id);
}

struct frustum
{
    vec4 corners[8];
    vec3 center;
};
vec3 GetFrustumCenter(frustum f)
{
    vec3 center = Vec3(0.0f, 0.0f, 0.0f);
    for(u32 i = 0; i < ArrayCount(f.corners); i++)
    {
        center += Vec3(f.corners[i]);
    }
    center /= ArrayCount(f.corners);

    return(center);
}

frustum GetFrustumInWorldSpace(mat4x4 projection_mul_view)
{
    mat4x4 to_world_space = Inverse(projection_mul_view);

    frustum f = {{
        Vec4(-1.0f, -1.0f, -1.0f, 1.0f),
        Vec4(-1.0f, -1.0f, 1.0f, 1.0f),
        Vec4(-1.0f, 1.0f, -1.0f, 1.0f),
        Vec4(-1.0f, 1.0f, 1.0f, 1.0f),
        Vec4(1.0f, -1.0f, -1.0f, 1.0f),
        Vec4(1.0f, -1.0f, 1.0f, 1.0f),
        Vec4(1.0f, 1.0f, -1.0f, 1.0f),
        Vec4(1.0f, 1.0f, 1.0f, 1.0f),
    }};

    for(u32 i = 0; i < ArrayCount(f.corners); i++)
    {
        f.corners[i] = to_world_space * f.corners[i];
        f.corners[i] /= f.corners[i].w;
    }

    f.center = GetFrustumCenter(f);

    return(f);
}

frustum GetFrustumInWorldSpace(mat4x4 projection, mat4x4 view)
{
    mat4x4 projection_mul_view = projection * view;
    frustum result = GetFrustumInWorldSpace(projection_mul_view);

    return(result);
}

// NOTE: gets the light space matrix for a single cascade
mat4x4 GetLightSpaceMatrix(vec3 light_direction, u16 width, u16 height, f32 near_plane, f32 far_plane)
{
    mat4x4 projection = Perspective(100.0f, width / height, near_plane, far_plane);
    mat4x4 view = CameraViewMatrix(&state.player_camera);
    frustum f = GetFrustumInWorldSpace(projection, view);

    mat4x4 light_view = LookAt(f.center + light_direction, f.center, Vec3(0.0f, 1.0f, 0.0f));
    f32 min_x = FLT_MAX;
    f32 min_y = FLT_MAX;
    f32 min_z = FLT_MAX;
    f32 max_x = FLT_MIN;
    f32 max_y = FLT_MIN;
    f32 max_z = FLT_MIN;

    for(u32 i = 0; i < ArrayCount(f.corners); i++)
    {
        vec4 light_space_corner = light_view * f.corners[i];
        min_x = Minimum(min_x, light_space_corner.x);
        min_y = Minimum(min_y, light_space_corner.y);
        min_z = Minimum(min_z, light_space_corner.z);
        max_x = Maximum(max_x, light_space_corner.x);
        max_y = Maximum(max_y, light_space_corner.y);
        max_z = Maximum(max_z, light_space_corner.z);
    }

    f32 z_multiplier = 10.0f;
    if(min_z < 0)
    {
        min_z *= z_multiplier;
    }
    else
    {
        min_z /= z_multiplier; 
    }
    if(max_z < 0)
    {
        max_z /= z_multiplier;
    }
    else
    {
        max_z *= z_multiplier; 
    }
    rect3 frustum_rect = Rect3(min_x, min_y, min_z, max_x, max_y, max_z);
    mat4x4 light_projection = Orthographic(frustum_rect);
    mat4x4 result = light_projection * light_view;

    return(result);
}

#define MAX_POINT_LIGHTS 16
GLOBAL vec3 default_light_color = Vec3(4.0f);
struct point_light
{
    vec3 position;
    vec3 color;
};

void SetLightInShader(ShaderProgram *shader, point_light *light_list, u32 light_count)
{
    shader->set_int("light_count", light_count);
    for(u32 i = 0; i < light_count; i++)
    {
        std::string position_str = "lights[" + std::to_string(i) + "].position";
        std::string color_str = "lights[" + std::to_string(i) + "].color";
        shader->set_vec3(position_str.c_str(), light_list[i].position);
        shader->set_vec3(color_str.c_str(), light_list[i].color);
    }
}

void SetTransform(ShaderProgram *shader, vec3 position, vec3 scale = Vec3(1.0f), mat4x4 rotation = Identity())
{
    mat4x4 model = Translation(position) * rotation * Scaling(scale);
    shader->set_mat4("model", model);
    shader->set_mat3("normal_matrix", Mat3x3(Transpose(Inverse(model)))); 
}
void SetTransform(ShaderProgram *shader, vec3 position, f32 scale, mat4x4 rotation)
{
    SetTransform(shader, position, Vec3(scale), rotation);
}
void SetTransform(ShaderProgram *shader, vec3 position, f32 scale)
{
    SetTransform(shader, position, Vec3(scale)); 
}

struct scene_node
{
    vec3 position;
    mat4x4 rotation;
    vec3 scale;

    std::vector<mesh_data> mesh_list;

    // TODO: temporary
    b32 gltf_model;
};

scene_node SceneNode(vec3 position, mat4x4 rotation, vec3 scale, std::vector<mesh_data> &mesh_list, b32 gltf_model = 0)
{
    scene_node result;
    result.position = position;
    result.rotation = rotation;
    result.scale = scale;
    result.mesh_list = mesh_list;
    result.gltf_model = gltf_model;

    return(result);
}

void RenderNodeList(ShaderProgram *shader, scene_node *node_list, u32 node_count)
{
    for(u32 node_index = 0; node_index < node_count; node_index++)
    {
        scene_node node = node_list[node_index];
        SetTransform(shader, node.position, node.scale, node.rotation);
        shader->set_int("use_metallic_roughness", node.gltf_model);
        for(mesh_data &mesh : node.mesh_list)
        {
            RenderMesh(&mesh);
        }
    }
}

struct bloom_mip
{
    GLuint texture_id;
    vec2 screen_size;
};

int main(void)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(default_window_width, default_window_height, "hello!", NULL, NULL);
    if (window == NULL)
    {
        return(1);
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        return(1);
    }
    glViewport(0, 0, default_window_width, default_window_height);

    glfwSwapInterval(0);
    glfwSetFramebufferSizeCallback(window, ResizeCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glEnable(GL_DEPTH_TEST);

    state.is_running = true;
    state.window_width = default_window_width;
    state.window_height = default_window_height;
    state.last_time = 0.0f;
    state.delta_time = 0.0f;
    state.window = window;
    state.player_camera = DefaultCamera();
    mouse_last_movement = Vec2(state.window_width / 2.0f, state.window_height / 2.0f);

    GLuint render_fbo, render_fbo_texture, render_fbo_depth_stencil;
    glGenFramebuffers(1, &render_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
    
    glGenTextures(1, &render_fbo_texture);
    glBindTexture(GL_TEXTURE_2D, render_fbo_texture);
    // TODO: framebuffer size is not updated with window resizing
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, state.window_width, state.window_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_fbo_texture, 0);
    
    glGenRenderbuffers(1, &render_fbo_depth_stencil);
    glBindRenderbuffer(GL_RENDERBUFFER, render_fbo_depth_stencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, state.window_width, state.window_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_fbo_depth_stencil);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Assert("framebuffer incomplete");
    }

    u32 depth_map_resolution = 4096;
    GLuint light_fbo, light_depth_maps;
    glGenFramebuffers(1, &light_fbo);
    glGenTextures(1, &light_depth_maps);
    glBindTexture(GL_TEXTURE_2D_ARRAY, light_depth_maps);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F,
                    depth_map_resolution, depth_map_resolution, SHADOW_CASCADES_COUNT, 
                    0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    vec4 border_color = Vec4(1.0f);
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, &border_color.e[0]);
    
    glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, light_depth_maps, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Assert("framebuffer incomplete");
    }

    GLuint matrices_ubo;
    glGenBuffers(1, &matrices_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, matrices_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(mat4x4) * SHADOW_CASCADES_COUNT, NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, matrices_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // NOTE: abritrary values based on the sponza scene
    // TODO: consider changing them at run time
    //       and maybe adding more cascades
    f32 light_near_plane = 1.0f;
    f32 light_far_plane = 100.0f;
    f32 cascades_ratio = pow(light_far_plane / light_near_plane, 1.0f / SHADOW_CASCADES_COUNT);
    f32 near_plane_cascades[SHADOW_CASCADES_COUNT] = 
    {
        light_near_plane, light_near_plane * cascades_ratio, light_near_plane * cascades_ratio * cascades_ratio
    };
    f32 far_plane_cascades[SHADOW_CASCADES_COUNT] =
    {
        light_near_plane * cascades_ratio, light_near_plane * cascades_ratio * cascades_ratio, light_far_plane
    };

    #define BLOOM_MIP_COUNT 5
    bloom_mip bloom_mip_list[BLOOM_MIP_COUNT]{};
    vec2 mip_size = Vec2(state.window_width, state.window_height);
    for(i32 mip_index = 0; mip_index < BLOOM_MIP_COUNT; mip_index++)
    {
        mip_size *= 0.5f;
        bloom_mip_list[mip_index].screen_size = mip_size;
        glGenTextures(1, &(bloom_mip_list[mip_index].texture_id));
        glBindTexture(GL_TEXTURE_2D, bloom_mip_list[mip_index].texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, (i32) mip_size.x, (i32) mip_size.y,
                     0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    GLuint bloom_fbo;
    glGenFramebuffers(1, &bloom_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloom_mip_list[0].texture_id, 0);
    u32 bloom_fbo_attachments[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, bloom_fbo_attachments);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        Assert("framebuffer incomplete");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint quad_vao, quad_vbo;
    glGenVertexArrays(1, &quad_vao);
    glGenBuffers(1, &quad_vbo);
    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTICES), &QUAD_VERTICES[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void *)(3 * sizeof(f32)));

    GLuint screen_quad_vao, screen_quad_vbo;
    glGenVertexArrays(1, &screen_quad_vao);
    glGenBuffers(1, &screen_quad_vbo);
    glBindVertexArray(screen_quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, screen_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SCREEN_QUAD_VERTICES), &SCREEN_QUAD_VERTICES[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void *)(2 * sizeof(f32)));

    ShaderProgram pbr_shader = ShaderProgram("src/shaders/vs.glsl", "src/shaders/pbr.fs.glsl");
    ShaderProgram light_shader = ShaderProgram("src/shaders/lighting.vs.glsl", "src/shaders/lighting.fs.glsl");
    ShaderProgram skybox_shader("src/shaders/skybox_vs.glsl", "src/shaders/skybox_fs.glsl");
    ShaderProgram shadow_shader("src/shaders/shadow_map.vs.glsl", "src/shaders/shadow_map.fs.glsl", 
                                "src/shaders/shadow_map.gs.glsl");
    ShaderProgram debug_quad_shader("src/shaders/debug_quad.vs.glsl", "src/shaders/debug_quad.fs.glsl");
    ShaderProgram postprocessing_shader("src/shaders/postprocessing.vs.glsl", "src/shaders/postprocessing.fs.glsl");
    ShaderProgram downsampler_shader("src/shaders/sampler.vs.glsl", "src/shaders/downsampler.fs.glsl");
    ShaderProgram upsampler_shader("src/shaders/sampler.vs.glsl", "src/shaders/upsampler.fs.glsl");

    std::string cubemap_faces[] =
    {
        "skybox/right.jpg",
        "skybox/left.jpg",
        "skybox/top.jpg",
        "skybox/bottom.jpg",
        "skybox/front.jpg",
        "skybox/back.jpg",
    };
    GLuint skybox_cubemap = LoadCubemap(cubemap_faces, ArrayCount(cubemap_faces));
    GLuint black_texture = LoadTexture(TEXTURE_DEFAULT_BLACK);
    GLuint white_texture = LoadTexture(TEXTURE_DEFAULT_WHITE); 
    GLuint default_normal_texture = LoadTexture(TEXTURE_DEFAULT_NORMAL_MAP);
    GLuint wood_diffuse = LoadTexture("wood.png");
    pbr_texture_group wood_textures =
    {
        wood_diffuse, default_normal_texture,
        black_texture, black_texture, white_texture,
    };
    pbr_texture_group rusted_iron_textures;
    rusted_iron_textures.albedo = LoadTexture("rusted_iron/albedo.png");
    rusted_iron_textures.normal = LoadTexture("rusted_iron/normal.png");
    rusted_iron_textures.metallic = LoadTexture("rusted_iron/metallic.png");
    rusted_iron_textures.roughness = LoadTexture("rusted_iron/roughness.png");
    rusted_iron_textures.ao = white_texture;

    mesh_data sky_mesh = MeshDataUntextured(sizeof(SKYBOX_VERTICES) / (sizeof(f32) * 3), &SKYBOX_VERTICES[0]);
    mesh_data plane_mesh = MeshData(sizeof(PLANE_VERTICES) / (sizeof(f32) * 8), &PLANE_VERTICES[0]);
    plane_mesh.textures = wood_textures;

    mesh_data light_mesh = MeshDataUntextured(sizeof(LIGHT_CUBE_VERTICES) / (sizeof(f32) * 3), &LIGHT_CUBE_VERTICES[0]);
    vec3 sun_position = Vec3(-4.0f, 100.0f, -4.0f);
    vec3 sun_direction = Normalize(sun_position);
    vec3 sun_intensity = Vec3(4000.0f);
    point_light light_list[MAX_POINT_LIGHTS] =
    {
        {Vec3(-0.5f, 2.5f, -0.5f), default_light_color},
        {Vec3(-1.25f, 1.0f, -1.25f), default_light_color},
        {Vec3(-2.0f, 0.5f, -1.75f), default_light_color},
        {Vec3(1.75f, 1.0f, 1.0f), default_light_color},
        {Vec3(1.5f, 1.5f, -1.625f), default_light_color},

        // NOTE: keep sun as the last light in the array
        {sun_position, sun_intensity},
    };
    u32 light_count = 6;

    std::vector<mesh_data> sponza_mesh_list = std::vector<mesh_data>();
    LoadModel(sponza_mesh_list, "sponza_khronos/Sponza.gltf");
    scene_node sponza_node = 
    {
        Vec3(0.0f, 0.0f, 0.0f), Identity(), Vec3(0.01f), sponza_mesh_list, 1,
    };

    std::vector<mesh_data> backpack_mesh_list = std::vector<mesh_data>();
    LoadModel(backpack_mesh_list, "backpack/backpack.obj");
    scene_node backpack_node = 
    {
        Vec3(-0.5f, 0.5f, -2.0f), RotationY(DegreesToRadians(-45.0f)), Vec3(0.25f), backpack_mesh_list, 0,
    };

    mesh_data cube_mesh = MeshData(sizeof(CUBE_VERTICES_TEXTURED) / (sizeof(f32) * 8), &CUBE_VERTICES_TEXTURED[0]);
    cube_mesh.textures = rusted_iron_textures;
    std::vector<mesh_data> cube_mesh_list = {cube_mesh};
    vec3 cube_positions[4] = 
    {
        Vec3(0.5f, 2.0f, 1.5f),
        Vec3(-0.2f, 1.0f, -1.0f),
        Vec3(2.0f, 1.5f, 0.5f),
        Vec3(0.5f, 3.0f, -1.0f),
    };
    scene_node cube_nodes[4] =
    {
        SceneNode(cube_positions[0], Identity(), Vec3(0.1f), cube_mesh_list),
        SceneNode(cube_positions[1], Identity(), Vec3(0.1f), cube_mesh_list),
        SceneNode(cube_positions[2], Identity(), Vec3(0.1f), cube_mesh_list),
        SceneNode(cube_positions[3], Identity(), Vec3(0.1f), cube_mesh_list),
    };
    
    scene_node big_rotating_cube =
    {
        Vec3(-1.0f, 2.0f, -1.0f), Identity(), Vec3(0.5f), cube_mesh_list, 0,
    };
    
    scene_node render_list[7] =
    {
        sponza_node, backpack_node, big_rotating_cube,
        cube_nodes[0], cube_nodes[1], cube_nodes[2], cube_nodes[3],
    };
    u32 render_list_count = 7;

    while(state.is_running)
    {
        f32 current_time = glfwGetTime();
        state.delta_time = current_time - state.last_time;
        state.last_time = current_time;

        ProcessInput(state.window);

        // NOTE: this is just a silly thing i pulled out
        //       of my a** to simulate a day/night cycle
        // TODO: real day/light cycle, sunlight, atmospheric scattering
        f32 day_time_sine = Sine(current_time / 10.0f);
        f32 day_time = Maximum((day_time_sine + 1.0f) / 2.0f, 0.01f);
        day_time *= day_time;
        light_list[light_count - 1].color = Hadamard(Vec3(day_time), sun_intensity);
        light_list[light_count - 1].position += Vec3(day_time_sine / 10.0f) * state.delta_time;
        sun_direction = Normalize(light_list[light_count - 1].position);

        // TODO: fix the function to rotate around a point
        render_list[2].rotation = render_list[2].rotation *
                                  RotationP(RotationY(DegreesToRadians(30) * state.delta_time), Vec3(0.0f, 0.0f, 0.0f)) *
                                  RotationZ(DegreesToRadians(15) * state.delta_time) * 
                                  RotationX(DegreesToRadians(45) * state.delta_time);

        mat4x4 light_spaces_matrices[SHADOW_CASCADES_COUNT] =
        {
            Transpose(GetLightSpaceMatrix(sun_direction, state.window_width, state.window_height, 
                                near_plane_cascades[0], far_plane_cascades[0])),
            Transpose(GetLightSpaceMatrix(sun_direction, state.window_width, state.window_height, 
                                near_plane_cascades[1], far_plane_cascades[1])),
            Transpose(GetLightSpaceMatrix(sun_direction, state.window_width, state.window_height, 
                                near_plane_cascades[2], far_plane_cascades[2])),
        };
        glBindBuffer(GL_UNIFORM_BUFFER, matrices_ubo);
        for (u32 i = 0; i < ArrayCount(light_spaces_matrices); i++)
        {
            glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4x4) * i, sizeof(mat4x4), &light_spaces_matrices[i]);
        }
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glEnable(GL_DEPTH_CLAMP);
        glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, depth_map_resolution, depth_map_resolution);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);
        shadow_shader.use();
        RenderNodeList(&shadow_shader, render_list, render_list_count);
        glCullFace(GL_BACK);

#if POST_PROCESSING_ENABLED
        glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
#else
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
        glViewport(0, 0, state.window_width, state.window_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_CLAMP);

        mat4x4 perspective_projection = PerspectiveProjection(&state.player_camera, state.window_width, state.window_height);
        mat4x4 projection = perspective_projection;
        // TODO: this is a temporary orthographic camera mode added for the funsies
        //       check for a better way of implementing it
        if(camera_mode_ortho)
        {
            f32 view_volume_scale = state.player_camera.settings.FOV / 3;
            f32 aspect_ratio = state.window_width / state.window_height;
            f32 far_plane = state.player_camera.settings.far_plane;
            projection = Orthographic(-aspect_ratio * view_volume_scale, -view_volume_scale, -far_plane,
                                      aspect_ratio * view_volume_scale, view_volume_scale, far_plane);
        }
        mat4x4 view = CameraViewMatrix(&state.player_camera);
        mat4x4 projection_mul_view = projection * view;

        // TODO: add normal matrix to the shaders to fix normals on non-uniform transforms
        mat4x4 model = Identity();
        light_shader.use();
        light_shader.set_mat4("projection_mul_view", projection_mul_view);
        // TODO: include light emitters in the render list
        for(u32 i = 0; i < light_count; i++)
        {
            model = Translation(light_list[i].position) * Scaling(0.1f);
            light_shader.set_mat4("model", model);
            light_shader.set_vec3("light_color", light_list[i].color);
            RenderMesh(&light_mesh);
        }
        
        pbr_shader.use();
        pbr_shader.set_mat4("projection_mul_view", projection_mul_view);
        pbr_shader.set_mat4("view", view);
        pbr_shader.set_vec3("viewer_position", state.player_camera.position);
        pbr_shader.set_int("albedo_map", 0);
        pbr_shader.set_int("normal_map", 1);
        pbr_shader.set_int("metallic_map", 2);
        pbr_shader.set_int("roughness_map", 3);
        pbr_shader.set_int("ambient_occlusion_map", 4);
        pbr_shader.set_vec3("sun_direction", sun_direction);
        pbr_shader.set_int("shadow_map", 5);
        pbr_shader.set_int("use_metallic_roughness", 0);
        pbr_shader.set_float("near_plane_cascades[0]", near_plane_cascades[0]);
        pbr_shader.set_float("near_plane_cascades[1]", near_plane_cascades[1]);
        pbr_shader.set_float("near_plane_cascades[2]", near_plane_cascades[2]);
        pbr_shader.set_float("far_plane_cascades[0]", far_plane_cascades[0]);
        pbr_shader.set_float("far_plane_cascades[1]", far_plane_cascades[1]);
        pbr_shader.set_float("far_plane_cascades[2]", far_plane_cascades[2]);
        SetLightInShader(&pbr_shader, light_list, light_count);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D_ARRAY, light_depth_maps);
        RenderNodeList(&pbr_shader, render_list, render_list_count);

        skybox_shader.use();
        view = Mat4x4(Mat3x3(CameraViewMatrix(&state.player_camera)));
        projection_mul_view = perspective_projection * view;
        skybox_shader.set_mat4("projection_mul_view", projection_mul_view);
        glDepthFunc(GL_LEQUAL);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_cubemap);
        skybox_shader.set_int("skybox_cubemap", 0);
        RenderMesh(&sky_mesh);
        glDepthFunc(GL_LESS);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo);
        downsampler_shader.use();
        downsampler_shader.set_int("source_texture", 0);
        downsampler_shader.set_vec2("source_resolution", Vec2(state.window_width, state.window_height));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, render_fbo_texture);
        glDisable(GL_BLEND);
        for(i32 i = 0; i < BLOOM_MIP_COUNT; i++)
        {
            vec2 dim = bloom_mip_list[i].screen_size;
            glViewport(0, 0, dim.x, dim.y);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
                                   bloom_mip_list[i].texture_id, 0);

            glBindVertexArray(screen_quad_vao);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            downsampler_shader.set_vec2("source_resolution", dim);
            glBindTexture(GL_TEXTURE_2D, bloom_mip_list[i].texture_id);
        }

        upsampler_shader.use();
        upsampler_shader.set_int("source_texture", 0);
        // NOTE, TODO: the radius should be different for the width and height
        //             as the blur can become noticeably wrong especially on 21:9 viewports
        //             the vertical filters_radius in particular should be multiplied by
        //             the aspect ratio  
        upsampler_shader.set_float("filter_radius", 0.005f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);
        for(i32 i = BLOOM_MIP_COUNT - 1; i > 0; i--)
        {
            bloom_mip mip = bloom_mip_list[i];
            bloom_mip next_mip = bloom_mip_list[i - 1];
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mip.texture_id);
            glViewport(0, 0, (i32)next_mip.screen_size.x, (i32)next_mip.screen_size.y);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, next_mip.texture_id, 0);
        
            glBindVertexArray(screen_quad_vao);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        glBindVertexArray(0);

#if POST_PROCESSING_ENABLED
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, state.window_width, state.window_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        postprocessing_shader.use();
        postprocessing_shader.set_int("screen_texture", 0);
        postprocessing_shader.set_int("bloom_texture", 1);
        postprocessing_shader.set_float("exposure", tonemapping_exposure);
        postprocessing_shader.set_int("tonemapper_choice", tonemapper_choice);

        // NOTE: post processing options at the moment don't actually save
        //       any computational time, this flag just helps
        //       seeing the visual differences
        // TODO: graphics settings, different compiled shaders
        //       for different settings
        postprocessing_shader.set_int("bloom_enabled", bloom_enabled);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, render_fbo_texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, bloom_mip_list[0].texture_id);
        glBindVertexArray(screen_quad_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnable(GL_DEPTH_TEST);
#endif

        if(render_debug_quad_layer < SHADOW_CASCADES_COUNT)
        {
            debug_quad_shader.use();
            debug_quad_shader.set_int("layer", render_debug_quad_layer);
            debug_quad_shader.set_float("near_plane", near_plane_cascades[render_debug_quad_layer]);
            debug_quad_shader.set_float("far_plane", far_plane_cascades[render_debug_quad_layer]);
            debug_quad_shader.set_int("shadow_map", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, light_depth_maps);
            glBindVertexArray(quad_vao);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);
        }

        glfwSwapBuffers(state.window);
        glfwPollEvents();

        current_time = glfwGetTime();
        state.delta_time = current_time - state.last_time;
#if 0
        std::cout << "sine: " << day_time_sine << ", day_time: " << day_time << std::endl;
#else
        std::cout << "frame delta: " << (state.delta_time * 1000.0f) << "ms, " << (1.0f / state.delta_time) << "fps" << std::endl;
#endif
    }
    glfwTerminate();
    return(0);
}
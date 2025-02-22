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
    }
    if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        camera_mode_ortho = !camera_mode_ortho;
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
    mat4x4 projection = Perspective(state.player_camera.settings.FOV, width / height, near_plane, far_plane);
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

struct point_light
{
    vec3 position;
    vec3 color;
};

#define MAX_POINT_LIGHTS 16
GLOBAL vec3 default_light_color = Vec3(10.0f, 10.0f, 10.0f);
struct scene
{
    std::vector<mesh_data> mesh_list;
    point_light *light_list;
    u32 light_count;
};

void RenderScene(scene *s)
{
    for(mesh_data &mesh : s->mesh_list)
    {
        RenderMesh(&mesh);
    }
}

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
void SetTransform(ShaderProgram *shader, vec3 position, f32 scale)
{
    SetTransform(shader, position, Vec3(scale)); 
}

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

    //ShaderProgram phong_shader = ShaderProgram("src/shaders/vs.glsl", "src/shaders/phong.fs.glsl");
    ShaderProgram pbr_shader = ShaderProgram("src/shaders/vs.glsl", "src/shaders/pbr.fs.glsl");
    ShaderProgram light_shader = ShaderProgram("src/shaders/lighting.vs.glsl", "src/shaders/lighting.fs.glsl");
    ShaderProgram skybox_shader("src/shaders/skybox_vs.glsl", "src/shaders/skybox_fs.glsl");
    ShaderProgram shadow_shader("src/shaders/shadow_map.vs.glsl", "src/shaders/shadow_map.fs.glsl", 
                                "src/shaders/shadow_map.gs.glsl");
    ShaderProgram debug_quad_shader("src/shaders/debug_quad.vs.glsl", "src/shaders/debug_quad.fs.glsl");

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
    mesh_data cube_mesh = MeshData(sizeof(CUBE_VERTICES_TEXTURED) / (sizeof(f32) * 8), &CUBE_VERTICES_TEXTURED[0]);
    cube_mesh.textures = rusted_iron_textures;
    mesh_data plane_mesh = MeshData(sizeof(PLANE_VERTICES) / (sizeof(f32) * 8), &PLANE_VERTICES[0]);
    plane_mesh.textures = wood_textures;
    mesh_data light_mesh = MeshDataUntextured(sizeof(LIGHT_CUBE_VERTICES) / (sizeof(f32) * 3), &LIGHT_CUBE_VERTICES[0]); 

    // TODO: we're technically passing the directional light POSITION to the shaders
    //       rather than the direction, this is incorrect but it works if we assume
    //       that the light is always looking at the center of the scene and we negate it
    //       in the shader. however, it should be fixed at some point
    vec3 dir_light_pos = Vec3(-10.0f, 30.0f, -10.0f);
    vec3 directional_light = Normalize(dir_light_pos);

    vec3 transparent_cube_positions[4] =
    {
        Vec3(0.5f, 2.0f, 1.5f),
        Vec3(-0.2f, 1.0f, -1.0f),
        Vec3(2.0f, 1.5f, 0.5f),
        Vec3(0.5f, 3.0f, -1.0f),
    };
    f32 transparent_cube_size = 0.8f;

    u32 depth_map_resolution = 4096;
    GLuint light_fbo, light_depth_maps;
    glGenFramebuffers(1, &light_fbo);
    glGenTextures(1, &light_depth_maps);
    glBindTexture(GL_TEXTURE_2D_ARRAY, light_depth_maps);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F,
                    depth_map_resolution, depth_map_resolution, SHADOW_CASCADES_COUNT + 1, 
                    0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
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
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint matrices_ubo;
    glGenBuffers(1, &matrices_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, matrices_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(mat4x4) * SHADOW_CASCADES_COUNT, NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, matrices_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // NOTE: the 0.5f is a value added to prevent the nearest cascade to have too high of a precision
    //       while leaving the farthest cascades with having to cover too large of an area
    f32 near_plane = state.player_camera.settings.near_plane + 0.5f;
    f32 far_plane = state.player_camera.settings.far_plane;
    f32 shadow_cascades_ratio = pow(far_plane / near_plane, 1.0f / SHADOW_CASCADES_COUNT);
    f32 near_plane_cascades[3] = {near_plane, near_plane * shadow_cascades_ratio, near_plane * shadow_cascades_ratio * shadow_cascades_ratio};
    f32 far_plane_cascades[3] = {near_plane * shadow_cascades_ratio, near_plane * shadow_cascades_ratio * shadow_cascades_ratio, far_plane};
    near_plane -= 0.5f;

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

    mat4x4 big_cube_model = Translation(-3.0f, 5.0f, -3.0f);
    mat4x4 big_cube_scale = Scaling(3.5f);

    vec3 backpack_scale = Vec3(0.1f, 0.1f, 0.1f);
    mat4x4 backpack_rotation = RotationY(DegreesToRadians(-45.0f));
    vec3 backpack_position = Vec3(-0.5f, 0.5f, -2.0f);

    std::vector<mesh_data> mesh_list = std::vector<mesh_data>();
    LoadModel(mesh_list, "backpack/backpack.obj");

    point_light light_list[MAX_POINT_LIGHTS] =
    {
        {Vec3(-5.0f, 200.0f, -5.0f), Vec3(1000.0f, 1000.0f, 1000.0f)},
        {Vec3(2.5f, 1.5f, -2.5f), default_light_color},
        {Vec3(-2.0f, 0.5f, -1.5f), default_light_color},
        {Vec3(-1.0f, 1.0f, -2.0f), default_light_color},
        {Vec3(2.0f, 2.0f, -1.5f), default_light_color},
        {Vec3(-3.0f, 1.5f, -2.5f), default_light_color},
        {Vec3(1.0f, 1.0f, 2.0f), default_light_color},
        {Vec3(2.5f, 1.5f, -2.5f), default_light_color},
        {Vec3(-2.0f, 2.0f, 1.5f), default_light_color},

        {Vec3(-4.0f, 1.25f, -0.75f), default_light_color},
        {Vec3(6.0f, 2.0f, 1.0f), default_light_color},
        {Vec3(-3.75f, 8.0f, -10.5f), default_light_color},
        {Vec3(0.75f, 3.5f, 2.5f), default_light_color},
        {Vec3(1.0f, 0.5f, 7.0f), default_light_color},
        {Vec3(3.25f, 10.0f, -1.5f), default_light_color},
        {Vec3(-1.0f, 5.0f, 2.0f), default_light_color},
    };
    u32 light_count = 16;

    scene main_scene = {
        mesh_list,
        light_list,
        light_count
    };

    while(state.is_running)
    {
        f32 current_time = glfwGetTime();
        state.delta_time = current_time - state.last_time;
        state.last_time = current_time;

        ProcessInput(state.window);

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4x4 light_spaces_matrices[3] =
        {
            Transpose(GetLightSpaceMatrix(directional_light, state.window_width, state.window_height, 
                                near_plane_cascades[0], far_plane_cascades[0])),
            Transpose(GetLightSpaceMatrix(directional_light, state.window_width, state.window_height, 
                                near_plane_cascades[1], far_plane_cascades[1])),
            Transpose(GetLightSpaceMatrix(directional_light, state.window_width, state.window_height, 
                                near_plane_cascades[2], far_plane_cascades[2])),
        };

        glBindBuffer(GL_UNIFORM_BUFFER, matrices_ubo);
        for (u32 i = 0; i < ArrayCount(light_spaces_matrices); i++)
        {
            glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4x4) * i, sizeof(mat4x4), &light_spaces_matrices[i]);
        }
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glEnable(GL_DEPTH_CLAMP);
        shadow_shader.use();
        glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_TEXTURE_2D_ARRAY, light_depth_maps, 0);
        glViewport(0, 0, depth_map_resolution, depth_map_resolution);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);

        vec3 plane_position = Vec3(0.0f);
        mat4x4 model = Translation(plane_position);
        shadow_shader.set_mat4("model", model);
        RenderMesh(&plane_mesh);
        vec3 cube_position = Vec3(0.0f);
        for(u32 i = 0; i < ArrayCount(transparent_cube_positions); i++)
        {
            cube_position = transparent_cube_positions[i];
            model = Translation(cube_position) * Scaling(transparent_cube_size);
            shadow_shader.set_mat4("model", model);
            RenderMesh(&cube_mesh);
        }

        // TODO: fix the function to rotate around a point
        big_cube_model = big_cube_model * RotationP(RotationY(DegreesToRadians(30) * state.delta_time), Vec3(0.0f, 0.0f, 0.0f));
        model = big_cube_model * big_cube_scale;
        shadow_shader.set_mat4("model", model);
        RenderMesh(&cube_mesh);

        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
            projection = Orthographic(-aspect_ratio * view_volume_scale, -view_volume_scale, -far_plane,
                                      aspect_ratio * view_volume_scale, view_volume_scale, far_plane);
        }
        mat4x4 view = CameraViewMatrix(&state.player_camera);
        mat4x4 projection_mul_view = projection * view;

        // TODO: add normal matrix to the shaders to fix normals on non-uniform transforms
        model = Identity();
        light_shader.use();
        light_shader.set_mat4("projection_mul_view", projection_mul_view);
        for(u32 i = 0; i < main_scene.light_count; i++)
        {
            model = Translation(main_scene.light_list[i].position) * Scaling(0.1f);
            light_shader.set_mat4("model", model);
            light_shader.set_vec3("light_color", main_scene.light_list[i].color);
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
        pbr_shader.set_vec3("directional_light", directional_light);
        pbr_shader.set_int("shadow_map", 5);
        pbr_shader.set_float("near_plane_cascades[0]", near_plane_cascades[0]);
        pbr_shader.set_float("near_plane_cascades[1]", near_plane_cascades[1]);
        pbr_shader.set_float("near_plane_cascades[2]", near_plane_cascades[2]);
        pbr_shader.set_float("far_plane_cascades[0]", far_plane_cascades[0]);
        pbr_shader.set_float("far_plane_cascades[1]", far_plane_cascades[1]);
        pbr_shader.set_float("far_plane_cascades[2]", far_plane_cascades[2]);
        SetLightInShader(&pbr_shader, main_scene.light_list, main_scene.light_count);

        SetShaderPBRTextures(&wood_textures);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D_ARRAY, light_depth_maps);
        SetTransform(&pbr_shader, plane_position);
        RenderMesh(&plane_mesh);

        SetShaderPBRTextures(&rusted_iron_textures);
        model = big_cube_model * big_cube_scale;
        pbr_shader.set_mat4("model", model);
        pbr_shader.set_mat3("normal_matrix", Mat3x3(Transpose(Inverse(model))));
        RenderMesh(&cube_mesh);

        for(u32 i = 0; i < ArrayCount(transparent_cube_positions); i++)
        {
            SetTransform(&pbr_shader, transparent_cube_positions[i], transparent_cube_size);
            RenderMesh(&cube_mesh);
        }

        SetTransform(&pbr_shader, backpack_position, backpack_scale, backpack_rotation);
        RenderScene(&main_scene);

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
        std::cout << "frame delta: " << (state.delta_time * 1000.0f) << "ms, " << (1.0f / state.delta_time) << "fps" << std::endl;
    }
    glfwTerminate();
    return(0);
}
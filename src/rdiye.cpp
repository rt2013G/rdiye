#include "glad/glad.h"
// NOTE: include glad before glfw
#include "GLFW/glfw3.h"

#include "thirdparty/stb/stb_image.h"

#include "rdiye_lib.h"
#include "camera.h"

#include "data.hpp"
#include "model.hpp"
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

GLOBAL u16 window_width = 1920;
GLOBAL u16 window_height = 1080;
GLOBAL game_state state;
GLOBAL bool has_mouse_moved = false;
GLOBAL vec2 mouse_last_movement;

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
        MoveCamera(&state.player_camera, FORWARD, state.delta_time);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        MoveCamera(&state.player_camera, BACKWARD, state.delta_time);
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
        state.player_camera.settings.speed = 5.0f;
    }
    else
    {
       state.player_camera.settings.speed = 2.5f; 
    }
}

void ScrollCallback(GLFWwindow *window, f64 offset_x, f64 offset_y)
{
    ProcessCameraScroll(&state.player_camera, (f32)offset_y);
}

GLuint LoadTexture(std::string filename)
{
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    i32 texture_width, texture_height, channel_count;
    stbi_set_flip_vertically_on_load(true);

    std::string path = ASSETS_FOLDER + filename;
    u8 *data = stbi_load(path.c_str(), &texture_width, &texture_height, &channel_count, 0);
    if(data)
    {
        GLenum format;
        if(channel_count == 1)
        {
            format = GL_RED;
        }
        else if(channel_count == 3)
        {
            format = GL_RGB;
        }
        else if(channel_count == 4)
        {
            format = GL_RGBA;
        }
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, texture_width, texture_height, 
                     0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        Assert(!"failed to load texture");
    }

    stbi_image_free(data);
    stbi_set_flip_vertically_on_load(false);

    return(texture_id);
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

int main(void)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(window_width, window_height, "hello!", NULL, NULL);
    if (window == NULL)
    {
        return(1);
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        return(1);
    }
    glViewport(0, 0, window_width, window_height);

    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, ResizeCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glEnable(GL_DEPTH_TEST);

    state.is_running = true;
    state.window_width = window_width;
    state.window_height = window_height;
    state.last_time = 0.0f;
    state.delta_time = 0.0f;
    state.window = window;
    state.player_camera = DefaultCamera();
    mouse_last_movement = Vec2(state.window_width / 2.0f, state.window_height / 2.0f);

    ShaderProgram object_shader = ShaderProgram("src/shaders/rtr/vs.glsl", "src/shaders/rtr/fs.glsl");
    ShaderProgram light_shader = ShaderProgram("src/shaders/rtr/lighting.vs.glsl", "src/shaders/rtr/lighting.fs.glsl");
    ShaderProgram skybox_shader("src/shaders/rtr/skybox_vs.glsl", "src/shaders/rtr/skybox_fs.glsl");

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
    GLuint cube_diffuse = LoadTexture("container2.png");
    GLuint cube_specular = LoadTexture("container2_s.png");
    GLuint wood_diffuse = LoadTexture("wood.png");

    GLuint skybox_vao, skybox_vbo;
    glGenVertexArrays(1, &skybox_vao);
    glGenBuffers(1, &skybox_vbo);
    glBindVertexArray(skybox_vao);
    glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(SKYBOX_VERTICES), &SKYBOX_VERTICES[0], GL_STATIC_DRAW);
    GLsizei skybox_triangles_count = sizeof(SKYBOX_VERTICES) / (sizeof(f32) * 3); 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void *)0);
    glBindVertexArray(0);

    GLuint cube_vao, cube_vbo;
    glGenVertexArrays(1, &cube_vao);
    glGenBuffers(1, &cube_vbo);
    glBindVertexArray(cube_vao);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES_TEXTURED), &CUBE_VERTICES_TEXTURED[0], GL_STATIC_DRAW);
    GLsizei cube_triangles_count = sizeof(CUBE_VERTICES_TEXTURED) / (sizeof(f32) * 8); 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void *)(3 * sizeof(f32)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void *)(6 * sizeof(f32)));
    glBindVertexArray(0);

    GLuint plane_vao, plane_vbo;
    glGenVertexArrays(1, &plane_vao);
    glGenBuffers(1, &plane_vbo);
    glBindVertexArray(plane_vao);
    glBindBuffer(GL_ARRAY_BUFFER, plane_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PLANE_VERTICES), &PLANE_VERTICES[0], GL_STATIC_DRAW);
    GLsizei plane_triangles_count = sizeof(PLANE_VERTICES) / (sizeof(f32) * 8); 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void *)(3 * sizeof(f32)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(f32), (void *)(6 * sizeof(f32)));
    glBindVertexArray(0);
    
    GLuint light_vao, light_vbo;
    glGenVertexArrays(1, &light_vao);
    glGenBuffers(1, &light_vbo);
    glBindVertexArray(light_vao);
    glBindBuffer(GL_ARRAY_BUFFER, light_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(LIGHT_CUBE_VERTICES), LIGHT_CUBE_VERTICES, GL_STATIC_DRAW);
    GLsizei light_triangles_count = sizeof(LIGHT_CUBE_VERTICES) / (sizeof(f32) * 3);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void *)0);
    glBindVertexArray(0);

    #define MAX_LIGHTS 16
    vec3 light_positions[MAX_LIGHTS] = {};
    light_positions[0] = Vec3(2.0f, 2.0f, -1.5f);
    light_positions[1] = Vec3(2.5f, 1.5f, -2.5f);
    light_positions[2] = Vec3(-2.0f, 0.5f, -1.5f);
    light_positions[3] = Vec3(3.0f, 1.5f, 2.5f);
    light_positions[4] = Vec3(-1.0f, 1.0f, -2.0f);
    i32 point_light_count = 5;

    vec3 transparent_cube_positions[4] =
    {
        Vec3(0.5f, 2.0f, 1.5f),
        Vec3(-0.2f, 1.0f, -1.0f),
        Vec3(2.0f, 1.5f, 0.5f),
        Vec3(0.5f, 3.0f, -1.0f),
    };
    f32 transparent_cube_size = 0.8f;
    
    while(state.is_running)
    {
        f32 current_time = glfwGetTime();
        state.delta_time = current_time - state.last_time;
        state.last_time = current_time;

        ProcessInput(state.window);

        glClearColor(0.2, 0.2, 0.2, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4x4 projection = PerspectiveProjection(&state.player_camera, state.window_width, state.window_height);
        mat4x4 view = CameraViewMatrix(&state.player_camera);
        mat4x4 projection_mul_view = projection * view;
        
        // TODO: add normal matrix to the shaders to fix normals on non-uniform transforms
        mat4x4 model = Identity();
        light_shader.use();
        light_shader.set_mat4("projection_mul_view", projection_mul_view);
        for(i32 i = 0; i < point_light_count; i++)
        {
            model = Translation(light_positions[i]) * Scaling(0.1f);
            light_shader.set_mat4("model", model);
            glBindVertexArray(light_vao);
            glDrawArrays(GL_TRIANGLES, 0, light_triangles_count);
        }
        glBindVertexArray(0);
        
        object_shader.use();
        object_shader.set_mat4("projection_mul_view", projection_mul_view);
        object_shader.set_vec3("viewer_position", state.player_camera.position);
        object_shader.set_int("diffuse_texture", 0);
        object_shader.set_int("specular_texture", 1);
        object_shader.set_float("shininess", 64.0f);

        object_shader.set_vec3("directional_light", Vec3(-0.2f, -1.0f, -0.3f));
        object_shader.set_int("light_count", point_light_count);
        for(i32 i = 0; i < point_light_count; i++)
        {
            std::string position_str = "lights[" + std::to_string(i) + "].position";
            object_shader.set_vec3(position_str.c_str(), light_positions[i]);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, wood_diffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, black_texture);
        vec3 plane_position = Vec3(0.0f, 0.0f, 0.0f);
        model = Translation(plane_position);
        object_shader.set_mat4("model", model);
        glBindVertexArray(plane_vao);
        glDrawArrays(GL_TRIANGLES, 0, plane_triangles_count);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cube_diffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, cube_specular);
        vec3 cube_position = Vec3(1.0f, 1.0f, -3.0f);
        model = Translation(cube_position) * Scaling(0.5f);
        object_shader.set_mat4("model", model);
        glBindVertexArray(cube_vao);
        glDrawArrays(GL_TRIANGLES, 0, cube_triangles_count);
        glBindVertexArray(0);

        for(u32 i = 0; i < ArrayCount(transparent_cube_positions); i++)
        {
            cube_position = transparent_cube_positions[i];
            model = Translation(cube_position) * Scaling(transparent_cube_size);
            object_shader.set_mat4("model", model);
            glBindVertexArray(cube_vao);
            glDrawArrays(GL_TRIANGLES, 0, cube_triangles_count);
            glBindVertexArray(0);
        }

        skybox_shader.use();
        view = Mat4x4(Mat3x3(CameraViewMatrix(&state.player_camera)));
        projection_mul_view = projection * view;
        skybox_shader.set_mat4("projection_mul_view", projection_mul_view);
        glDepthFunc(GL_LEQUAL);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_cubemap);
        skybox_shader.set_int("skybox_cubemap", 0);
        glBindVertexArray(skybox_vao);
        glDrawArrays(GL_TRIANGLES, 0, skybox_triangles_count);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        glBindTexture(GL_TEXTURE_2D, 0);

        glfwSwapBuffers(state.window);
        glfwPollEvents(); 
    }
    glfwTerminate();
    return(0);
}
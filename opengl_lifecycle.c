#define GL_SILENCE_DEPRECATION 1

#include "opengl_lifecycle.h"

#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <cglm/cglm.h>

#include "bmp.h"
#include "cube.h"
#include "shader.h"
#include "texture_uv.h"
#include "utils.h"

bool gl_init(SDL_Window** window, SDL_GLContext** context) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unable to initialize SDL: %s",
                     SDL_GetError());
        return false;
    }

    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

    // Version
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    // Double Buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    u32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    _Bool fullscreen = false;
    if (fullscreen) {
        flags |= SDL_WINDOW_MAXIMIZED;
    }

    const char window_title[] = "hello";
    const u16 window_width = 800;
    const u16 window_height = 600;
    *window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, window_width,
                               window_height, flags);

    if (!*window) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unable to create window: %s",
                     SDL_GetError());
        return false;
    }

    *context = SDL_GL_CreateContext(*window);
    if (!*context) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "Unable to create context from window: %s",
                     SDL_GetError());
        return false;
    }

    glEnable(GL_DEPTH_TEST);

    return true;
}

static void gl_triangle_setup() {
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
}

static GLuint gl_scene_setup() {
    gl_triangle_setup();

    return shader_load("resources/texture_vertex.glsl",
                       "resources/texture_fragment.glsl");
}

static void gl_triangle_vertex_buffer(GLuint* vertex_buffer,
                                      GLuint* color_buffer) {
    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, vertex_buffer);
    // The following commands will talk about our 'vertexbuffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, *vertex_buffer);
    // Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertex_buffer_data),
                 cube_vertex_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(1, color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, *color_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texture_uv_buffer_data),
                 texture_uv_buffer_data, GL_STATIC_DRAW);
}

static void texture_load(GLuint* texture_id) {
    const usize data_capacity = 1000 * 1000;
    u8* data = ogl_malloc(data_capacity);
    usize data_len = 0, width = 0, height = 0, img_size = 0, data_pos = 0;

    bmp_load("resources/uvtemplate.bmp", &data, data_capacity, &data_len,
             &width, &height, &img_size, &data_pos);
    u8* const img_data = data + data_pos;
    printf(
        "BMP: data_len=%zu, width=%zu height=%zu img_size=%zu data_pos=%zu "
        "img_data[0]=%hhu\n",
        data_len, width, height, img_size, data_pos, img_data[0]);

    glGenTextures(1, texture_id);
    glBindTexture(GL_TEXTURE_2D, *texture_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR,
                 GL_UNSIGNED_BYTE, img_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void gl_loop(SDL_Window* window) {
    GLuint texture_id;
    texture_load(&texture_id);

    const GLuint program_id = gl_scene_setup();
    /* u16 fps_desired = 60; */
    /* u16 frame_rate = 1000 / fps_desired; */

    /* u32 start = 0, end = 0, elapsed_time = 0; */

    GLuint vertex_buffer, color_buffer;
    gl_triangle_vertex_buffer(&vertex_buffer, &color_buffer);

    SDL_Event event;

    mat4 projection;
    glm_perspective(degree_to_radian(45.0f), 800 / 600.0, 0.1f, 100.0f,
                    projection);
    /* glm_ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 100.0f, projection); */

    mat4 view;
    vec3 eye = {4, 3, -3};
    vec3 center = {0, 0, 0};
    vec3 up = {0, 1, 0};
    glm_lookat(eye, center, up, view);

    mat4 model;
    glm_mat4_identity(model);

    mat4 mvp;
    glm_mat4_mul(projection, view, mvp);
    glm_mat4_mul(mvp, model, mvp);

    while (true) {
        /* start = SDL_GetTicks(); */

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return;
            }
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(program_id);

            // Pass matrix to glsl
            const GLuint matrix_id = glGetUniformLocation(program_id, "MVP");
            glUniformMatrix4fv(matrix_id, 1, GL_FALSE, (const f32*)mvp);

            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
            glVertexAttribPointer(
                0,  // attribute 0. No particular reason for 0, but must match
                    // the layout in the shader.
                3,         // size
                GL_FLOAT,  // type
                GL_FALSE,  // normalized?
                0,         // stride
                (void*)0   // array buffer offset
            );

            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
            glVertexAttribPointer(
                1,  // attribute 1. No particular reason for 1, but must match
                    // the layout in the shader.
                2,         // size
                GL_FLOAT,  // type
                GL_FALSE,  // normalized?
                0,         // stride
                (void*)0   // array buffer offset
            );

            glDrawArrays(GL_TRIANGLES, 0,
                         12 * 3);  // 6 squares = 12 triangles = 12*3 vertices
            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(0);

            SDL_GL_SwapWindow(window);
        }
        /* end = SDL_GetTicks(); */
        /* elapsed_time = end - start; */

        /* if (elapsed_time < frame_rate) SDL_Delay(frame_rate - elapsed_time);
         */
    }
}

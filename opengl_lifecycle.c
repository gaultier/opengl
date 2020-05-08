#define GL_SILENCE_DEPRECATION 1

#include "opengl_lifecycle.h"

#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <cglm/cglm.h>

#include "shader.h"
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

    return shader_load("resources/vertex_shader.glsl",
                       "resources/fragment_shader.glsl");
}

static GLuint gl_triangle_vertex_buffer() {
    static const f32 g_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };
    // This will identify our vertex buffer
    GLuint vertexbuffer;
    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &vertexbuffer);
    // The following commands will talk about our 'vertexbuffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
                 g_vertex_buffer_data, GL_STATIC_DRAW);

    return vertexbuffer;
}

void gl_loop(SDL_Window* window) {
    const GLuint program_id = gl_scene_setup();
    /* u16 fps_desired = 60; */
    /* u16 frame_rate = 1000 / fps_desired; */

    /* u32 start = 0, end = 0, elapsed_time = 0; */

    GLuint vertexbuffer = gl_triangle_vertex_buffer();
    SDL_Event event;

    mat4 projection;
    glm_perspective(degree_to_radian(45.0f), 800 / 600.0, 0.1f, 100.0f,
                    projection);
    /* glm_ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 100.0f, projection); */

    mat4 view;
    vec3 eye = {4, 3, 3};
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
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            glVertexAttribPointer(
                0,  // attribute 0. No particular reason for 0, but must match
                    // the layout in the shader.
                3,         // size
                GL_FLOAT,  // type
                GL_FALSE,  // normalized?
                0,         // stride
                (void*)0   // array buffer offset
            );
            // Draw the triangle !
            glDrawArrays(
                GL_TRIANGLES, 0,
                3);  // Starting from vertex 0; 3 vertices total -> 1 triangle
            glDisableVertexAttribArray(0);

            SDL_GL_SwapWindow(window);
        }
        /* end = SDL_GetTicks(); */
        /* elapsed_time = end - start; */

        /* if (elapsed_time < frame_rate) SDL_Delay(frame_rate - elapsed_time);
         */
    }
}

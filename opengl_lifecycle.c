#define GL_SILENCE_DEPRECATION 1

#include "opengl_lifecycle.h"

#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>

#include "types.h"

void gl_drop(SDL_Window* window, SDL_GLContext* context) {
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool gl_init(SDL_Window** window, SDL_GLContext** context) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unable to initialize SDL: %s",
                     SDL_GetError());
        return false;
    }
    atexit(SDL_Quit);

    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

    // Version
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    // Double Buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    _Bool fullscreen_ = false;
    u32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    if (fullscreen_) {
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

void gl_loop(SDL_Window* window) {
    u16 fps_desired = 60;
    u16 frame_rate = 1000 / fps_desired;

    u32 start = 0, end = 0, elapsed_time = 0;

    while (true) {
        start = SDL_GetTicks();

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        SDL_GL_SwapWindow(window);

        end = SDL_GetTicks();
        elapsed_time = end - start;

        if (elapsed_time < frame_rate) SDL_Delay(frame_rate - elapsed_time);
    }
}

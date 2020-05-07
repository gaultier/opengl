#define GL_SILENCE_DEPRECATION 1

#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#include "opengl_lifecycle.h"
#include "types.h"

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

int main() {
    SDL_Window* window;
    SDL_GLContext* context;
    if (!gl_init(&window, &context)) return 1;

    gl_loop(window);
    gl_drop(window, context);
}


#include <stdbool.h>

#include "opengl_lifecycle.h"
#include "utils.h"

int main() {
    SDL_Window* window;
    SDL_GLContext* context;
    if (!gl_init(&window, &context)) return 1;

    gl_loop(window);
}


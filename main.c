#include <stdbool.h>

#include "opengl_lifecycle.h"
#include "types.h"

int main() {
    SDL_Window* window;
    SDL_GLContext* context;
    if (!gl_init(&window, &context)) return 1;

    gl_loop(window);
    gl_drop(window, context);
}


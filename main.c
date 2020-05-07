#include <stdlib.h>
#include "SDL2/SDL.h"

int main() {
    atexit(SDL_Quit);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
         SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unable to initialize SDL: %s", SDL_GetError());
    }

    // Version
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

}

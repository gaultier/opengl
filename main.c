#include "SDL2/SDL.h"

int main() {
    atexit(SDL_Quit);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
         SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unable to initialize SDL: %s", SDL_GetError());
    }

}

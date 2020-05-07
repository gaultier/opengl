
#include <SDL2/SDL_video.h>
#include <stdbool.h>

struct SDL_Window;
struct SDL_GLContext;

void gl_drop(SDL_Window* window, SDL_GLContext* context);
bool gl_init(SDL_Window** window, SDL_GLContext** context);

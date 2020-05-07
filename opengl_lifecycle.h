
#include <stdbool.h>

struct SDL_Window;
typedef struct SDL_Window SDL_Window;
typedef void* SDL_GLContext;

void gl_drop(SDL_Window* window, SDL_GLContext* context);
bool gl_init(SDL_Window** window, SDL_GLContext** context);

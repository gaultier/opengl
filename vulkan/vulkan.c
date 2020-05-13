#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "../utils.h"

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unable to initialize SDL: %s",
                     SDL_GetError());
        exit(1);
    }

    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
    u32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;
    const char window_title[] = "hello";
    const u16 window_width = 1024;
    const u16 window_height = 768;
    SDL_Window* window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, window_width,
                                          window_height, flags);

    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unable to create window: %s",
                     SDL_GetError());
        exit(1);
    }

    u32 extension_count = 0;
    const char* extension_names[64] = {0};
    extension_names[extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;

    const VkApplicationInfo app = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                   .pApplicationName = "hello triangle",
                                   .apiVersion = VK_MAKE_VERSION(0, 0, 1)};

    u32 c = 64 - extension_count;
    if (!SDL_Vulkan_GetInstanceExtensions(window, &c,
                                          &extension_names[extension_count])) {
        fprintf(stderr, "SDL_GetVulkanInstanceExtensions failed: %s\n",
                SDL_GetError());
        exit(1);
    }

    printf("Instance extensions: count=%u\n", c);
    for (u32 i = 0; i < c; i++) {
        printf("Extension: %s\n", extension_names[c]);
    }
}

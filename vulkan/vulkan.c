#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "../utils.h"

#define MAX_EXTENSIONS 64

int main() {
    //
    // SDL init
    //
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Unable to initialize SDL: %s", SDL_GetError());
        exit(1);
    }

    //
    // Create window
    //
    u32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;
    const char window_title[] = "hello";
    const u16 window_width = 1024;
    const u16 window_height = 768;
    SDL_Window* window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED, window_width,
                                          window_height, flags);

    if (!window) {
        fprintf(stderr, "Unable to create window: %s", SDL_GetError());
        exit(1);
    }

    //
    // Get Vulkan extensions
    //
    u32 extension_count = 0;
    const char* extension_names[MAX_EXTENSIONS] = {0};
    extension_names[extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;

    const VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "hello triangle",
        .apiVersion = VK_MAKE_VERSION(0, 0, 1)};

    u32 detected_extension_count = 64 - extension_count;
    if (!SDL_Vulkan_GetInstanceExtensions(window, &detected_extension_count,
                                          &extension_names[extension_count])) {
        fprintf(stderr, "SDL_GetVulkanInstanceExtensions failed: %s\n",
                SDL_GetError());
        exit(1);
    }
    extension_count += detected_extension_count;

    printf("Instance extensions: count=%u\n", extension_count);
    for (u32 i = 0; i < extension_count; i++) {
        printf("Extension: %s\n", extension_names[i]);
    }

    //
    // Create Vulkan instance
    //
    VkInstanceCreateInfo instance_create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = extension_count,
        .ppEnabledExtensionNames = extension_names,
    };

    VkInstance instance;
    VkResult err = vkCreateInstance(&instance_create_info, NULL, &instance);
    if (err) {
        fprintf(stderr, "vkCreateInstance failed: %d\n", err);
        exit(1);
    }
}

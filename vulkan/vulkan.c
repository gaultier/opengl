#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <stdio.h>
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

    //
    // Create Vulkan surface
    //
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
        fprintf(stderr, "SDL_Vulkan_CreateSurface failed: %s\n",
                SDL_GetError());
        exit(1);
    }

    //
    // Create Vulkan physical device
    //
    VkPhysicalDevice gpu;
    u32 gpu_count;
    err = vkEnumeratePhysicalDevices(instance, &gpu_count, NULL);
    if (err) {
        fprintf(stderr, "vkEnumeratePhysicalDevices failed: %d\n", err);
        exit(1);
    }

    if (gpu_count == 0) {
        fprintf(stderr, "No GPUs detected\n");
        gpu = VK_NULL_HANDLE;
    } else {
        VkPhysicalDevice gpus[gpu_count];
        err = vkEnumeratePhysicalDevices(instance, &gpu_count, gpus);
        if (err) {
            fprintf(stderr, "vkEnumeratePhysicalDevices (2) failed: %d\n", err);
            exit(1);
        }
        gpu = gpus[0];
    }
    printf("GPUs detected: %u\n", gpu_count);

    //
    // Get information about the device
    //
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(gpu, &device_properties);

    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(gpu, &device_features);

    printf("GPU information: name=%s driver_version=%u\n",
           device_properties.deviceName, device_properties.driverVersion);

    //
    // Create command pool & queue
    //
    VkDevice device;
    VkQueue queue;
    VkCommandPool command_pool;

    u32 queue_family_index = UINT32_MAX;
    u32 queue_count;

    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, NULL);
    VkQueueFamilyProperties queue_properties[queue_count];
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count,

                                             queue_properties);

    if (queue_count == 0) {
        fprintf(stderr, "No queue family properties were found\n");
        exit(1);
    }
    printf("Found %u family properties\n", queue_count);

    for (u32 i = 0; i < queue_count; i++) {
        VkBool32 supported;
        vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &supported);

        if (supported &&
            (queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            queue_family_index = i;
            break;
        }
    }

    if (queue_family_index == UINT32_MAX) {
        fprintf(stderr, "No proper queue family found\n");
        exit(1);
    }

    {
        u32 extension_count = 0;

        const char* extension_names[MAX_EXTENSIONS];
        extension_names[extension_count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

        f32 queue_priorities[1] = {0.0};
        const VkDeviceQueueCreateInfo queue_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queue_family_index,
            .queueCount = 1,
            .pQueuePriorities = queue_priorities};

        VkDeviceCreateInfo device_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queue_info,
            .enabledExtensionCount = extension_count,
            .ppEnabledExtensionNames = extension_names,
        };

        err = vkCreateDevice(gpu, &device_create_info, NULL, &device);
        if (err) {
            fprintf(stderr, "vkCreateDevice failed: %d\n", err);
            exit(1);
        }

        vkGetDeviceQueue(device, queue_family_index, 0, &queue);

        const VkCommandPoolCreateInfo command_pool_create_info = {

            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = queue_family_index,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT};

        err = vkCreateCommandPool(device, &command_pool_create_info, NULL,
                                  &command_pool);

        if (err) {
            fprintf(stderr, "vkCreateCommandPool failed: %d\n", err);
            exit(1);
        }
    }

    //
    // Get color format
    //
    VkFormat format;
    VkColorSpaceKHR color_space;
    u32 format_count;

    err =
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, NULL);
    if (err) {
        fprintf(stderr, "vkGetPhysicalDeviceSurfaceFormatsKHR failed: %d\n",
                err);
        exit(1);
    }

    printf("Found %d formats\n", format_count);

    VkSurfaceFormatKHR formats[format_count];

    err = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count,
                                               formats);
    if (err) {
        fprintf(stderr, "vkGetPhysicalDeviceSurfaceFormatsKHR failed: %d\n",
                err);
        exit(1);
    }

    if (format_count == 0) {
        fprintf(stderr, "Found zero format\n");
        exit(1);
    }

    if (format_count == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
        printf(
            "Found only one format which is undefined,defaulting to "
            "VK_FORMAT_B8G8R8A8_SRGB\n");
        format = VK_FORMAT_B8G8R8A8_SRGB;
    } else {
        format = formats[0].format;
    }

    printf("Format: %d\n", format);
}

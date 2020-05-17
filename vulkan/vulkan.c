#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <assert.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "../utils.h"

#define MAX_EXTENSIONS 64
#define MAX_LAYERS 64

#define MAX_FRAMES_IN_FLIGHT 2

static SDL_Window* window_create() {
    SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "1");
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Unable to initialize SDL: %s", SDL_GetError());
        exit(1);
    }

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

    return window;
}

static void vk_get_extensions(SDL_Window* window,
                              const char* extension_names[MAX_EXTENSIONS],
                              u32* extension_count) {
    extension_names[*extension_count] = VK_KHR_SURFACE_EXTENSION_NAME;
    *extension_count = *extension_count + 1;

    u32 detected_extension_count = 64 - *extension_count;
    if (!SDL_Vulkan_GetInstanceExtensions(window, &detected_extension_count,
                                          &extension_names[*extension_count])) {
        fprintf(stderr, "SDL_GetVulkanInstanceExtensions failed: %s\n",
                SDL_GetError());
        exit(1);
    }
    *extension_count += detected_extension_count;

    printf("Instance extensions: count=%u\n", *extension_count);
    for (u32 i = 0; i < *extension_count; i++) {
        printf("Extension: %s\n", extension_names[i]);
    }
}

static void vk_get_validation_layers(const char* validation_layer) {
    VkLayerProperties layers[MAX_LAYERS];
    u32 layer_count;

    vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    assert(layer_count < MAX_LAYERS);
    printf("Available layers: %u\n", layer_count);

    vkEnumerateInstanceLayerProperties(&layer_count, layers);

    _Bool found = false;
    for (u32 i = 0; i < layer_count; i++) {
        printf("Available layer: %s\n", layers[i].layerName);

        if (strcmp(validation_layer, layers[i].layerName) == 0) {
            found = true;
            break;
        }
    }
    if (!found) {
        fprintf(stderr, "Validation layer not found\n");
    }
    printf("Validation layer found, activating\n");
}

static void vk_create_instance(VkInstance* instance,
                               const char* extension_names[MAX_EXTENSIONS],
                               u32 extension_count,
                               const char* validation_layer,
                               u32 validation_layer_count) {
    const VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = VK_MAKE_VERSION(1, 0, 0)};

    const VkInstanceCreateInfo instance_create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = extension_count,
        .ppEnabledExtensionNames = extension_names,
        .enabledLayerCount = validation_layer_count,
        .ppEnabledLayerNames = &validation_layer};

    assert(!vkCreateInstance(&instance_create_info, NULL, instance));
}

static void vk_create_physical_device(VkInstance* instance,
                                      VkPhysicalDevice* gpu) {
    u32 gpu_count;
    assert(!vkEnumeratePhysicalDevices(*instance, &gpu_count, NULL));

    if (gpu_count == 0) {
        fprintf(stderr, "No GPUs detected\n");
        exit(1);
    } else {
        VkPhysicalDevice gpus[gpu_count];
        assert(!vkEnumeratePhysicalDevices(*instance, &gpu_count, gpus));
        *gpu = gpus[0];
    }
    printf("GPUs detected: %u\n", gpu_count);
}

static u32 vk_find_queue_family(VkPhysicalDevice* gpu, VkSurfaceKHR* surface) {
    u32 queue_family_index = UINT32_MAX;
    u32 queue_count;

    vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queue_count, NULL);
    VkQueueFamilyProperties queue_properties[queue_count];
    vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queue_count,

                                             queue_properties);

    if (queue_count == 0) {
        fprintf(stderr, "No queue family properties were found\n");
        exit(1);
    }
    printf("Found %u family properties\n", queue_count);

    for (u32 i = 0; i < queue_count; i++) {
        VkBool32 supported;
        vkGetPhysicalDeviceSurfaceSupportKHR(*gpu, i, *surface, &supported);

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

    return queue_family_index;
}

static void vk_create_logical_device(VkPhysicalDevice* gpu,
                                     u32 queue_family_index, VkDevice* device) {
    u32 extension_count = 0;

    const char* extension_names[MAX_EXTENSIONS];
    extension_names[extension_count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

    f32 queue_priorities[1] = {0.0};
    const VkDeviceQueueCreateInfo queue_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queue_family_index,
        .queueCount = 1,
        .pQueuePriorities = queue_priorities};

    const VkDeviceCreateInfo device_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_info,
        .enabledExtensionCount = extension_count,
        .ppEnabledExtensionNames = extension_names,
    };

    assert(!vkCreateDevice(*gpu, &device_create_info, NULL, device));
}

static void vk_create_command_pool(VkDevice* device, u32 queue_family_index,
                                   VkCommandPool* command_pool) {
    const VkCommandPoolCreateInfo command_pool_create_info = {

        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = queue_family_index,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT};

    assert(!vkCreateCommandPool(*device, &command_pool_create_info, NULL,
                                command_pool));
}

static void vk_get_color_info(VkPhysicalDevice* gpu, VkSurfaceKHR* surface,
                              VkFormat* format, u32* format_count,
                              VkColorSpaceKHR* color_space) {
    assert(!vkGetPhysicalDeviceSurfaceFormatsKHR(*gpu, *surface, format_count,
                                                 NULL));

    printf("Found %d formats\n", *format_count);

    VkSurfaceFormatKHR formats[*format_count];

    assert(!vkGetPhysicalDeviceSurfaceFormatsKHR(*gpu, *surface, format_count,
                                                 formats));

    if (*format_count == 0) {
        fprintf(stderr, "Found zero format\n");
        exit(1);
    }

    if (*format_count == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
        printf(
            "Found only one format which is undefined,defaulting to "
            "VK_FORMAT_B8G8R8A8_SRGB\n");
        *format = VK_FORMAT_B8G8R8A8_SRGB;
    } else {
        *format = formats[0].format;
    }

    *color_space = formats[0].colorSpace;
    printf("Format: %d\n", *format);
    printf("Color space: %d\n", *color_space);
}

static void vk_create_shader_module(VkDevice* device, const char path[],
                                    u8* buffer, usize buffer_capacity,
                                    usize* buffer_len,
                                    VkShaderModule* shader_module) {
    if (file_read(path, buffer, buffer_capacity, buffer_len) != 0) {
        exit(errno);
    }

    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = *buffer_len,
        .pCode = (u32*)buffer,
    };

    assert(!vkCreateShaderModule(*device, &create_info, NULL, shader_module));
    memset(buffer, 0, buffer_capacity);

    printf("Created shader module for `%s`\n", path);
}

static void vk_create_shader_stages(
    VkShaderModule* vert_shader_module, VkShaderModule* frag_shader_module,
    VkPipelineShaderStageCreateInfo shader_stages[2]) {
    const VkPipelineShaderStageCreateInfo vert_shader_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = *vert_shader_module,
        .pName = "main"  // Entrypoint function
    };

    const VkPipelineShaderStageCreateInfo frag_shader_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = *frag_shader_module,
        .pName = "main"  // Entrypoint function
    };

    shader_stages[0] = vert_shader_stage_info;
    shader_stages[1] = frag_shader_stage_info;
}

static u32 memory_type_find(VkPhysicalDeviceMemoryProperties* memory_properties,
                            u32 type_flag,
                            VkMemoryPropertyFlags properties_flag) {
    for (u32 i = 0; i < memory_properties->memoryTypeCount; i++) {
        if ((type_flag & (1 << i)) &&
            (memory_properties->memoryTypes[i].propertyFlags &
             properties_flag) == properties_flag)
            return i;
    }
    assert(0);
}

int main() {
    // Create window
    SDL_Window* window = window_create();

    // get extensions
    u32 extension_count = 0;
    const char* extension_names[MAX_EXTENSIONS] = {0};
    vk_get_extensions(window, extension_names, &extension_count);

    // Get validation layers
    const char* validation_layer = "VK_LAYER_KHRONOS_validation";
    const char* const debug = getenv("DEBUG");
    if (debug) vk_get_validation_layers(validation_layer);

    // Create instance
    VkInstance instance;
    vk_create_instance(&instance, extension_names, extension_count,
                       debug ? validation_layer : NULL, debug ? 1 : 0);

    // Create Vulkan surface
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
        fprintf(stderr, "SDL_Vulkan_CreateSurface failed: %s\n",
                SDL_GetError());
        exit(1);
    }

    // Create physical device
    VkPhysicalDevice gpu;
    vk_create_physical_device(&instance, &gpu);

    // Find appropriate queue family
    const u32 queue_family_index = vk_find_queue_family(&gpu, &surface);

    // Create logical device
    VkDevice device;
    vk_create_logical_device(&gpu, queue_family_index, &device);

    // Create queue
    VkQueue queue;
    vkGetDeviceQueue(device, queue_family_index, 0, &queue);

    // Create command pool
    VkCommandPool command_pool;
    vk_create_command_pool(&device, queue_family_index, &command_pool);

    // Get color format
    VkFormat format;
    u32 format_count;
    VkColorSpaceKHR color_space;
    vk_get_color_info(&gpu, &surface, &format, &format_count, &color_space);

    // Set up shaders
    VkPipelineShaderStageCreateInfo shader_stages[2];
    const usize buffer_capacity = 10 * 1000;
    u8* buffer = ogl_malloc(buffer_capacity);
    usize buffer_len;

    VkShaderModule vert_shader_module;
    vk_create_shader_module(&device, "resources/triangle_vert.spv", buffer,
                            buffer_capacity, &buffer_len, &vert_shader_module);

    VkShaderModule frag_shader_module;
    vk_create_shader_module(&device, "resources/triangle_frag.spv", buffer,
                            buffer_capacity, &buffer_len, &frag_shader_module);

    vk_create_shader_stages(&vert_shader_module, &frag_shader_module,
                            shader_stages);

    VkSurfaceCapabilitiesKHR surface_capabilities;
    assert(!vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface,
                                                      &surface_capabilities));
    //
    // Swap chain
    //
    VkSwapchainKHR swapchain;
    u32 swapchain_image_count;
    VkExtent2D swapchain_extent;

    // Get drawing surface dimensions
    if (surface_capabilities.currentExtent.width == (UINT32_MAX)) {
        i32 w, h;
        SDL_GetWindowSize(window, &w, &h);
        printf("Window size as reported by the SDL: w=%d h=%d\n", w, h);
        swapchain_extent.width = (u32)w;
        swapchain_extent.height = (u32)h;
    } else {
        swapchain_extent = surface_capabilities.currentExtent;
        printf("Window size as reported by Vulkan: w=%u h=%u\n",
               swapchain_extent.width, swapchain_extent.height);
    }

    swapchain_image_count = surface_capabilities.minImageCount + 1;

    if ((surface_capabilities.maxImageCount > 0) &&
        (swapchain_image_count > surface_capabilities.maxImageCount)) {
        swapchain_image_count = surface_capabilities.maxImageCount;
    }

    const VkSwapchainCreateInfoKHR swapchain_create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = swapchain_image_count,
        .imageFormat = format,
        .imageColorSpace = color_space,
        .imageExtent = swapchain_extent,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = surface_capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .imageArrayLayers = 1,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = 1};

    assert(!vkCreateSwapchainKHR(device, &swapchain_create_info, NULL,
                                 &swapchain));

    printf("Swapchain image count: %u\n", swapchain_image_count);

    //
    // Fixed functions
    //

    const VkPipelineViewportStateCreateInfo viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    const VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
    };

    const VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f,
    };

    const VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_B_BIT,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
    };

    const VkPipelineColorBlendStateCreateInfo color_blending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
    };

    const VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    };

    VkPipelineLayout pipeline_layout;

    // Per vertex data
    struct Vertex {
        vec2 position;
        vec3 color;
    } vertices[] = {{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
    printf("Vertices: size=%lu count=%lu vertex_struct_size=%lu\n",
           sizeof(vertices), sizeof(vertices) / sizeof(vertices[0]),
           sizeof(vertices[0]));

    VkVertexInputBindingDescription vertex_binding_description = {
        .binding = 0,
        .stride = sizeof(struct Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription vertex_attribute_descriptions[2] = {
        // Metadata about the `position` field
        {.format = VK_FORMAT_R32G32_SFLOAT,
         .binding = 0,
         .location = 0,
         .offset = offsetof(struct Vertex, position)},

        // Metadata about the `color` field
        {.location = 1,
         .binding = 0,
         .format = VK_FORMAT_R32G32B32_SFLOAT,
         .offset = offsetof(struct Vertex, color)}};

    // Shader input
    const VkPipelineVertexInputStateCreateInfo vertex_input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .vertexAttributeDescriptionCount = 2,
        .pVertexBindingDescriptions = &vertex_binding_description,
        .pVertexAttributeDescriptions = vertex_attribute_descriptions,
    };

    const VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };
    assert(!vkCreatePipelineLayout(device, &pipeline_layout_create_info, NULL,
                                   &pipeline_layout));

    VkImage images[swapchain_image_count];
    VkImageView views[swapchain_image_count];
    VkFramebuffer frame_buffers[swapchain_image_count];

    assert(
        !vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, 0));

    VkImage swapchain_images[swapchain_image_count];
    assert(!vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count,
                                    swapchain_images));

    for (u32 i = 0; i < swapchain_image_count; i++)
        images[i] = swapchain_images[i];

    for (u32 i = 0; i < swapchain_image_count; i++) {
        VkImageViewCreateInfo color_attachment_view = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .format = format,
            .components =
                {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
            .subresourceRange =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .image = images[i],
        };

        assert(!vkCreateImageView(device, &color_attachment_view, NULL,
                                  &views[i]));

        printf("Initialized image view #%u\n", i);
    }

    //
    // Attachments
    //
    const VkAttachmentDescription attachment = {
        .format = format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    const VkAttachmentReference color_reference = {
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    const VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_reference,
    };

    const VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};

    const VkRenderPassCreateInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    VkRenderPass render_pass;
    assert(!vkCreateRenderPass(device, &render_pass_info, NULL, &render_pass));

    printf("Created render pass\n");

    // Dynamic state
    VkDynamicState dynamic_states[2] = {VK_DYNAMIC_STATE_VIEWPORT,
                                        VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic_states_create_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pDynamicStates = dynamic_states,
        .dynamicStateCount = ARR_SIZE(dynamic_states),
    };

    //
    // Graphics pipeline
    //
    VkPipeline graphics_pipeline;
    const VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shader_stages,
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &color_blending,
        .layout = pipeline_layout,
        .renderPass = render_pass,
        .pDynamicState = &dynamic_states_create_info,
    };

    assert(!vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info,
                                      NULL, &graphics_pipeline));
    printf("Created graphics pipeline\n");

    //
    // Frame buffers
    //
    for (u32 i = 0; i < swapchain_image_count; i++) {
        VkImageView attachments[1] = {
            [0] = views[i],
        };

        const VkFramebufferCreateInfo frame_buffer_create_info = {

            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = render_pass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = swapchain_extent.width,
            .height = swapchain_extent.height,
            .layers = 1,
        };

        assert(!vkCreateFramebuffer(device, &frame_buffer_create_info, NULL,
                                    &frame_buffers[i]));

        printf("Created frame buffer #%u\n", i);
    }

    //
    // Create semaphores
    //
    VkSemaphore image_available_semaphore[MAX_FRAMES_IN_FLIGHT],
        render_finished_semaphore[MAX_FRAMES_IN_FLIGHT];

    const VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    const VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT};

    VkFence in_flight_fences[MAX_FRAMES_IN_FLIGHT];
    VkFence images_in_flight_fences[MAX_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE};

    for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        assert(!vkCreateSemaphore(device, &semaphore_create_info, NULL,
                                  &image_available_semaphore[i]));
        printf("Created semaphore #%u 1/2\n", i);

        assert(!vkCreateSemaphore(device, &semaphore_create_info, NULL,
                                  &render_finished_semaphore[i]));
        printf("Created semaphore #%u 2/2\n", i);

        assert(!vkCreateFence(device, &fence_create_info, NULL,
                              &in_flight_fences[i]));
        printf("Created fence #%u\n", i);
    }

    //
    // Vertex buffers
    //
    VkBufferCreateInfo buffer_create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(vertices),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
    VkBuffer vertex_buffer;
    assert(!vkCreateBuffer(device, &buffer_create_info, NULL, &vertex_buffer));

    // Get memory requirements
    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device, vertex_buffer, &memory_requirements);

    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(gpu, &memory_properties);
    VkMemoryAllocateInfo memory_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = memory_type_find(
            &memory_properties, memory_requirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)};

    // Allocate
    VkDeviceMemory vertex_buffer_memory;
    assert(!vkAllocateMemory(device, &memory_allocate_info, NULL,
                             &vertex_buffer_memory));
    /* printf("Allocated memory for the vertex buffer: size=%lld\n", */
    /*        memory_requirements.size); */
    vkBindBufferMemory(device, vertex_buffer, vertex_buffer_memory, 0);

    // Fill the memory
    void* data;
    vkMapMemory(device, vertex_buffer_memory, 0, buffer_create_info.size, 0,
                &data);
    assert(data != NULL);
    memcpy(data, vertices, buffer_create_info.size);
    vkUnmapMemory(device, vertex_buffer_memory);

    //
    // Command buffers
    //
    const VkCommandBufferAllocateInfo allocate_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = swapchain_image_count,
    };

    VkCommandBuffer command_buffers[swapchain_image_count];
    assert(!vkAllocateCommandBuffers(device, &allocate_info, command_buffers));

    const VkCommandBufferBeginInfo command_buffer_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    const VkClearValue clear_color = {
        .color.float32 = {0.15f, 0.15f, 0.15f, 1.0f}};

    for (usize i = 0; i < swapchain_image_count; i++) {
        const VkRenderPassBeginInfo render_pass_begin_info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = render_pass,
            .framebuffer = frame_buffers[i],
            .renderArea.extent = swapchain_extent,
            .clearValueCount = 1,
            .pClearValues = &clear_color,
        };

        assert(!vkBeginCommandBuffer(command_buffers[i],
                                     &command_buffer_begin_info));

        vkCmdBeginRenderPass(command_buffers[i], &render_pass_begin_info,
                             VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                          graphics_pipeline);

        const VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffers[i], 0, 1, &vertex_buffer,
                               offsets);

        const VkViewport viewport = {
            .width = swapchain_extent.width,
            .height = swapchain_extent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        printf("Viewport: w=%u h=%u\n", (u32)viewport.width,
               (u32)viewport.height);

        const VkRect2D scissor = {.extent = swapchain_extent};
        vkCmdSetViewport(command_buffers[i], 0, 1, &viewport);
        vkCmdSetScissor(command_buffers[i], 0, 1, &scissor);
        vkCmdDraw(command_buffers[i], /* FIXME */ 3, 1, 0, 0);
        vkCmdEndRenderPass(command_buffers[i]);

        assert(!vkEndCommandBuffer(command_buffers[i]));
    }

    //
    // Main loop
    //
    const VkPipelineStageFlags wait_stages =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    usize current_frame = 0;
    for (;;) {
        // Inputs
        {
            SDL_Event event;
            SDL_bool done = SDL_FALSE;

            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:
                        done = SDL_TRUE;
                        break;
                    case SDL_KEYDOWN:

                        if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                            done = SDL_TRUE;
                        break;
                }
            }
            if (done) break;
        }

        //
        // Draw
        //
        vkWaitForFences(device, 1, &in_flight_fences[current_frame], VK_TRUE,
                        UINT64_MAX);

        u32 current_image;

        vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
                              image_available_semaphore[current_frame],
                              VK_NULL_HANDLE, &current_image);

        if (images_in_flight_fences[current_image] != VK_NULL_HANDLE) {
            vkWaitForFences(device, 1, &images_in_flight_fences[current_frame],
                            VK_TRUE, UINT64_MAX);
        }
        images_in_flight_fences[current_image] =
            in_flight_fences[current_frame];

        const VkSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &image_available_semaphore[current_frame],
            .pWaitDstStageMask = &wait_stages,
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffers[current_image],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &render_finished_semaphore[current_frame],
        };

        vkResetFences(device, 1, &in_flight_fences[current_frame]);
        assert(!vkQueueSubmit(queue, 1, &submit_info,
                              in_flight_fences[current_frame]));

        const VkPresentInfoKHR present_info = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &render_finished_semaphore[current_frame],
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &current_image,
        };

        vkQueuePresentKHR(queue, &present_info);

        current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
}

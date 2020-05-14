#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <assert.h>
#include <stdio.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "../utils.h"

#define MAX_EXTENSIONS 64

int main() {
    //
    // SDL init
    //
    SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "1");
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
    VkResult err;
    VkInstance instance;
    {
        u32 extension_count = 0;
        const char* extension_names[MAX_EXTENSIONS] = {0};
        extension_names[extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;

        const VkApplicationInfo app_info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "hello triangle",
            .apiVersion = VK_MAKE_VERSION(0, 0, 1)};

        u32 detected_extension_count = 64 - extension_count;
        if (!SDL_Vulkan_GetInstanceExtensions(
                window, &detected_extension_count,
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

        err = vkCreateInstance(&instance_create_info, NULL, &instance);
        if (err) {
            fprintf(stderr, "vkCreateInstance failed: %d\n", err);
            exit(1);
        }
    }

    //
    // Create Vulkan surface
    //
    VkSurfaceKHR surface;
    {
        if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
            fprintf(stderr, "SDL_Vulkan_CreateSurface failed: %s\n",
                    SDL_GetError());
            exit(1);
        }
    }

    //
    // Create Vulkan physical device
    //
    VkPhysicalDevice gpu;
    {
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
                fprintf(stderr, "vkEnumeratePhysicalDevices (2) failed: %d\n",
                        err);
                exit(1);
            }
            gpu = gpus[0];
        }
        printf("GPUs detected: %u\n", gpu_count);
    }

    //
    // Get information about the device, optional
    //
    {
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(gpu, &device_properties);

        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceFeatures(gpu, &device_features);

        printf("GPU information: name=%s driver_version=%u\n",
               device_properties.deviceName, device_properties.driverVersion);
    }

    //
    // Create command pool & queue
    //
    VkDevice device;
    VkQueue queue;
    VkCommandPool command_pool;

    {
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
            extension_names[extension_count++] =
                VK_KHR_SWAPCHAIN_EXTENSION_NAME;

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
    }

    //
    // Get color format
    //
    VkFormat format;
    VkColorSpaceKHR color_space;
    {
        u32 format_count;

        err = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count,
                                                   NULL);
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

        color_space = formats[0].colorSpace;
    }
    printf("Format: %d\n", format);
    printf("Color space: %d\n", color_space);

    //
    // Command buffer
    //
    VkCommandBuffer command_buffer;
    {
        const VkCommandBufferAllocateInfo allocate_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1};

        err = vkAllocateCommandBuffers(device, &allocate_info, &command_buffer);
        if (err) {
            fprintf(stderr, "vkAllocateCommandBuffers failed: %d\n", err);
            exit(1);
        }
    }

    //
    // Shaders
    //
    VkPipelineShaderStageCreateInfo shader_stages[2];
    {
        const usize buffer_capacity = 10 * 1000 * 1000;
        u8* buffer = ogl_malloc(buffer_capacity);
        usize buffer_len;

        if (file_read("resources/triangle_vert.spv", buffer, buffer_capacity,
                      &buffer_len) != 0) {
            exit(errno);
        }

        VkShaderModuleCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = buffer_len,
            .pCode = (u32*)buffer,
        };

        VkShaderModule vert_shader_module;
        err = vkCreateShaderModule(device, &create_info, NULL,
                                   &vert_shader_module);
        assert(!err);

        printf("Created shader module for `resources/triangle_vert.spv`\n");

        if (file_read("resources/triangle_frag.spv", buffer, buffer_capacity,
                      &buffer_len) != 0) {
            exit(errno);
        }

        create_info = (VkShaderModuleCreateInfo){
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = buffer_len,
            .pCode = (u32*)buffer,
        };

        VkShaderModule frag_shader_module;
        err = vkCreateShaderModule(device, &create_info, NULL,
                                   &frag_shader_module);
        assert(!err);
        printf("Created shader module for `resources/triangle_frag.spv`\n");

        VkPipelineShaderStageCreateInfo vert_shader_stage_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vert_shader_module,
            .pName = "main"  // Entrypoint function
        };

        VkPipelineShaderStageCreateInfo frag_shader_stage_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = frag_shader_module,
            .pName = "main"  // Entrypoint function
        };

        shader_stages[0] = vert_shader_stage_info;
        shader_stages[1] = frag_shader_stage_info;
    }

    //
    // Swap chain
    //
    VkSwapchainKHR swapchain;
    u32 swapchain_image_count;
    VkExtent2D swapchain_extent;
    {
        VkSurfaceCapabilitiesKHR surface_capabilities;
        err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface,
                                                        &surface_capabilities);

        if (err) {
            fprintf(stderr,
                    "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed: %d\n",
                    err);
            exit(1);
        }

        // Get drawing surface dimensions
        if (surface_capabilities.currentExtent.width == (UINT32_MAX)) {
            i32 w, h;
            SDL_GetWindowSize(window, &w, &h);
            swapchain_extent.width = w;
            swapchain_extent.height = h;
        } else {
            swapchain_extent = surface_capabilities.currentExtent;
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

        err = vkCreateSwapchainKHR(device, &swapchain_create_info, NULL,
                                   &swapchain);

        if (err) {
            fprintf(stderr, "vkCreateSwapchainKHR failed: %d\n", err);
            exit(1);
        }
    }
    printf("Swapchain image count: %u\n", swapchain_image_count);

    //
    // Fixed functions
    //
    VkPipelineLayout pipeline_layout;
    VkPipelineViewportStateCreateInfo viewport_state;
    VkPipelineRasterizationStateCreateInfo rasterizer;
    VkPipelineMultisampleStateCreateInfo multisampling;
    VkPipelineInputAssemblyStateCreateInfo input_assembly;
    VkPipelineVertexInputStateCreateInfo vertex_input_info;
    VkPipelineColorBlendStateCreateInfo color_blending;
    {
        vertex_input_info = (VkPipelineVertexInputStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        };

        input_assembly = (VkPipelineInputAssemblyStateCreateInfo){
            .sType =
                VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        };

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = swapchain_extent.width,
            .height = swapchain_extent.height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };

        VkRect2D scissor = {.extent = swapchain_extent};

        viewport_state = (VkPipelineViewportStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor,
        };

        rasterizer = (VkPipelineRasterizationStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .lineWidth = 1.0f,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
        };

        multisampling = (VkPipelineMultisampleStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .minSampleShading = 1.0f,
        };

        VkPipelineColorBlendAttachmentState color_blend_attachment = {
            .colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_B_BIT,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
        };

        color_blending = (VkPipelineColorBlendStateCreateInfo){
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments = &color_blend_attachment,
        };

        VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                           VK_DYNAMIC_STATE_LINE_WIDTH};
        VkPipelineDynamicStateCreateInfo dynamic_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = 2,
            .pDynamicStates = dynamic_states};

        VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        };

        err = vkCreatePipelineLayout(device, &pipeline_layout_create_info, NULL,
                                     &pipeline_layout);
    }

    struct {
        VkImage image;
        VkCommandBuffer command;
        VkImageView view;
        VkFramebuffer frame_buffer;
    } buffers[swapchain_image_count];

    {
        err = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count,
                                      0);
        if (err) {
            fprintf(stderr, "vkGetSwapchainImagesKHR failed: %d\n", err);
            exit(1);
        }

        VkImage swapchain_images[swapchain_image_count];
        err = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count,
                                      swapchain_images);

        if (err) {
            fprintf(stderr, "vkGetSwapchainImagesKHR (2) failed: %d\n", err);
            exit(1);
        }

        for (u32 i = 0; i < swapchain_image_count; i++)
            buffers[i].image = swapchain_images[i];
    }

    for (u32 i = 0; i < swapchain_image_count; i++) {
        VkImageViewCreateInfo color_attachment_view = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .format = format,
            .components =
                {
                    .r = VK_COMPONENT_SWIZZLE_R,
                    .g = VK_COMPONENT_SWIZZLE_G,
                    .b = VK_COMPONENT_SWIZZLE_B,
                    .a = VK_COMPONENT_SWIZZLE_A,
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
            .image = buffers[i].image,
        };

        err = vkCreateImageView(device, &color_attachment_view, NULL,
                                &buffers[i].view);

        if (err) {
            fprintf(stderr, "vkCreateImageView (%u) failed: %d\n", i, err);
            exit(1);
        }
        printf("Initialized image view #%u\n", i);
    }

    //
    // Attachments
    //
    const VkAttachmentDescription attachments[1] = {
        [0] =
            {
                .format = format,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            },
    };

    const VkAttachmentReference color_reference = {
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    const VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_reference,
    };

    const VkRenderPassCreateInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };

    VkRenderPass render_pass;
    err = vkCreateRenderPass(device, &render_pass_info, NULL, &render_pass);

    if (err) {
        fprintf(stderr, "vkCreateRenderPass failed: %d\n", err);
        exit(1);
    }

    printf("Created render pass\n");

    //
    // Graphics pipeline
    //
    VkPipeline graphics_pipeline;
    {
        VkGraphicsPipelineCreateInfo pipeline_info = {
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
        };

        err =
            vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info,
                                      NULL, &graphics_pipeline);
        assert(!err);
    }
    printf("Created graphics pipeline\n");

    //
    // Frame buffers
    //
    for (u32 i = 0; i < swapchain_image_count; i++) {
        VkImageView attachments[1] = {
            [0] = buffers[i].view,
        };

        const VkFramebufferCreateInfo frame_buffer_create_info = {

            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO,
            .renderPass = render_pass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = swapchain_extent.width,
            .height = swapchain_extent.height,
            .layers = 1,
        };

        err = vkCreateFramebuffer(device, &frame_buffer_create_info, NULL,
                                  &buffers[i].frame_buffer);
        if (err) {
            fprintf(stderr, "vkCreateFramebuffer failed: %d\n", err);
            exit(1);
        }

        printf("Created frame buffer #%u\n", i);
    }

    //
    // Main loop
    //
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

        VkSemaphore present_complete_semaphore;
        {
            VkSemaphoreCreateInfo semaphore_create_info = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };

            err = vkCreateSemaphore(device, &semaphore_create_info, NULL,
                                    &present_complete_semaphore);
            if (err) {
                fprintf(stderr, "vkCreateSemaphore failed: %d\n", err);
                exit(1);
            }
        }

        u32 current_buffer;
        err = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX,
                                    present_complete_semaphore, (VkFence)0,
                                    &current_buffer);

        assert(!err);

        const VkCommandBufferBeginInfo command_buffer_begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        const VkClearValue clear_values[1] = {
            [0] = {.color.float32 = {current_buffer * 1.f, .2f, .2f, .2f}},
        };

        const VkRenderPassBeginInfo render_pass_begin_info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = render_pass,
            .framebuffer = buffers[current_buffer].frame_buffer,
            .renderArea.extent = swapchain_extent,
            .clearValueCount = 1,
            .pClearValues = clear_values,
        };

        err = vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);
        assert(!err);

        VkImageMemoryBarrier image_memory_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
            .image = buffers[current_buffer].image,
        };

        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL,
                             0, NULL, 1, &image_memory_barrier);

        vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info,
                             VK_SUBPASS_CONTENTS_INLINE);
        vkCmdEndRenderPass(command_buffer);

        VkImageMemoryBarrier present_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
            .image = buffers[current_buffer].image,
        };

        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL,
                             0, NULL, 1, &present_barrier);

        err = vkEndCommandBuffer(command_buffer);
        assert(!err);

        VkPipelineStageFlags pipe_stage_flags =
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

        VkSubmitInfo submit_info = {

            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffer,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &present_complete_semaphore,
            .pWaitDstStageMask = &pipe_stage_flags,
        };

        err = vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        assert(!err);

        VkPresentInfoKHR present = {.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                    .swapchainCount = 1,
                                    .pSwapchains = &swapchain,
                                    .pImageIndices = &current_buffer};

        err = vkQueuePresentKHR(queue, &present);

        if (err == VK_SUBOPTIMAL_KHR) {
            fprintf(stderr, "Warning: suboptimal present\n");
        } else
            assert(!err);

        err = vkQueueWaitIdle(queue);
        assert(err == VK_SUCCESS);

        vkDestroySemaphore(device, present_complete_semaphore, NULL);
    }
}

#include "vk_engine.h"

#include <iostream>

#define VK_CHECK(x)                                                     \
    do                                                                  \
    {                                                                   \
        VkResult err = x;                                               \
        if (err)                                                        \
        {                                                               \
            std::cout << "Detected Vulkan error: " << err << std::endl; \
            abort();                                                    \
        }                                                               \
    } while (0)

namespace vk
{
    void VulkanEngine::init()
    {
        // We initialize SDL and create a window with it.
        SDL_Init(SDL_INIT_VIDEO);

        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

        _window = SDL_CreateWindow(
            "Vulkan Engine",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            _windowExtent.width,
            _windowExtent.height,
            window_flags);

        // load the core Vulkan structures
        init_vulkan();

        // everything went fine
        _isInitialized = true;
    }

    void VulkanEngine::init_vulkan()
    {
        // nothing yet
    }

}
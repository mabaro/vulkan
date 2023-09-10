#include "SDLWindowVulkan.h"

#include "gfx/vk_types.h"
#include <SDL2/SDL_vulkan.h>

#include <vulkan/vulkan_core.h>

#include <iostream>

bool
SDLWindowVulkan::Init()
{
    bool result = true;
    result |= SDLWindow::Init();

    result &= _InitSurface();

    return result;
}

bool
SDLWindowVulkan::_InitSurface()
{
    unsigned int requiredExtensionsCount = 0;
    SDL_Vulkan_GetInstanceExtensions(_window, &requiredExtensionsCount, nullptr);
    std::vector<const char*> requiredExtensionNames(requiredExtensionsCount);
    SDL_Vulkan_GetInstanceExtensions(_window, &requiredExtensionsCount, requiredExtensionNames.data());
    std::cout << "Required extensions(" << requiredExtensionsCount << "):" << std::endl;
    for (const char* name : requiredExtensionNames) {
        std::cout << name << std::endl;
    }

    // requiredExtensionNames.push_back("myExtension");

    {   // retrieve available extensions
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availabelExtensionNames(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availabelExtensionNames.data());

        std::cout << "available extensions(" << availabelExtensionNames.size() << "):\n";

        for (const auto& extension : availabelExtensionNames) {
            std::cout << '\t' << extension.extensionName << '\n';
        }
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = GetName();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    std::vector<const char*> validationLayers {};

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.enabledLayerCount = validationLayers.size();
    instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    instanceCreateInfo.enabledExtensionCount = requiredExtensionNames.size();
    instanceCreateInfo.ppEnabledExtensionNames = requiredExtensionNames.data();

    VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));
    SDL_Vulkan_CreateSurface(_window, instance, &surface);

    return true;
}
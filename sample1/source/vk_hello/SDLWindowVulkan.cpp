#include "SDLWindowVulkan.h"

#include "gfx/vk_types.h"
#include <SDL2/SDL_vulkan.h>

#include <vulkan/vulkan_core.h>

#include <iostream>

#define VALIDATION_LAYERS USING(!IS_DEBUG)

SDLWindowVulkan::SDLWindowVulkan()
    : SDLWindow()
    , _validationLayers({ "VK_LAYER_KHRONOS_validation" })
{
}

bool
SDLWindowVulkan::Init()
{
    bool result = true;
    result |= SDLWindow::Init();

    result &= _InitSurface();

    return result;
}

void
SDLWindowVulkan::Close()
{
    vkDestroyInstance(_instance, nullptr);
    _instance = nullptr;

    SDLWindow::Close();
}

bool
SDLWindowVulkan::_InitSurface()
{
    bool validationLayersEnabled = !_validationLayers.empty();
    if (validationLayersEnabled && !_AreValidationLayersSupported(_validationLayers)) {
        LOG_ERROR("Validation layers enabled but not supported.");
        validationLayersEnabled = false;
    }

    unsigned int requiredExtensionsCount = 0;
    SDL_Vulkan_GetInstanceExtensions(_window, &requiredExtensionsCount, nullptr);
    std::vector<const char*> requiredExtensionNames(requiredExtensionsCount);
    SDL_Vulkan_GetInstanceExtensions(_window, &requiredExtensionsCount, requiredExtensionNames.data());
    std::cout << "Required extensions(" << requiredExtensionsCount << "):" << std::endl;
    for (const char* name : requiredExtensionNames) {
        std::cout << name << std::endl;
    }

    {   // retrieve available extensions
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availabelExtensionNames(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availabelExtensionNames.data());

        std::cout << "Available extensions(" << availabelExtensionNames.size() << "):\n";
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

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;
    if (validationLayersEnabled) {
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = _validationLayers.data();
    } else {
        instanceCreateInfo.enabledLayerCount = 0;
    }
    instanceCreateInfo.enabledExtensionCount = requiredExtensionNames.size();
    instanceCreateInfo.ppEnabledExtensionNames = requiredExtensionNames.data();

    VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &_instance));
    SDL_Vulkan_CreateSurface(_window, _instance, &_surface);

    return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool
SDLWindowVulkan::_AreValidationLayersSupported(const std::vector<const char*>& validationLayers)
{
    assert(!validationLayers.empty());

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

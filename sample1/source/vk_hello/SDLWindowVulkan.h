#pragma once

#include "SDLWindow.h"

#include "gfx/vk_types.h"

#include <cassert>
#include <vector>

#define VALIDATION_LAYERS USING(!IS_DEBUG)

class SDLWindowVulkan : public SDLWindow {
    VkInstance _instance = VK_NULL_HANDLE;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice _device = VK_NULL_HANDLE;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;

    VkQueue _graphicsQueue;
    VkQueue _presentQueue;

    VkDebugUtilsMessengerEXT _debugMessenger;

#if USING(VALIDATION_LAYERS)
    std::vector<const char*> _validationLayers;
    bool _validationLayersEnabled = true;
#endif   // #if USING(VALIDATION_LAYERS)

public:
    SDLWindowVulkan();

    bool Init() override;
    void Close() override;

protected:
    bool _CreateInstance();
    bool _SelectAdapter();
    bool _CreateLogicalDevice();
    bool _CreateSurface();

    bool _IsPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) const;

#if USING(VALIDATION_LAYERS)
    inline bool _AreValidationLayersEnabled() const { return _validationLayersEnabled; }
    bool _AreInstanceLayersSupported(const std::vector<const char*>& validationLayers);
    void _PopulateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    bool _InitDebugMessenger();
    bool _DeinitDebugMessenger();
#endif   // #if USING(VALIDATION_LAYERS)
};

////////////////////////////////////////////////////////////////////////////////

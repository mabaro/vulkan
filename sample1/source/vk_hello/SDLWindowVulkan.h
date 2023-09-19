#pragma once

#include "SDLWindow.h"

#include "gfx/vk_types.h"

#include <cassert>
#include <vector>

#define VALIDATION_LAYERS USING(!IS_DEBUG)

class SDLWindowVulkan : public SDLWindow {
    VkInstance _instance;
    VkDevice _device;
    VkPhysicalDevice _physicalDevice;
    VkSurfaceKHR _surface;

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
    bool _InitSurface();
    bool _SelectAdapter();

    bool _IsPhysicalDeviceSuitable(VkPhysicalDevice device) const;

#if USING(VALIDATION_LAYERS)
        inline bool
        _AreValidationLayersEnabled() const { return _validationLayersEnabled; }
    bool _AreInstanceLayersSupported(const std::vector<const char*>& validationLayers);
    void _PopulateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    bool _InitDebugMessenger();
    bool _DeinitDebugMessenger();
#endif   // #if USING(VALIDATION_LAYERS)
};

////////////////////////////////////////////////////////////////////////////////

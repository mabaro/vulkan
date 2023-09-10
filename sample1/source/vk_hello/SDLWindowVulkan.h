#pragma once

#include "SDLWindow.h"

#include "gfx/vk_types.h"

#include <cassert>
#include <vector>

#define VALIDATION_LAYERS USING(!IS_DEBUG)

class SDLWindowVulkan : public SDLWindow {
    VkInstance _instance;
    VkSurfaceKHR _surface;
    VkDevice _device;
    VkPhysicalDevice _physicalDevice;

#if USING(VALIDATION_LAYERS)
    std::vector<const char*> _validationLayers;
    VkDebugUtilsMessengerEXT _debugMessenger;
#endif // #if USING(VALIDATION_LAYERS)

public:
    SDLWindowVulkan();

    bool Init() override;
    void Close() override;

protected:
    bool _InitSurface();

#if USING(VALIDATION_LAYERS)
    bool _AreValidationLayersSupported(const std::vector<const char*>& validationLayers);
    bool _InitDebugMessenger();
    bool _DeinitDebugMessenger();
#endif   // #if USING(VALIDATION_LAYERS)
};

////////////////////////////////////////////////////////////////////////////////

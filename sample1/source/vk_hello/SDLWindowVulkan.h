#pragma once

#include "SDLWindow.h"

#include <vulkan/vulkan.h>

#include <cassert>
#include <vector>

class SDLWindowVulkan : public SDLWindow {
    VkInstance _instance;
    VkSurfaceKHR _surface;
    VkDevice _device;
    VkPhysicalDevice _physicalDevice;
    std::vector<const char*> _validationLayers;

public:
    SDLWindowVulkan();

    bool Init() override;
    void Close() override;

protected:
    bool _InitSurface();
    bool _AreValidationLayersSupported(const std::vector<const char*>& validationLayers);
};

////////////////////////////////////////////////////////////////////////////////

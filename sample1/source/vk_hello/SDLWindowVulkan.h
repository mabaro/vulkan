#pragma once

#include "SDLWindow.h"
#include <cassert>
#include <vector>
#include <vulkan/vulkan.h>

class SDLWindowVulkan : public SDLWindow {
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;

public:
    SDLWindowVulkan()
        : SDLWindow()
    {
    }

    bool Init() override;

protected:
    bool _InitSurface();

};

////////////////////////////////////////////////////////////////////////////////

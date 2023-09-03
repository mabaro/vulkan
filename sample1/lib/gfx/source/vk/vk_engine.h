#pragma once

#include "vk_init.h"
#include "vk_types.h"

#include "VkBootstrap.h"

namespace vk {
class VulkanEngine {
  public:
    VkInstance _instance;                        // Vulkan library handle
    VkDebugUtilsMessengerEXT _debug_messenger;   // Vulkan debug output handle
    VkPhysicalDevice _chosenGPU;   // GPU chosen as the default device
    VkDevice _device;              // Vulkan device for commands
    VkSurfaceKHR _surface;         // Vulkan window surface

  private:
    void init_vulkan();
};
}   // namespace vk
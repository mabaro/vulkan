#pragma once

#include "SDLWindow.h"

#include "gfx/vk_types.h"

#include <vector>

#define VALIDATION_LAYERS USING(IS_DEBUG)

namespace gfx {
////////////////////////////////////////////////////////////////////////////////

class SDLWindowVulkan : public SDLWindow {
    VkInstance       _instance       = VK_NULL_HANDLE;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkDevice         _device         = VK_NULL_HANDLE;
    VkSurfaceKHR     _surface        = VK_NULL_HANDLE;

    VkQueue _graphicsQueue = VK_NULL_HANDLE;
    VkQueue _presentQueue  = VK_NULL_HANDLE;

    VkDebugUtilsMessengerEXT _debugMessenger;

    std::vector<const char*> _deviceExtensions;

#if USING(VALIDATION_LAYERS)
    std::vector<const char*> _validationLayers;
    bool                     _validationLayersEnabled = true;
#endif   // #if USING(VALIDATION_LAYERS)

    // swapchain
    VkSwapchainKHR             _swapChain = VK_NULL_HANDLE;
    VkFormat                   _swapChainImageFormat;
    VkExtent2D                 _swapChainExtent;
    std::vector<VkImage>       _swapChainImages;
    std::vector<VkImageView>   _swapChainImageViews;
    std::vector<VkFramebuffer> _swapChainFramebuffers;

    VkRenderPass     _renderPass;
    VkPipelineLayout _pipelineLayout;
    VkPipeline       _graphicsPipeline;

    VkCommandPool _commandPool;
    std::vector<VkCommandBuffer> _commandBuffers;

    VkSemaphore _imageAvailableSemaphore;
    VkSemaphore _renderFinishedSemaphore;
    VkFence     _inFlightFence;

public:
    SDLWindowVulkan();

    bool Init() override;
    void Close() override;

    void DrawFrame() override;

protected:
    bool _CreateInstance();
    bool _SelectAdapter();
    bool _CreateLogicalDevice();
    bool _CreateSurface();
    bool _CreateSwapchain();
    bool _CreateImageViews();
    bool _CreateRenderPass();
    bool _CreateGraphicsPipeline();
    bool _CreateFramebuffers();
    bool _CreateCommandPool();
    bool _CreateCommandBuffer();
    bool _RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    bool _CreateSyncObjects();

    bool _IsPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) const;

#if USING(VALIDATION_LAYERS)
    inline bool _AreValidationLayersEnabled() const { return _validationLayersEnabled; }
    bool        _AreInstanceLayersSupported(const std::vector<const char*>& validationLayers);
    void        _PopulateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    bool        _InitDebugMessenger();
    bool        _DeinitDebugMessenger();
#endif   // #if USING(VALIDATION_LAYERS)
    };

////////////////////////////////////////////////////////////////////////////////
}   // namespace gfx {

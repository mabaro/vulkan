#pragma once

#include "SDLWindow.h"

#include "gfx/vk_types.h"

#include <functional>
#include <vector>

#define VALIDATION_LAYERS USING(IS_DEBUG)

namespace gfx {
////////////////////////////////////////////////////////////////////////////////

class SDLWindowVulkan : public SDLWindow {
    
    static constexpr int MAX_FRAMES_IN_FLIGHT = 5;

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

    VkCommandPool                _commandPool;

    std::vector<VkCommandBuffer> _commandBuffers;
    std::vector<VkSemaphore>  _imageAvailableSemaphores;
    std::vector<VkSemaphore>  _renderFinishedSemaphores;
    std::vector<VkFence>      _inFlightFences;
    uint8_t                   _currentFrameIndex = 0;

private:
    struct UploadContext {
        VkFence         _uploadFence;
        VkCommandPool   _commandPool;
        VkCommandBuffer _commandBuffer;
    };
    UploadContext _uploadContext;

    std::vector<std::function<void(void)>> _mainDeletionQueue;

public:
    SDLWindowVulkan();

    bool Init() override;
    void Close() override;

protected:
    void _DrawFrame() override;
    void _OnMainLoopExit() override;
    void _OnResize(uint32_t width, uint32_t height) override;

protected:
    void _InitImgui();

    void _ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

    enum class DeletionQueue {
        Main,
    };
    void _EnqueueForDeletion(DeletionQueue queue, std::function<void()> func);

protected:
    bool _CreateInstance();
    bool _SelectAdapter();
    bool _CreateLogicalDevice();
    bool _CreateSurface();
    bool _CreateSwapChain();
    bool _CreateImageViews();
    bool _CreateRenderPass();
    bool _CreateGraphicsPipeline();
    bool _CreateFramebuffers();
    bool _CreateCommandPool();
    bool _CreateCommandBuffer();
    bool _RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    bool _CreateSyncObjects();

    void _CleanupSwapChain();
    void _RecreateSwapChain();

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

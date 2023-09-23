#include "SDLWindowVulkan.h"

#include "Shader.h"
#include "gfx/vk_types.h"

#include <SDL2/SDL_vulkan.h>

#include <vulkan/vulkan_core.h>

#include <algorithm>   // Necessary for std::clamp
#include <limits>      // Necessary for std::numeric_limits
#include <optional>
#include <set>

#define LIST_AVAILABLE_EXTENSIONS NOT_IN_USE

namespace gfx {
/////////////////////////////////////////////////////////////////////////////////

SDLWindowVulkan::SDLWindowVulkan()
    : SDLWindow()
    , _deviceExtensions {VK_KHR_SWAPCHAIN_EXTENSION_NAME}
#if USING(VALIDATION_LAYERS)
    , _validationLayers({
          "VK_LAYER_KHRONOS_validation",
      })
#endif   // #if USING(VALIDATION_LAYERS)
{
}

bool
SDLWindowVulkan::Init()
{
    bool result = true;
    result &= SDLWindow::Init();
    ASSERT(result);

    result &= _CreateInstance();
    ASSERT(result);
    result &= _CreateSurface();
    ASSERT(result);
    result &= _SelectAdapter();
    ASSERT(result);
    result &= _CreateLogicalDevice();
    ASSERT(result);
    result &= _CreateSwapChain();
    ASSERT(result);
    result &= _CreateImageViews();
    ASSERT(result);
    result &= _CreateRenderPass();
    ASSERT(result);
    result &= _CreateFramebuffers();
    ASSERT(result);
    result &= _CreateGraphicsPipeline();
    ASSERT(result);
    result &= _CreateCommandPool();
    ASSERT(result);
    result &= _CreateCommandBuffer();
    ASSERT(result);
    result &= _CreateSyncObjects();
    ASSERT(result);

#if USING(VALIDATION_LAYERS)
    result &= _InitDebugMessenger();
    ASSERT(result);
#endif   // #if USING(VALIDATION_LAYERS)

    return result;
}

void
SDLWindowVulkan::Close()
{
    _CleanupSwapChain();

    VK_DESTROY_WITH_DEVICE(vkDestroyPipeline,_device, _graphicsPipeline, nullptr);
    VK_DESTROY_WITH_DEVICE(vkDestroyPipelineLayout,_device, _pipelineLayout, nullptr);
    VK_DESTROY_WITH_DEVICE(vkDestroyRenderPass,_device, _renderPass, nullptr);

    {   // _DestroySyncObjects()
        VK_DESTROY_LIST_WITH_DEVICE(vkDestroySemaphore, _device, _imageAvailableSemaphores, nullptr);
        VK_DESTROY_LIST_WITH_DEVICE(vkDestroySemaphore, _device, _renderFinishedSemaphores, nullptr);
        VK_DESTROY_LIST_WITH_DEVICE(vkDestroyFence, _device, _inFlightFences, nullptr);
    }

    VK_DESTROY_WITH_DEVICE(vkDestroyCommandPool,_device, _commandPool, nullptr);
    VK_DESTROY(vkDestroyDevice, _device, nullptr);

#if USING(VALIDATION_LAYERS)
    _DeinitDebugMessenger();
#endif   // #if USING(VALIDATION_LAYERS)

    VK_DESTROY(vkDestroySurfaceKHR, _instance, _surface, nullptr);
    VK_DESTROY(vkDestroyInstance, _instance, nullptr);

    SDLWindow::Close();
}

////////////////////////////////////////////////////////////////////////////////

void
SDLWindowVulkan::_DrawFrame()
{
    SDLWindow::_DrawFrame();

    const uint32_t currentFrame = _currentFrameIndex++ % MAX_FRAMES_IN_FLIGHT;
    LOG_DEBUG("Draw: %d", _currentFrameIndex);

    const uint64_t fenceTimeout = UINT64_MAX;
    vkWaitForFences(_device, 1, &_inFlightFences[currentFrame], VK_TRUE, fenceTimeout);

    uint32_t swapchainIndex;
    {
        VkResult result = vkAcquireNextImageKHR(
            _device, _swapChain, UINT64_MAX, _imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &swapchainIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            _RecreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            LOG_ERROR("failed to acquire swap chain image!");
        }
    }

    // only reset if we are submitting work to avoid deadlock due to no signaling
    vkResetFences(_device, 1, &_inFlightFences[currentFrame]);

    VkCommandBuffer commandBuffer = _commandBuffers[currentFrame];
    // VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT
    VkCommandBufferResetFlags commandBufferFlags {};
    vkResetCommandBuffer(commandBuffer, commandBufferFlags);
    _RecordCommandBuffer(commandBuffer, swapchainIndex);

    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore          waitSemaphores[] = {_imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[]     = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount         = 1;
    submitInfo.pWaitSemaphores            = waitSemaphores;
    submitInfo.pWaitDstStageMask          = waitStages;
    submitInfo.commandBufferCount         = 1;
    submitInfo.pCommandBuffers            = &_commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[]  = {_renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphores;

    VK_CHECK_MSG(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFences[currentFrame]),
        "failed to submit draw command buffer!");

    {
        VkPresentInfoKHR presentInfo {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = signalSemaphores;

        VkSwapchainKHR swapChains[] = {_swapChain};
        presentInfo.swapchainCount  = 1;
        presentInfo.pSwapchains     = swapChains;
        presentInfo.pImageIndices   = &swapchainIndex;

        presentInfo.pResults = nullptr;   // Optional

        {
            VkResult result = vkQueuePresentKHR(_presentQueue, &presentInfo);
            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                _RecreateSwapChain();
                return;
            } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
                LOG_ERROR("failed to acquire swap chain image!");
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void
SDLWindowVulkan::_OnMainLoopExit()
{
    // need to wait for any async processes
    vkDeviceWaitIdle(_device);
}

////////////////////////////////////////////////////////////////////////////////

void
SDLWindowVulkan::_OnResize(uint32_t width, uint32_t height)
{
    SDLWindow::_OnResize(width, height);

    _RecreateSwapChain();
}

////////////////////////////////////////////////////////////////////////////////

bool
SDLWindowVulkan::_CreateInstance()
{
#if USING(VALIDATION_LAYERS)
    if (_AreValidationLayersEnabled() && !_AreInstanceLayersSupported(_validationLayers)) {
        LOG_WARN("Validation layers enabled but not supported.");
        _validationLayersEnabled = false;
    }
#endif   // #if USING(VALIDATION_LAYERS)

    std::vector<const char*> requiredExtensionNames;
    {
        unsigned int requiredExtensionsCount = 0;
        bool         success = SDL_Vulkan_GetInstanceExtensions(_window, &requiredExtensionsCount, nullptr);
        ASSERT(success);
        requiredExtensionNames.resize(requiredExtensionsCount);
        success = SDL_Vulkan_GetInstanceExtensions(_window, &requiredExtensionsCount, requiredExtensionNames.data());
        ASSERT(success);
    }

    requiredExtensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    std::cout << "Required extensions(" << requiredExtensionNames.size() << "):" << std::endl;
    for (const char* name : requiredExtensionNames) {
        std::cout << name << std::endl;
    }

    if (USING(LIST_AVAILABLE_EXTENSIONS)) {   // retrieve available extensions
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensionNames(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensionNames.data());

        std::cout << "Available extensions(" << availableExtensionNames.size() << "):\n";
        for (const auto& extension : availableExtensionNames) {
            std::cout << '\t' << extension.extensionName << '\n';
        }
    }

    VkApplicationInfo appInfo  = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = GetName();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "No Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo     = &appInfo;
    //
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.pNext             = nullptr;
#if USING(VALIDATION_LAYERS)
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (_AreValidationLayersEnabled()) {
        instanceCreateInfo.enabledLayerCount += static_cast<uint32_t>(_validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = _validationLayers.data();
        _PopulateDebugMessenger(debugCreateInfo);
        instanceCreateInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
    }
    requiredExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif   // #if USING(VALIDATION_LAYERS)
    //
    instanceCreateInfo.enabledExtensionCount   = requiredExtensionNames.size();
    instanceCreateInfo.ppEnabledExtensionNames = requiredExtensionNames.data();

    VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &_instance));

    return true;
}

////////////////////////////////////////////////////////////////////////////////

struct QueueFamilyIndices {
    std::optional<uint32_t> optGraphicsFamily;
    std::optional<uint32_t> optPresentFamily;

    bool IsComplete() const { return optGraphicsFamily.has_value() && optPresentFamily.has_value(); }
};

QueueFamilyIndices
findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.optGraphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            indices.optPresentFamily = i;
        }
        if (indices.IsComplete()) {
            break;
        }
        i++;
    }

    return indices;
}

bool
checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& i_deviceExtensions)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(i_deviceExtensions.begin(), i_deviceExtensions.end());
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};
SwapChainSupportDetails
querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

bool
SDLWindowVulkan::_IsPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) const
{
    VkPhysicalDeviceFeatures adapterFeatures;
    vkGetPhysicalDeviceFeatures(device, &adapterFeatures);
    VkPhysicalDeviceProperties adapterProperties;
    vkGetPhysicalDeviceProperties(device, &adapterProperties);

    const std::vector<const char*> deviceExtensions    = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const bool                     extensionsSupported = checkDeviceExtensionSupport(device, deviceExtensions);
    ASSERT(extensionsSupported);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        const SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        ASSERT(swapChainAdequate);
    }

    LOG_DEBUG("PhysDev: %s | %s\n", adapterProperties.deviceName, adapterProperties.deviceID);

    // kkASSERT(adapterProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);

    const QueueFamilyIndices queueFamily = findQueueFamilies(device, surface);
    return extensionsSupported
        && swapChainAdequate
        // kk && adapterProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
        //  && adapterFeatures.multiDrawIndirect
        && queueFamily.IsComplete();
}

bool
SDLWindowVulkan::_SelectAdapter()
{
    ASSERT(_surface != nullptr);

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        LOG_ERROR("No physical devices found");
        return false;
    }

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(_instance, &deviceCount, physicalDevices.data());

    for (const VkPhysicalDevice& physicalDevice : physicalDevices) {
        if (_IsPhysicalDeviceSuitable(physicalDevice, _surface)) {
            _physicalDevice = physicalDevice;
            break;
        }
    }
    ASSERT(_physicalDevice != VK_NULL_HANDLE);

    if (_physicalDevice == VK_NULL_HANDLE) {
        LOG_ERROR("Couldn't select physical device");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool
SDLWindowVulkan::_CreateLogicalDevice()
{
    ASSERT(_physicalDevice != nullptr && _surface != nullptr);

    QueueFamilyIndices indices = findQueueFamilies(_physicalDevice, _surface);

    VkDeviceQueueCreateInfo queueCreateInfo {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    ASSERT(indices.optGraphicsFamily.has_value());
    queueCreateInfo.queueFamilyIndex = indices.optGraphicsFamily.value();
    queueCreateInfo.queueCount       = 1;

    float queuePriority              = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures {};

    VkDeviceCreateInfo createInfo {};
    createInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos    = &queueCreateInfo;
    createInfo.pEnabledFeatures     = &deviceFeatures;
    {
        createInfo.enabledExtensionCount   = _deviceExtensions.size();
        createInfo.ppEnabledExtensionNames = _deviceExtensions.data();
        createInfo.enabledLayerCount       = 0;
        createInfo.ppEnabledLayerNames     = nullptr;
#if USING(VALIDATION_LAYERS)
        if (_AreValidationLayersEnabled()) {
            createInfo.enabledLayerCount   = static_cast<uint32_t>(_validationLayers.size());
            createInfo.ppEnabledLayerNames = _validationLayers.data();
        }
#endif   // #if USING(VALIDATION_LAYERS)
    }
    if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS) {
        LOG_ERROR("Failed to create logical device!");
        return false;
    }

    vkGetDeviceQueue(_device, indices.optGraphicsFamily.value(), 0, &_graphicsQueue);
    if (_graphicsQueue == VK_NULL_HANDLE) {
        LOG_ERROR("Couldn't create graphics queue");
        return false;
    }

    {   // create presentation queue
        QueueFamilyIndices indices = findQueueFamilies(_physicalDevice, _surface);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.optGraphicsFamily.value(), indices.optPresentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo {};
            queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount       = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos    = queueCreateInfos.data();
    }
    vkGetDeviceQueue(_device, indices.optPresentFamily.value(), 0, &_presentQueue);
    if (_presentQueue == VK_NULL_HANDLE) {
        LOG_ERROR("Couldn't create present queue");
        return false;
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
SDLWindowVulkan::_CreateSurface()
{
    /* handling on our own we'd need per platform stuff:
    #define VK_USE_PLATFORM_WIN32_KHR
    #define GLFW_INCLUDE_VULKAN
    #include <GLFW/glfw3.h>
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>

    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = glfwGetWin32Window(window);
    createInfo.hinstance = GetModuleHandle(nullptr);
    if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
    */
    if (!SDL_Vulkan_CreateSurface(_window, _instance, &_surface)) {
        LOG_ERROR("Couldn't create a surface!");
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////

VkSurfaceFormatKHR
chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats.front();
}

VkPresentModeKHR
chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    /*
    VK_PRESENT_MODE_MAILBOX_KHR // vsync - discard any previous presentation requests on vsync
    VK_PRESENT_MODE_FIFO_KHR // double/triple buffering - sequentially execute presentation requests on vsync
    */
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
chooseSwapExtent(SDL_Window* window, const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        int display_width, display_height;

        SDL_GetWindowSize(window, &width, &height);
        SDL_GL_GetDrawableSize(window, &display_width, &display_height);
        // const float wScale = width > 0 ? ((float)display_width / width) : 0;
        // const float hScale = height > 0 ? ((float)display_height / height) : 0;

        VkExtent2D actualExtent {.width = std::clamp(static_cast<uint32_t>(display_width),
                                     capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            .height = std::clamp(static_cast<uint32_t>(display_height), capabilities.minImageExtent.height,
                capabilities.maxImageExtent.height)};

        return actualExtent;
    }
}

bool
SDLWindowVulkan::_CreateSwapChain()
{
    const SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_physicalDevice, _surface);

    const VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    const VkPresentModeKHR   presentMode   = chooseSwapPresentMode(swapChainSupport.presentModes);
    const VkExtent2D         extent        = chooseSwapExtent(_window, swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo {};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = _surface;
    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const QueueFamilyIndices indices    = findQueueFamilies(_physicalDevice, _surface);
    const uint32_t queueFamilyIndices[] = {indices.optGraphicsFamily.value(), indices.optPresentFamily.value()};

    if (indices.optGraphicsFamily != indices.optPresentFamily) {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;   // slow!
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;         // Optional
        createInfo.pQueueFamilyIndices   = nullptr;   // Optional
    }

    // i.e., use SAMSUNG extension for swapchain transform
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    // to be checked for support if needed//swapChainSupport.capabilities.supportedTransforms

    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;   // opaque surface
    createInfo.clipped        = VK_TRUE;

    createInfo.presentMode = presentMode;

    createInfo.oldSwapchain = VK_NULL_HANDLE;   // should point to old one when recreating (i.e., resize)

    if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain) != VK_SUCCESS) {
        LOG_ERROR("failed to create swap chain!");
        return false;
    }

    vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, nullptr);
    _swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, _swapChainImages.data());

    _swapChainImageFormat = surfaceFormat.format;
    _swapChainExtent      = extent;

    return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool
SDLWindowVulkan::_CreateImageViews()
{
    _swapChainImageViews.resize(_swapChainImages.size());

    size_t index = 0;
    for (VkImage& image : _swapChainImages) {
        VkImageViewCreateInfo createInfo
        {
            .sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image                           = image,
            .viewType                        = VK_IMAGE_VIEW_TYPE_2D,
            .format                          = _swapChainImageFormat,
            .components = {
                .r                    = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g                    = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b                    = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a                    = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1,
            },
        };

        if (vkCreateImageView(_device, &createInfo, nullptr, &_swapChainImageViews[index]) != VK_SUCCESS) {
            LOG_ERROR("failed to create image views!");
            return false;
        }
        ++index;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
SDLWindowVulkan::_CreateRenderPass()
{
    VkAttachmentDescription colorAttachment {};
    colorAttachment.format  = _swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    // color and depth
    colorAttachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkSubpassDescription subpass {};
    {
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        VkAttachmentReference colorAttachmentRef {};
        {
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        // shader will write into this attachment with the following reference
        // layout(location = 0) out vec4 outColor
        subpass.colorAttachmentCount    = 1;
        subpass.pColorAttachments       = &colorAttachmentRef;
        subpass.pInputAttachments       = nullptr;   //  Attachments that are read from a shader
        subpass.pResolveAttachments     = nullptr;   //  Attachments used for multisampling color attachments
        subpass.pDepthStencilAttachment = nullptr;   //  Attachment for depth and stencil data
        subpass.pPreserveAttachments
            = nullptr;   //  Attachments that are not used by this subpass, but for which the data must be preserved
    }

    VkRenderPassCreateInfo renderPassInfo {};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments    = &colorAttachment;
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    {
        VkSubpassDependency dependency {};
        dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass    = 0;
        dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies   = &dependency;
    }

    if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS) {
        LOG_ERROR("failed to create render pass!");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool
SDLWindowVulkan::_CreateGraphicsPipeline()
{
    auto vertShaderCode = utils::readFile("shaders/shader.vert.spv");
    auto fragShaderCode = utils::readFile("shaders/shader.frag.spv");

    VkShaderModule fragmentShaderModule;
    core::Scoped   scopedFragmentShader(
        [&]() { fragmentShaderModule = gfx::createShaderModule(_device, fragShaderCode); },
        [&]() {
            vkDestroyShaderModule(_device, fragmentShaderModule, nullptr);
            fragmentShaderModule = VK_NULL_HANDLE;
        });

    VkShaderModule vertexShaderModule;
    core::Scoped   scopedVertexShader([&]() { vertexShaderModule = gfx::createShaderModule(_device, vertShaderCode); },
        [&]() {
            vkDestroyShaderModule(_device, vertexShaderModule, nullptr);
            vertexShaderModule = VK_NULL_HANDLE;
        });

    VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
    vertShaderStageInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage               = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module              = vertexShaderModule;
    vertShaderStageInfo.pName               = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr;   // specify shader constants

    VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
    fragShaderStageInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage               = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module              = fragmentShaderModule;
    fragShaderStageInfo.pName               = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr;   // specify shader constants

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    ////////////////////////////////////////////////////////////////////////////////

    VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
    vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount   = 0;
    vertexInputInfo.pVertexBindingDescriptions      = nullptr;   // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions    = nullptr;   // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
    inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    ////////////////////////////////////////////////////////////////////////////////

#if 1   // dynamic state to be defined
    /// at draw time
    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState {};
    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates    = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState {};
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount  = 1;
#else   // specify viewport/scissor statically
    VkViewport viewport {};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = (float)_swapChainExtent.width;
    viewport.height   = (float)_swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor {};
    scissor.offset = {0, 0};
    scissor.extent = _swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState {};
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports    = &viewport;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = &scissor;
#endif

    VkPipelineRasterizationStateCreateInfo rasterizer {};
    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;   // Optional
    rasterizer.depthBiasClamp          = 0.0f;   // Optional
    rasterizer.depthBiasSlopeFactor    = 0.0f;   // Optional

    VkPipelineMultisampleStateCreateInfo multisampling {};
    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0f;       // Optional
    multisampling.pSampleMask           = nullptr;    // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE;   // Optional
    multisampling.alphaToOneEnable      = VK_FALSE;   // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment {};
    colorBlendAttachment.colorWriteMask
        = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable         = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;    // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;   // Optional
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;        // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;    // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;   // Optional
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;        // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending {};
    colorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable     = VK_FALSE;
    colorBlending.logicOp           = VK_LOGIC_OP_COPY;   // Optional
    colorBlending.attachmentCount   = 1;
    colorBlending.pAttachments      = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;   // Optional
    colorBlending.blendConstants[1] = 0.0f;   // Optional
    colorBlending.blendConstants[2] = 0.0f;   // Optional
    colorBlending.blendConstants[3] = 0.0f;   // Optional

    ////////////////////////////////////////////////////////////////////////////////

    // define shader uniforms
    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount         = 0;         // Optional
    pipelineLayoutInfo.pSetLayouts            = nullptr;   // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0;         // Optional
    pipelineLayoutInfo.pPushConstantRanges    = nullptr;   // Optional

    if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
        LOG_ERROR("failed to create pipeline layout!");
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////

    VkGraphicsPipelineCreateInfo pipelineInfo {};
    pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount          = 2;
    pipelineInfo.pStages             = shaderStages;
    pipelineInfo.pVertexInputState   = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pDepthStencilState  = nullptr;   // Optional
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.pDynamicState       = &dynamicState;

    pipelineInfo.layout = _pipelineLayout;

    pipelineInfo.renderPass = _renderPass;
    pipelineInfo.subpass    = 0;

    // using the flag VK_PIPELINE_CREATE_DERIVATIVE_BIT we can create new pipelines from existing ones(cheaper)
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;   // Optional
    pipelineInfo.basePipelineIndex  = -1;               // Optional

    ////////////////////////////////////////////////////////////////////////////////

    VK_CHECK(vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline));

    return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool
SDLWindowVulkan::_CreateFramebuffers()
{
    _swapChainFramebuffers.resize(_swapChainImageViews.size());

    for (size_t i = 0; i < _swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {_swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo {};
        framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass      = _renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments    = attachments;
        framebufferInfo.width           = _swapChainExtent.width;
        framebufferInfo.height          = _swapChainExtent.height;
        framebufferInfo.layers          = 1;

        if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_swapChainFramebuffers[i]) != VK_SUCCESS) {
            LOG_ERROR("failed to create framebuffer!");
            return false;
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool
SDLWindowVulkan::_CreateCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(_physicalDevice, _surface);

    VkCommandPoolCreateInfo poolInfo {};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.optGraphicsFamily.value();

    if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
        LOG_ERROR("failed to create command pool!");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool
SDLWindowVulkan::_CreateCommandBuffer()
{
    _commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = _commandPool;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = _commandBuffers.size();

    VK_CHECK(vkAllocateCommandBuffers(_device, &allocInfo, _commandBuffers.data()));

    return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool
SDLWindowVulkan::_RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t swapchainIndex)
{
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    // flags:
    // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT:
    //   The command buffer will be rerecorded right after executing it once.
    // VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT:
    //   This is a secondary command buffer that will be entirely within a single render pass.
    // VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT:
    //   The command buffer can be resubmitted while it is also already pending execution.
    beginInfo.flags = 0;   // Optional

    beginInfo.pInheritanceInfo = nullptr;   // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        LOG_ERROR("failed to begin recording command buffer!");
        return false;
    }

    {   // renderpass
        VkRenderPassBeginInfo renderPassInfo {};
        renderPassInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass  = _renderPass;
        renderPassInfo.framebuffer = _swapChainFramebuffers[swapchainIndex];

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = _swapChainExtent;

        VkClearValue clearColor        = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues    = &clearColor;

        VkViewport viewport {};
        viewport.x        = 0.0f;
        viewport.y        = 0.0f;
        viewport.width    = static_cast<float>(_swapChainExtent.width);
        viewport.height   = static_cast<float>(_swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor {};
        scissor.offset = {0, 0};
        scissor.extent = _swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        {
            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

            uint32_t vertexCount   = 3;
            uint32_t instanceCount = 1;
            uint32_t firstVertex   = 0;
            uint32_t firstInstance = 0;
            vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);

            vkCmdEndRenderPass(commandBuffer);
        }
    }   // renderPass

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        LOG_ERROR("failed to record command buffer!");
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
SDLWindowVulkan::_CreateSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;   // first shot already signaled

    _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VK_CHECK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]));
        VK_CHECK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]));
        VK_CHECK(vkCreateFence(_device, &fenceInfo, nullptr, &_inFlightFences[i]));
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////

void
SDLWindowVulkan::_CleanupSwapChain()
{
    VK_DESTROY_LIST_WITH_DEVICE(vkDestroyFramebuffer, _device, _swapChainFramebuffers, nullptr);
    VK_DESTROY_LIST_WITH_DEVICE(vkDestroyImageView, _device, _swapChainImageViews, nullptr);
    VK_DESTROY_WITH_DEVICE(vkDestroySwapchainKHR, _device, _swapChain, nullptr);

    // no need to destroy
    _swapChainImages.clear();
}

////////////////////////////////////////////////////////////////////////////////

void
SDLWindowVulkan::_RecreateSwapChain()
{
    vkDeviceWaitIdle(_device);

    _CreateSwapChain();
    _CreateImageViews();
    _CreateFramebuffers();
}

/////////////////////////////////////////////////////////////////////////////////

#if USING(VALIDATION_LAYERS)

bool
SDLWindowVulkan::_AreInstanceLayersSupported(const std::vector<const char*>& validationLayers)
{
    ASSERT(!validationLayers.empty());

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

/////////////////////////////////////////////////////////////////////////////////

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    UNUSED(pUserData);

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) { }
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) { }
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) { }
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        std::cerr << "Error in validation layer: " << pCallbackData->pMessage << std::endl;
    }

    switch (messageType) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
        break;
    default:
        LOG_ERROR("unexpected message type");
        ASSERT(false);
    }

    return VK_FALSE;
}

void
SDLWindowVulkan::_PopulateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData       = nullptr;   // Optional
}

/////////////////////////////////////////////////////////////////////////////////

VkResult
CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

bool
SDLWindowVulkan::_InitDebugMessenger()
{
    if (_AreValidationLayersEnabled()) {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        _PopulateDebugMessenger(createInfo);
        if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS) {
            LOG_ERROR("failed to set up debug messenger!");
            return false;
        }
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////////

void
DestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

bool
SDLWindowVulkan::_DeinitDebugMessenger()
{
    if (_AreValidationLayersEnabled()) {
        DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
    }
    return true;
}

#endif   // #if USING(VALIDATION_LAYERS)

////////////////////////////////////////////////////////////////////////////////
}   // namespace gfx {

#include "SDLWindowVulkan.h"

#include "gfx/vk_types.h"
#include <SDL2/SDL_vulkan.h>

#include <vulkan/vulkan_core.h>

#include <algorithm>   // Necessary for std::clamp
#include <limits>   // Necessary for std::numeric_limits
#include <optional>
#include <set>

#define LIST_AVAILABLE_EXTENSIONS NOT_IN_USE

namespace gfx {
/////////////////////////////////////////////////////////////////////////////////

SDLWindowVulkan::SDLWindowVulkan()
    : SDLWindow()
    , _deviceExtensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME }
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
    result &= _CreateSwapchain();
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
    vkDestroySwapchainKHR(_device, _swapChain, nullptr);
    _swapChain = VK_NULL_HANDLE;

    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    _surface = VK_NULL_HANDLE;

    vkDestroyDevice(_device, nullptr);
    _device = VK_NULL_HANDLE;

#if USING(VALIDATION_LAYERS)
    _DeinitDebugMessenger();
#endif   // #if USING(VALIDATION_LAYERS)

    vkDestroyInstance(_instance, nullptr);
    _instance = nullptr;

    SDLWindow::Close();
}

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
        bool success = SDL_Vulkan_GetInstanceExtensions(_window, &requiredExtensionsCount, nullptr);
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
    //
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.pNext = nullptr;
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
    instanceCreateInfo.enabledExtensionCount = requiredExtensionNames.size();
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
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
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

    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    const bool extensionsSupported = checkDeviceExtensionSupport(device, deviceExtensions);
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
    queueCreateInfo.queueCount = 1;

    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures {};

    VkDeviceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.pEnabledFeatures = &deviceFeatures;
    {
        createInfo.enabledExtensionCount = _deviceExtensions.size();
        createInfo.ppEnabledExtensionNames = _deviceExtensions.data();
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
#if USING(VALIDATION_LAYERS)
        if (_AreValidationLayersEnabled()) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayers.size());
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
        std::set<uint32_t> uniqueQueueFamilies
            = { indices.optGraphicsFamily.value(), indices.optPresentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
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
    VK_PRESENT_MODE_MAILBOX_KHR
    VK_PRESENT_MODE_FIFO_KHR
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
        //const float wScale = width > 0 ? ((float)display_width / width) : 0;
        //const float hScale = height > 0 ? ((float)display_height / height) : 0;

        VkExtent2D actualExtent {
            .width = std::clamp(static_cast<uint32_t>(display_width),
                                      capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            .height = std::clamp(static_cast<uint32_t>(display_height), capabilities.minImageExtent.height,
                capabilities.maxImageExtent.height) };

        return actualExtent;
    }
}

bool
SDLWindowVulkan::_CreateSwapchain()
{
    const SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_physicalDevice, _surface);

    const VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    const VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    const VkExtent2D extent = chooseSwapExtent(_window, swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    const QueueFamilyIndices indices = findQueueFamilies(_physicalDevice, _surface);
    const uint32_t queueFamilyIndices[] = { indices.optGraphicsFamily.value(), indices.optPresentFamily.value() };

    if (indices.optGraphicsFamily != indices.optPresentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // slow!
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;       // Optional
        createInfo.pQueueFamilyIndices = nullptr;   // Optional
    }

    // i.e., use SAMSUNG extension for swapchain transform
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    //to be checked for support if needed//swapChainSupport.capabilities.supportedTransforms

    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // opaque surface
    createInfo.clipped = VK_TRUE;

    createInfo.presentMode = presentMode;

    createInfo.oldSwapchain = VK_NULL_HANDLE; // should point to old one when recreating (i.e., resize)

    if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain) != VK_SUCCESS) {
        LOG_ERROR("failed to create swap chain!");
        return false;
    }

    vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, nullptr);
    _swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, _swapChainImages.data());

    _swapChainImageFormat = surfaceFormat.format;
    _swapChainExtent = extent;

    return true;
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
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;   // Optional
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

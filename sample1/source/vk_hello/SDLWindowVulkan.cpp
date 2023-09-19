#include "SDLWindowVulkan.h"

#include "gfx/vk_types.h"
#include <SDL2/SDL_vulkan.h>

#include <vulkan/vulkan_core.h>

#include <iostream>
#include <optional>
#include <set>

#define LIST_AVAILABLE_EXTENSIONS NOT_IN_USE

/////////////////////////////////////////////////////////////////////////////////

SDLWindowVulkan::SDLWindowVulkan()
    : SDLWindow()
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
    assert(result);

    result &= _CreateInstance();
    assert(result);
    result &= _CreateSurface();
    assert(result);
    result &= _SelectAdapter();
    assert(result);
    result &= _CreateLogicalDevice();
    assert(result);

#if USING(VALIDATION_LAYERS)
    result &= _InitDebugMessenger();
    assert(result);
#endif   // #if USING(VALIDATION_LAYERS)

    return result;
}

void
SDLWindowVulkan::Close()
{
    vkDestroySurfaceKHR(_instance, _surface, nullptr);

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
        LOG_ERROR("Validation layers enabled but not supported.");
        _validationLayersEnabled = false;
    }
#endif   // #if USING(VALIDATION_LAYERS)

    std::vector<const char*> requiredExtensionNames;
    {
        unsigned int requiredExtensionsCount = 0;
        bool success = SDL_Vulkan_GetInstanceExtensions(_window, &requiredExtensionsCount, nullptr);
        assert(success);
        requiredExtensionNames.resize(requiredExtensionsCount);
        success = SDL_Vulkan_GetInstanceExtensions(_window, &requiredExtensionsCount, requiredExtensionNames.data());
        assert(success);
    }

    requiredExtensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    std::cout << "Required extensions(" << requiredExtensionNames.size() << "):" << std::endl;
    for (const char* name : requiredExtensionNames) {
        std::cout << name << std::endl;
    }

    if (USING(LIST_AVAILABLE_EXTENSIONS)) {   // retrieve available extensions
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availabelExtensionNames(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availabelExtensionNames.data());

        std::cout << "Available extensions(" << availabelExtensionNames.size() << "):\n";
        for (const auto& extension : availabelExtensionNames) {
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

    bool IsComplete() { return optGraphicsFamily.has_value() && optPresentFamily.has_value(); }
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
SDLWindowVulkan::_IsPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) const
{
    VkPhysicalDeviceFeatures adapterFeatures;
    vkGetPhysicalDeviceFeatures(device, &adapterFeatures);
    VkPhysicalDeviceProperties adapterProperties;
    vkGetPhysicalDeviceProperties(device, &adapterProperties);

    LOG_DEBUG("PhysDev: %s | %s\n", adapterProperties.deviceName, adapterProperties.deviceID);

    assert(adapterProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);

    QueueFamilyIndices queueFamily = findQueueFamilies(device, surface);
    return
        // kk adapterProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
        /* && adapterFeatures.multiDrawIndirect */
        queueFamily.IsComplete();
}

bool
SDLWindowVulkan::_SelectAdapter()
{
    assert(_surface != nullptr);

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
    assert(_physicalDevice != VK_NULL_HANDLE);

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
    assert(_physicalDevice != nullptr && _surface != nullptr);

    QueueFamilyIndices indices = findQueueFamilies(_physicalDevice, _surface);

    VkDeviceQueueCreateInfo queueCreateInfo {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    assert(indices.optGraphicsFamily.has_value());
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
    {   // Legacy: only needed to support old drivers. Not used in modern implementations.
        createInfo.enabledExtensionCount = 0;
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
        if (_AreValidationLayersEnabled()) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayers.size());
            createInfo.ppEnabledLayerNames = _validationLayers.data();
        }
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
    if (_presentQueue == VK_NULL_HANDLE)
    {
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

/////////////////////////////////////////////////////////////////////////////////

#if USING(VALIDATION_LAYERS)

bool
SDLWindowVulkan::_AreInstanceLayersSupported(const std::vector<const char*>& validationLayers)
{
    assert(!validationLayers.empty());

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
        assert(false);
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

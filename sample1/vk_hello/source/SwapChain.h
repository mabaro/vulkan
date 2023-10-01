#pragma once

#include <vulkan/vulkan.h>

class ISwapChain {
public:
	virtual VkResult AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex) = 0;
	virtual VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE) = 0;
};
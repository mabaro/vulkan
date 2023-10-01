#pragma once

#include "gfx/vk_types.h"

namespace gfx {
namespace vk_init {
/////////////////////////////////////////////////////////////////////////////////

inline VkFenceCreateInfo
fence_create_info(bool signaled)
{
    VkFenceCreateInfo fenceInfo {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;   // first shot already signaled

    return fenceInfo;
}

/////////////////////////////////////////////////////////////////////////////////

VkCommandPoolCreateInfo
command_pool_create_info(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0)
{
    VkCommandPoolCreateInfo info = {};
    info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext                   = nullptr;

    info.queueFamilyIndex = queueFamilyIndex;
    info.flags            = flags;
    return info;
}

/////////////////////////////////////////////////////////////////////////////////

inline VkCommandBufferAllocateInfo
command_buffer_allocate_info(
    VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY)
{
    VkCommandBufferAllocateInfo info = {};
    info.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext                       = nullptr;

    info.commandPool        = pool;
    info.commandBufferCount = count;
    info.level              = level;
    return info;
}

/////////////////////////////////////////////////////////////////////////////////

inline VkCommandBufferBeginInfo
command_buffer_begin_info(VkCommandBufferUsageFlags flags = 0)
{
    VkCommandBufferBeginInfo info = {};
    info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext                    = nullptr;

    info.pInheritanceInfo = nullptr;
    info.flags            = flags;
    return info;
}

/////////////////////////////////////////////////////////////////////////////////

inline VkSubmitInfo
submit_info(VkCommandBuffer* cmd)
{
    VkSubmitInfo info = {};
    info.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.pNext        = nullptr;

    info.waitSemaphoreCount   = 0;
    info.pWaitSemaphores      = nullptr;
    info.pWaitDstStageMask    = nullptr;
    info.commandBufferCount   = 1;
    info.pCommandBuffers      = cmd;
    info.signalSemaphoreCount = 0;
    info.pSignalSemaphores    = nullptr;

    return info;
}

/////////////////////////////////////////////////////////////////////////////////
}   // namespace gfx {
}   // namespace vk_init

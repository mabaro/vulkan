#pragma once

#include "core/core.h"
#include <vulkan/vulkan.h>

#include <iostream>

#define VK_CHECK(x)                                              \
    do {                                                         \
        VkResult err = x;                                        \
        if (err) {                                               \
            ASSERT_MSG(false, "Detected Vulkan error: %d", err); \
            std::abort();                                        \
        }                                                        \
    } while (0)

#define VK_CHECK_MSG(x, msg, ...)                                \
    do {                                                         \
        VkResult err = x;                                        \
        if (err) {                                               \
            ASSERT_MSG(false, "Detected Vulkan error: %d", err); \
            LOG_ERROR(msg, ##__VA_ARGS__);                       \
            std::abort();                                        \
        }                                                        \
    } while (0)

/////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "core/core.h"
#include <vulkan/vulkan.h>

#include <iostream>

#define VK_CHECK(x)                                       \
    do {                                                  \
        VkResult err = x;                                 \
        if (err) {                                        \
            FAIL_MSG("Vulkan error: [%d] @ %s", err, #x); \
        }                                                 \
    } while (0)

#define VK_CHECK_MSG(x, msg, ...)                         \
    do {                                                  \
        VkResult err = x;                                 \
        if (err) {                                        \
            FAIL_MSG("Vulkan error: [%d] @ %s", err, #x); \
            LOG_ERROR(msg, ##__VA_ARGS__);                \
        }                                                 \
    } while (0)

/////////////////////////////////////////////////////////////////////////////////
#define VK_DESTROY(func, object, ...) \
    do {                              \
        func(object, ##__VA_ARGS__);  \
        object = VK_NULL_HANDLE;      \
    } while (0)

#define VK_DESTROY_WITH_DEVICE(func, device, object, ...) \
    do {                                                  \
        func(device, object, ##__VA_ARGS__);              \
        object = VK_NULL_HANDLE;                          \
    } while (0)

#define VK_DESTROY_LIST_WITH_DEVICE(func, device, object_list, ...)       \
    do {                                                                  \
        for (auto object : object_list) {                                 \
            VK_DESTROY_WITH_DEVICE(func, _device, object, ##__VA_ARGS__); \
        }                                                                 \
        object_list.clear();                                              \
    } while (0)
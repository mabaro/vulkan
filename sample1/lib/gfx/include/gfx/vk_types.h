#pragma once

#include <vulkan/vulkan.h>

#include <iostream>

#define VK_CHECK(x)                                                            \
    do {                                                                       \
        VkResult err = x;                                                      \
        if (err) {                                                             \
            std::cout << "Detected Vulkan error: " << err << std::endl;        \
            abort();                                                           \
        }                                                                      \
    } while (0)

#define LOG_ERROR(X) fprintf(stderr, "Error: %s\n", X)

/////////////////////////////////////////////////////////////////////////////////

#define USING(_X_) _X_&& _X_
#define IN_USE 1
#define NOT_IN_USE 0

/////////////////////////////////////////////////////////////////////////////////

#ifdef NDEBUG
#define IS_DEBUG IN_USE
#else
#define IS_DEBUG NOT_IN_USE
#endif


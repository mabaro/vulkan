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

#define IS_DEBUG defined(_NDEBUG)
#define USING(_X_) _X_ && _X_
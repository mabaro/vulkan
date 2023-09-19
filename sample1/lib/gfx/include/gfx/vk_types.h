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

#ifdef UNUSED
#elif defined(__GNUC__)
#define UNUSED(x) (void)x
#elif defined(__LCLINT__)
#define UNUSED(x) /*@unused@*/ x
#elif defined(__cplusplus)
#define UNUSED(x)
#else
#define UNUSED(x) x
#endif

////////////////////////////////////////////////////////////////////////////////

static volatile struct LoggerConfiguration {
    bool showFilename = false;
    FILE* forceOutput = nullptr;
} s_loggerConfiguration;

template <typename... T>
static inline void
LogFormatter(FILE* output, const char* filename, uint32_t line, const char* level, const char* format, T&&... args)
{
    output = s_loggerConfiguration.forceOutput != nullptr ? s_loggerConfiguration.forceOutput : output;
    if (s_loggerConfiguration.showFilename) {
        fprintf(output, "%s:%d: ", filename, line);
    }
    fprintf(output, "[%s]: ", level);
    fprintf(output, format, std::forward<T>(args)...);
    fprintf(output, "\n");
}

#define LOG_ERROR(str, ...) LogFormatter(stderr, __FILE__, __LINE__, "ERROR", str, ##__VA_ARGS__)
#define LOG_WARN(str, ...)  LogFormatter(stderr, __FILE__, __LINE__, "WARN", str, ##__VA_ARGS__)
#define LOG_INFO(str, ...)  LogFormatter(stdout, __FILE__, __LINE__, "INFO", str, ##__VA_ARGS__)
#if USING(IS_DEBUG)
#define LOG_DEBUG(str, ...) LogFormatter(stdout, __FILE__, __LINE__, "DEBUG", str, ##__VA_ARGS__)
#else // #if USING(IS_DEBUG)
#define LOG_DEBUG(...)
#endif // #else // #if USING(IS_DEBUG)

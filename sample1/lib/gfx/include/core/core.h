#pragma once

#include <iostream>

/////////////////////////////////////////////////////////////////////////////////

#define USING(_X_) _X_&& _X_
#define IN_USE     1
#define NOT_IN_USE 0

/////////////////////////////////////////////////////////////////////////////////

#define FAIL_ABORT NOT_IN_USE

/////////////////////////////////////////////////////////////////////////////////

#if !defined(NDEBUG)
#define IS_DEBUG IN_USE
// #warning "This is a DEBUG build"
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
namespace core {
namespace impl {
static volatile struct LoggerConfiguration {
    bool  showFilename = true;
    FILE* forceOutput  = nullptr;
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

}   // impl
}   // namespace

#define LOG_ERROR(str, ...) core::impl::LogFormatter(stderr, __FILE__, __LINE__, "ERROR", str, ##__VA_ARGS__)
#define LOG_WARN(str, ...)  core::impl::LogFormatter(stderr, __FILE__, __LINE__, "WARN", str, ##__VA_ARGS__)
#define LOG_INFO(str, ...)  core::impl::LogFormatter(stdout, __FILE__, __LINE__, "INFO", str, ##__VA_ARGS__)
#if USING(IS_DEBUG)
#define LOG_DEBUG(str, ...) core::impl::LogFormatter(stdout, __FILE__, __LINE__, "DEBUG", str, ##__VA_ARGS__)
#else   // #if USING(IS_DEBUG)
#define LOG_DEBUG(...)
#endif   // #else // #if USING(IS_DEBUG)

/////////////////////////////////////////////////////////////////////////////////

#include "core/debug_break/debugbreak.h"
#if USING(IS_DEBUG)
#define DebugBreak() debug_break()
#else   // #if USING(IS_DEBUG)
#define DebugBreak()
#endif   // #else   // #if USING(IS_DEBUG)

#if USING(FAIL_ABORT)
#define HALT()        \
    do {              \
        DebugBreak(); \
        abort();      \
    } while (1)
#else   // #if USING(FAIL_ABORT)
#define HALT() DebugBreak()
#endif   // #else   // #if USING(FAIL_ABORT)


////////////////////////////////////////////////////////////////////////////////

namespace core {
namespace impl {
inline void
assert_handler(const char* cond, const char* filename, uint32_t line)
{
    fprintf(stderr, "%s:%d: Assertion failed: [%s]\n", filename, line, cond);
}
}   // impl
}   // namespace

#if USING(IS_DEBUG)
#define FAIL() HALT()
#define FAIL_MSG(msg, ...)             \
    do {                               \
        LOG_ERROR(msg, ##__VA_ARGS__); \
        HALT();                        \
    } while (0)
#define ASSERT(cond)                                               \
    do {                                                           \
        if (!(cond)) {                                             \
            core::impl::assert_handler(#cond, __FILE__, __LINE__); \
            HALT();                                                \
        }                                                          \
    } while (0)
#define ASSERT_MSG(cond, msg, ...)                                 \
    do {                                                           \
        if (!(cond)) {                                             \
            core::impl::assert_handler(#cond, __FILE__, __LINE__); \
            LOG_ERROR(msg, ##__VA_ARGS__);                         \
            HALT();                                                \
        }                                                          \
    } while (0)
#else   // #if USING(IS_DEBUG)
#define FAIL()
#define FAIL_MSG(...)
#define ASSERT(...)
#define ASSERT_MSG(...)
#endif   // #define ASSERT(X, ...)

#define DISABLE_OPTIMIZATION //#pragma clang optimize off

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include <functional>

namespace core {
////////////////////////////////////////////////////////////////////////////////

template <typename T>
bool
isAligned(const T value, size_t alignmentBytes)
{
    T* valuePtr = &value;
    return (valuePtr % alignmentBytes) == 0;
}

template <typename EnterFuncT = std::function<void(void)>, typename ExitFuncT = std::function<void(void)>>
struct Scoped {
private:
    ExitFuncT _onExit;

public:
    Scoped(ExitFuncT onExit)
        : _onExit(onExit)
    {
    }
    Scoped(EnterFuncT onEnter, ExitFuncT onExit)
        : _onExit(onExit)
    {
        onEnter();
    }
    ~Scoped() { _onExit(); }
};

////////////////////////////////////////////////////////////////////////////////
}   // namespace core {

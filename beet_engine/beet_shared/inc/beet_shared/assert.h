#ifndef BEETROOT_ASSERT_H
#define BEETROOT_ASSERT_H

#include <cstdio>

#if defined(__linux__) || defined(__APPLE__)
#include <signal.h>
#define BVM_DEBUG_BREAK() raise(SIGTRAP)
#elif defined(_WIN32)
#define DEBUG_BREAK() __debugbreak()
#endif

#define LOG_ERROR(...) printf(__VA_ARGS__);
#define LOG_CURRENT_POSITION() \
    LOG_ERROR("Assertion triggered \n\t file: \t %s \n\t func: \t %s \n\t line: \t %d \n", __FILE__, __FUNCTION__, __LINE__);

#define ASSERT_MSG(condition, ...)          \
{                                           \
    if(!(condition)){                       \
        LOG_CURRENT_POSITION();             \
        LOG_ERROR(__VA_ARGS__);             \
        DEBUG_BREAK();                      \
    }                                       \
}

#define ASSERT(condition)                   \
    {                                       \
        if (!(condition)) {                 \
            LOG_CURRENT_POSITION();         \
            DEBUG_BREAK();                  \
        }                                   \
    }

#define SANITY_CHECK()                      \
{                                           \
    LOG_CURRENT_POSITION();                 \
    LOG_ERROR("Sanity check failed ;~;");   \
    DEBUG_BREAK();                          \
}

#endif //BEETROOT_ASSERT_H
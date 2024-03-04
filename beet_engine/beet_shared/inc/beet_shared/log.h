#ifndef BEETROOT_LOG_H
#define BEETROOT_LOG_H

#include <cstdio>
#include <chrono>
#include <ctime>

enum MSG_LEVEL : uint8_t {
    MSG_VERBOSE = 0u,
    MSG_INFO = 1u,
    MSG_DEBUG = 2u,
    MSG_WARNING = 3u,
    MSG_ERROR = 4u,
    MSG_CRITICAL = 5u,
};

enum MSG_CHANNEL : uint32_t {
    MSG_NONE = 0u,
    MSG_RUNTIME = 1u << 1u,
    MSG_SERVER = 1u << 2u,
    MSG_PIPELINE = 1u << 3u,

    MSG_GFX = 1u << 4u,
    MSG_MATH = 1u << 5u,
    MSG_DDS = 1u << 6u,

    MSG_DBG = 1u << 31u,
    MSG_ALL = UINT32_MAX,
};


#define MSG_MIN_WARNING_LEVEL MSG_VERBOSE
#define MSG_ACTIVE_CHANNELS ((MSG_ALL) & ~MSG_DBG)
//#define MSG_ACTIVE_CHANNELS MSG_DBG

#if BEET_DEBUG

const char *log_channel_name_lookup(const MSG_CHANNEL &channel);

#define log_verbose(channel, ...){                              \
    if(channel & MSG_ACTIVE_CHANNELS){                          \
        beet_log(MSG_VERBOSE, channel,"[verbose]", __VA_ARGS__) \
    }                                                           \
}

#define log_info(channel, ...){                             \
    if(channel & MSG_ACTIVE_CHANNELS){                      \
        beet_log(MSG_INFO, channel,"[info]", __VA_ARGS__)   \
    }                                                       \
}

#define log_debug(channel, ...){                            \
    if(channel & MSG_ACTIVE_CHANNELS){                      \
        beet_log(MSG_DEBUG, channel,"[debug]",__VA_ARGS__)  \
    }                                                       \
}

#define log_warning(channel, ...){                              \
    if(channel & MSG_ACTIVE_CHANNELS){                          \
        beet_log(MSG_WARNING, channel,"[warning]", __VA_ARGS__) \
    }                                                           \
}

#define log_error(channel, ...){                            \
    if(channel & MSG_ACTIVE_CHANNELS){                      \
        beet_log(MSG_ERROR, channel,"[error]", __VA_ARGS__) \
    }                                                       \
}

#define log_critical(channel, ...){                                 \
    if(channel & MSG_ACTIVE_CHANNELS){                              \
        beet_log(MSG_CRITICAL, channel,"[critical]",  __VA_ARGS__)  \
    }                                                               \
}

#define beet_log(level, channel, levelName, ...){                       \
    time_t t = time(nullptr);                                           \
    struct tm buf{};                                                    \
    localtime_r(&t, &buf);                                              \
    if(level >= MSG_MIN_WARNING_LEVEL){                                 \
        printf("[%i:%i:%i]", buf.tm_hour, buf.tm_min, buf.tm_sec);      \
        printf("%s", levelName);                                        \
        const char* channelName = log_channel_name_lookup(channel);     \
        printf("%s : ", channelName);                                   \
        printf(__VA_ARGS__);                                            \
    }                                                                   \
}
#else
#define log_verbose(channel, ...){}
#define log_info(channel, ...){}
#define log_debug(channel, ...){}
#define log_warning(channel, ...){}
#define log_error(channel, ...){}
#define log_critical(channel, ...){}

#define beet_log(level, channel, levelName, ...){}
#endif

#endif //BEETROOT_LOG_H
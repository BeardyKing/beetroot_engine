#include <beet_shared/feature_defines.h>

#if CHECK_FEATURE(FEATURE_PLATFORM_LINUX)
#include <cstddef>
#include <sys/time.h>
#include <stdint.h>

//===INTERNAL_STRUCTS===================================================================================================
static struct Time {
    struct timeval startTime;
    struct timeval lastTime;
    struct timeval currentTime;
    double timeScaleDelta;
    double timeScale;
    uint32_t frameCount;
    double deltaTime;
} s_time = {};
//======================================================================================================================

//===API================================================================================================================
double time_delta() {
    return s_time.deltaTime;
}

double time_current() {
    return (s_time.currentTime.tv_sec - s_time.startTime.tv_sec) +
           (s_time.currentTime.tv_usec - s_time.startTime.tv_usec) / 1e6;
}

uint32_t time_frame_count() {
    return s_time.frameCount;
}

void time_tick() {
    //FIXME:HACK: This is deeply cursed, Ubuntu is not allowing me to use ctime/time.h as it's trying to use gcc headers when compiling clang.
    gettimeofday(&s_time.currentTime, NULL);

    double lastTimeInSec = s_time.lastTime.tv_sec + s_time.lastTime.tv_usec / 1e6;
    double currentTimeInSec = s_time.currentTime.tv_sec + s_time.currentTime.tv_usec / 1e6;

    s_time.deltaTime = currentTimeInSec - lastTimeInSec;
    s_time.lastTime = s_time.currentTime;
    s_time.frameCount++;
}
//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void time_create() {
    gettimeofday(&s_time.startTime, NULL);
    s_time.lastTime = s_time.startTime;
    s_time.frameCount = 0;
    s_time.deltaTime = 0.0;
    s_time.timeScaleDelta = 1.0;
}

void time_cleanup() {
    s_time = (struct Time){0};
}
//======================================================================================================================
#endif //CHECK_FEATURE(FEATURE_PLATFORM_LINUX)
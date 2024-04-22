#include <beet_shared/platform_defines.h>

#if PLATFORM_WINDOWS

#include <beet_core/time.h>
#include <windows.h>

//===INTERNAL_STRUCTS===================================================================================================
static struct Time {
    double timeOnStartUp;
    double lastTime;
    double currentTime;
    double timeScaleDelta;
    double timeScale;
    double frequency;
    uint32_t frameCount;
    double deltaTime;
} s_time = {};
//======================================================================================================================

//===API================================================================================================================
double time_delta() {
    return s_time.deltaTime;
}

double time_current() {
    return s_time.currentTime - s_time.timeOnStartUp;
}

uint32_t time_frame_count() {
    return s_time.frameCount;
}

void time_tick() {
    LARGE_INTEGER timeNow;

    QueryPerformanceCounter(&timeNow);

    s_time.frameCount += 1;
    s_time.lastTime = s_time.currentTime;
    s_time.currentTime = (double) timeNow.QuadPart / s_time.frequency;
    s_time.deltaTime = (s_time.currentTime - s_time.lastTime);
}
//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void time_create() {
    LARGE_INTEGER now;
    LARGE_INTEGER frequency;

    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&frequency);

    s_time.timeOnStartUp = (double) now.QuadPart / (double) frequency.QuadPart;
    s_time.lastTime = (double) now.QuadPart / (double) frequency.QuadPart;
    s_time.currentTime = (double) now.QuadPart / (double) frequency.QuadPart;
    s_time.timeScaleDelta = 1.0;
    s_time.timeScale = 1000.0;
    s_time.frequency = (double) frequency.QuadPart;
    s_time.frameCount = 0;
}

void time_cleanup() {
    s_time = {0};
}
//======================================================================================================================

#endif
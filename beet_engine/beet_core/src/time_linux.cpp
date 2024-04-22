#include <beet_shared/platform_defines.h>

#if PLATFORM_LINUX

#include <beet_core/time.h>

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
}
//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void time_create() {

}

void time_cleanup() {
    s_time = {0};
}
//======================================================================================================================

#endif
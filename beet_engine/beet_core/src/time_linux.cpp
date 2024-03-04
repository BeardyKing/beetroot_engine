#include <beet_shared/platform_defines.h>
#if PLATFORM_LINUX

#include <beet_core/time.h>

//===internal structs========
static struct Time {
    double timeOnStartUp;
    double lastTime;
    double currentTime;
    double timeScaleDelta;
    double timeScale;
    double frequency;
    uint32_t frameCount;
    double deltaTime;
} g_time = {};

//===internal functions======
//===api=====================
double time_delta() {
    return g_time.deltaTime;
}

double time_current() {
    return g_time.currentTime - g_time.timeOnStartUp;
}

uint32_t time_frame_count() {
    return g_time.frameCount;
}

void time_tick() {
}

//===init & shutdown=========
void time_create() {

}

void time_cleanup() {
    g_time = {0};
}

#endif
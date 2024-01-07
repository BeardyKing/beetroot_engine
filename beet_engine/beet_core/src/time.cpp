#include <beet_core/time.h>

#include <windows.h>

//===internal structs========
struct Time {
    double timeOnStartUp;
    double lastTime;
    double currentTime;
    double timeScaleDelta;
    double timeScale;
    double frequency;
    uint32_t frameCount;
    double deltaTime;
};

Time *g_time;

//===internal functions======
//===api=====================
double time_delta() {
    return g_time->deltaTime;
}

double time_current() {
    return g_time->currentTime - g_time->timeOnStartUp;
}

uint32_t time_frame_count() {
    return g_time->frameCount;
}

void time_tick() {
    LARGE_INTEGER timeNow;

    QueryPerformanceCounter(&timeNow);

    g_time->frameCount += 1;
    g_time->lastTime = g_time->currentTime;
    g_time->currentTime = (double) timeNow.QuadPart / g_time->frequency;
    g_time->deltaTime = (g_time->currentTime - g_time->lastTime);
}

//===init & shutdown=========
void time_create() {
    LARGE_INTEGER now;
    LARGE_INTEGER frequency;

    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&frequency);

    g_time = (Time *) malloc(sizeof(Time));
    memset(g_time, 0, sizeof(Time));


    g_time->timeOnStartUp = (double) now.QuadPart / (double) frequency.QuadPart;
    g_time->lastTime = (double) now.QuadPart / (double) frequency.QuadPart;
    g_time->currentTime = (double) now.QuadPart / (double) frequency.QuadPart;
    g_time->timeScaleDelta = 1.0;
    g_time->timeScale = 1000.0;
    g_time->frequency = (double) frequency.QuadPart;
    g_time->frameCount = 0;
}

void time_cleanup() {
    free(g_time);
    g_time = nullptr;
}


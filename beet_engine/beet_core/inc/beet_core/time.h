#ifndef BEETROOT_TIME_H
#define BEETROOT_TIME_H

#include <cstdint>

//===api=====================
void time_tick();
double time_delta();
double time_current();
uint32_t time_frame_count();

//===init & shutdown=========
void time_create();
void time_cleanup();

#endif //BEETROOT_TIME_H

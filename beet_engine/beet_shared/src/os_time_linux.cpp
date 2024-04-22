#include <beet_shared/os_time.h>
#include <beet_shared/platform_defines.h>

#if PLATFORM_LINUX

//===API================================================================================================================
void os_localtime(struct tm &buf, time_t &time) {
    localtime_r(&t, &buf);
}
//======================================================================================================================

#endif
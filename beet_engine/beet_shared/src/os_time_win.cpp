#include <beet_shared/os_time.h>
#include <beet_shared/platform_defines.h>

#if CHECK_FEATURE(PLATFORM_WINDOWS)

//===API================================================================================================================
void os_localtime(struct tm &buf, time_t &time) {
    localtime_s(&buf, &time);
}
//======================================================================================================================

#endif
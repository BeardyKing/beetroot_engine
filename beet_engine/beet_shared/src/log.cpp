#include <beet_shared/log.h>

const char *log_channel_name_lookup(const MSG_CHANNEL &channel) {
    switch (channel) {
        case MSG_RUNTIME:
            return "[runtime]";
        case MSG_SERVER:
            return "[server]";
        case MSG_PIPELINE:
            return "[pipeline]";
        case MSG_CONVERTER:
            return "[converter]";

        case MSG_GFX:
            return "[gfx]";
        case MSG_MATH:
            return "[math]";
        case MSG_DDS:
            return "[dds]";

        case MSG_DBG:
            return "[debugging]";

        case MSG_ALL:
            return "[all]";

        case MSG_NONE:
            break;
    }
    return "[unknown]";
}
#include <beet_core/window.h>
#include "beet_core/time.h"
#include "beet_core/input.h"
#include "beet_shared/log.h"

int main() {
    window_create("beetroot engine - runtime", {1024, 769});
    time_create();
    input_create();

    while (window_is_open()) {
        time_tick();
        input_set_time(time_current());
        window_update();
        input_update();
        //TODO: GFX backend.

        log_info(MSG_RUNTIME, "hello beetroot engine\n");
    }

    input_cleanup();
    time_cleanup();
    window_cleanup();
}
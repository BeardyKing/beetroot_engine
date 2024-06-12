#include <beet_core/window.h>
#include <beet_core/time.h>
#include <beet_core/input.h>

#include <beet_shared/log.h>
#include <beet_shared/memory.h>

#include <beet_gfx/gfx_interface.h>
#include <beet_gfx/gfx_imgui.h>

#include <runtime/entity_builder.h>
#include <runtime/script_camera.h>

#if BEET_GFX_IMGUI

#include <runtime/widget_manager.h>

#endif //BEET_GFX_IMGUI

#if BEET_GFX_IMGUI
void imgui_update() {
    gfx_imgui_begin();
    {
        widget_manager_update();
    }
}
#endif //BEET_GFX_IMGUI

int main() {
    window_create("beetroot engine - runtime", {1024, 768});
#if BEET_GFX_IMGUI
    window_set_procedure_callback_func(gfx_imgui_get_win32_proc_function_pointer());
#endif //BEET_GFX_IMGUI
    time_create();
    input_create();
    gfx_create(window_get_handle());
    entities_create();

    log_info(MSG_RUNTIME, "hello beetroot engine\n");
    while (window_is_open()) {
        time_tick();
        input_set_time(time_current());
        window_update();
        input_update();
        script_update_camera();
#if BEET_GFX_IMGUI
        imgui_update();
#endif //BEET_GFX_IMGUI
        gfx_update(time_delta());
    }
    entities_cleanup();
    gfx_cleanup();
    input_cleanup();
    time_cleanup();
    window_cleanup();
#if BEET_MEMORY_DEBUG
    mem_dump_memory_info();
    mem_validate_empty();
#endif //BEET_MEMORY_DEBUG
}
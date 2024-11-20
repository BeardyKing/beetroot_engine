#include <beet_core/window.h>
#include <beet_core/time.h>
#include <beet_core/input.h>

#include <beet_shared/log.h>
#include <beet_shared/memory.h>

#include <beet_gfx/vulkan/gfx_vk_imgui.h>
#include <beet_gfx/gfx_interface.h>
#include <beet_gfx/db_asset.h>

#include <runtime/entity_builder.h>
#include <runtime/scripts/script.h>

#if CHECK_FEATURE(FEATURE_GFX_IMGUI)

#include <runtime/widget_manager.h>

#endif //CHECK_FEATURE(FEATURE_GFX_IMGUI)

#if CHECK_FEATURE(FEATURE_GFX_IMGUI)
void imgui_update() {
    gfx_imgui_begin();
}
#endif //CHECK_FEATURE(FEATURE_GFX_IMGUI)

int main() {
    const vec2i windowSize = {1024, 768};
    window_create("beetroot engine - runtime", windowSize, {});
#if CHECK_FEATURE(FEATURE_GFX_IMGUI)
    window_set_procedure_callback_func(gfx_imgui_get_proc_function_pointer());
#endif //CHECK_FEATURE(FEATURE_GFX_IMGUI)
    time_create();
    input_create();
    gfx_create(window_get_handle());
    entities_create();
    script_create();
    log_info(MSG_RUNTIME, "hello beetroot engine\n");
    while (window_is_open()) {
        time_tick();
        input_set_time(time_current());
        window_update();
        input_update();
#if CHECK_FEATURE(FEATURE_GFX_IMGUI)
        imgui_update();
#endif //CHECK_FEATURE(FEATURE_GFX_IMGUI)
        script_update(time_delta_f());
        widget_manager_update();
        gfx_update(time_delta_d());
    }
    db_dump_pool_alloc_table();
    entities_cleanup();
    gfx_cleanup();
    input_cleanup();
    time_cleanup();
    window_cleanup();
    db_cleanup_pools();
#if CHECK_FEATURE(FEATURE_MEMORY_TRACKING)
    mem_dump_memory_info();
    mem_validate_empty();
#endif //CHECK_FEATURE(FEATURE_MEMORY_TRACKING)
}
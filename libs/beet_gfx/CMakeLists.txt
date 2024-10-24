project(beet_gfx)
add_library(beet_gfx
#       GFX SHARED
        inc/beet_gfx/gfx_interface.h
        inc/beet_gfx/IconsFontAwesome5.h
        inc/beet_gfx/gfx_generate_geometry.h
        inc/beet_gfx/gfx_converter.h
        inc/beet_gfx/db_asset.h
        src/gfx_generate_geometry.cpp
        src/gfx_converter.cpp
        src/db_asset.cpp

#       GFX IMGUI X BACKEND
        inc/beet_gfx/imgui/imgui_impl_x11.h
        src/imgui/imgui_impl_x11.cpp

#       GFX VULKAN
        inc/beet_gfx/vulkan/gfx_vk_platform_defines.h
        inc/beet_gfx/vulkan/gfx_vulkan_surface.h
        inc/beet_gfx/vulkan/gfx_vk_samplers.h
        inc/beet_gfx/vulkan/gfx_vk_mesh.h
        inc/beet_gfx/vulkan/gfx_vk_pipeline.h
        inc/beet_gfx/vulkan/gfx_vk_descriptors.h
        inc/beet_gfx/vulkan/gfx_vk_shader.h
        inc/beet_gfx/vulkan/gfx_vk_buffer.h
        inc/beet_gfx/vulkan/gfx_vk_utils.h
        inc/beet_gfx/vulkan/gfx_vk_command.h
        inc/beet_gfx/vulkan/gfx_vk_function_pointers.h
        inc/beet_gfx/vulkan/gfx_vk_debug.h
        inc/beet_gfx/vulkan/gfx_vk_imgui.h
        inc/beet_gfx/vulkan/gfx_vk_lit.h
        inc/beet_gfx/vulkan/gfx_vk_texture.h
        inc/beet_gfx/vulkan/gfx_vk_sky.h
        inc/beet_gfx/vulkan/gfx_vk_line.h
        inc/beet_gfx/vulkan/gfx_vk_triangle_strip.h
        src/vulkan/gfx_backend_vulkan.cpp
        src/vulkan/gfx_vulkan_surface_win.cpp
        src/vulkan/gfx_vk_samplers.cpp
        src/vulkan/gfx_vulkan_surface_linux.cpp
        src/vulkan/gfx_vk_pipeline.cpp
        src/vulkan/gfx_vk_descriptors.cpp
        src/vulkan/gfx_vk_shader.cpp
        src/vulkan/gfx_vk_buffer.cpp
        src/vulkan/gfx_vk_utils.cpp
        src/vulkan/gfx_vk_command.cpp
        src/vulkan/gfx_vk_mesh.cpp
        src/vulkan/gfx_vk_function_pointers.cpp
        src/vulkan/gfx_vk_debug.cpp
        src/vulkan/gfx_vk_imgui.cpp
        src/vulkan/gfx_vk_lit.cpp
        src/vulkan/gfx_vk_texture.cpp
        src/vulkan/gfx_vk_sky.cpp
        src/vulkan/gfx_vk_line.cpp
        src/vulkan/gfx_vk_triangle_strip.cpp
)

target_include_directories(beet_gfx
        PUBLIC inc
        PRIVATE src
        PUBLIC third/imgui/
)

find_package(Vulkan REQUIRED)
target_link_libraries(beet_gfx
        beet_shared
        beet_math
        beet_converter

        Vulkan::Vulkan
        imgui
)

set_target_properties(beet_gfx PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

target_compile_definitions(beet_gfx PUBLIC "BEET_CMAKE_PIPELINE_ASSETS_DIR=\"${BEET_CMAKE_ROOT_DIR}/\"")
target_compile_definitions(beet_gfx PUBLIC "BEET_CMAKE_RUNTIME_ASSETS_DIR=\"${CMAKE_BINARY_DIR}/dist/runtime/\"")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(beet_gfx PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(beet_gfx PUBLIC NDEBUG=1)
endif ()
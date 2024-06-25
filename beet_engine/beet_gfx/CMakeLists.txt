add_subdirectory(third)

project(beet_gfx)
add_library(beet_gfx
        inc/beet_gfx/gfx_interface.h
        inc/beet_gfx/gfx_vulkan_platform_defines.h

        src/gfx_backend_vulkan.cpp
        src/gfx_vulkan_surface_win.cpp
        inc/beet_gfx/gfx_vulkan_surface.h

        inc/beet_gfx/gfx_samplers.h
        src/gfx_samplers.cpp
        inc/beet_gfx/gfx_mesh.h
        src/gfx_vulkan_surface_linux.cpp
        inc/beet_gfx/gfx_pipeline.h
        src/gfx_pipeline.cpp
        inc/beet_gfx/gfx_descriptors.h src/gfx_descriptors.cpp
        inc/beet_gfx/gfx_indexed_indirect.h
        src/gfx_indexed_indirect.cpp
        inc/beet_gfx/gfx_shader.h
        src/gfx_shader.cpp
        inc/beet_gfx/gfx_buffer.h
        src/gfx_buffer.cpp
        inc/beet_gfx/gfx_utils.h
        src/gfx_utils.cpp
        inc/beet_gfx/gfx_command.h
        src/gfx_command.cpp
        src/gfx_mesh.cpp
        inc/beet_gfx/gfx_function_pointers.h
        src/gfx_function_pointers.cpp
        inc/beet_gfx/gfx_deprecated.h
        src/gfx_deprecated.cpp
        inc/beet_gfx/gfx_debug.h
        src/gfx_debug.cpp
        inc/beet_gfx/gfx_imgui.h
        src/gfx_imgui.cpp
        inc/beet_gfx/gfx_lit.h
        src/gfx_lit.cpp
        inc/beet_gfx/db_asset.h
        src/db_asset.cpp
        inc/beet_gfx/gfx_texture.h
        src/gfx_texture.cpp
        inc/beet_gfx/gfx_sky.h
        src/gfx_sky.cpp
        inc/beet_gfx/gfx_converter.h
        src/gfx_converter.cpp
        inc/beet_gfx/gfx_line.h
        src/gfx_line.cpp
        inc/beet_gfx/IconsFontAwesome5.h
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
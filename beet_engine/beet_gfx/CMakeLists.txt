project(beet_gfx)
add_library(beet_gfx
        inc/beet_gfx/gfx_interface.h
        inc/beet_gfx/gfx_vulkan_platform_defines.h
        
        src/gfx_backend_vulkan.cpp
        src/gfx_vulkan_surface_win.cpp
        inc/beet_gfx/gfx_vulkan_surface.h

        inc/beet_gfx/gfx_types.h
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
)

target_include_directories(beet_gfx
        PUBLIC inc
        PRIVATE src
)

find_package(Vulkan REQUIRED)
target_link_libraries(beet_gfx
        beet_shared
        beet_math
        Vulkan::Vulkan
)

set_target_properties(beet_gfx PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(beet_gfx PUBLIC BEET_DEBUG=1)
    target_compile_definitions(beet_gfx PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(beet_gfx PUBLIC NDEBUG=1)
endif ()
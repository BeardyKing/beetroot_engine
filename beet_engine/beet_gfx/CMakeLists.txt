project(beet_gfx)
add_library(beet_gfx
        inc/beet_gfx/gfx_interface.h
        inc/beet_gfx/gfx_vulkan_platform_defines.h
        
        src/gfx_backend_vulkan.cpp
        src/gfx_vulkan_surface_windows.cpp
        inc/beet_gfx/gfx_vulkan_surface.h
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

set_target_properties(beet_gfx PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(beet_gfx PUBLIC BEET_DEBUG=1)
    target_compile_definitions(beet_gfx PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(beet_gfx PUBLIC NDEBUG=1)
endif ()
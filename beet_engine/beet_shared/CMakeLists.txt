cmake_minimum_required(VERSION 3.15)

#====LIB SOURCE===========
project(beet_shared)
add_library(beet_shared
        inc/beet_shared/assert.h
        inc/beet_shared/log.h
        src/log.cpp
        inc/beet_shared/shared_utils.h
        src/shared_utils.cpp
        inc/beet_shared/memory.h
        src/memory.cpp
        inc/beet_shared/texture_formats.h
        inc/beet_shared/dds_loader.h
        src/dds_loader.cpp
        inc/beet_shared/filesystem.h
        src/filesystem_win.cpp
        inc/beet_shared/c_string.h
        src/c_string.cpp
        inc/beet_shared/platform_defines.h
        inc/beet_shared/os_time.h
        src/os_time_win.cpp
        src/os_time_linux.cpp
        inc/beet_shared/beet_types.h
)

#====LIB TARGET DIR=======
target_include_directories(beet_shared
        PUBLIC inc
        PRIVATE inc/beet_shared
        )

target_link_libraries(beet_shared
        beet_math
)

set_target_properties(beet_shared PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

#====DEBUG================
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(beet_shared PUBLIC BEET_DEBUG=1)
    target_compile_definitions(beet_shared PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(beet_shared PUBLIC NDEBUG=1)
endif ()
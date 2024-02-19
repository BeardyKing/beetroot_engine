cmake_minimum_required(VERSION 3.15)

#====LIB SOURCE===========
project(beet_core)
add_library(beet_core
        inc/beet_core/input.h
        inc/beet_core/input_types.h
        inc/beet_core/time.h
        inc/beet_core/window.h
        inc/beet_core/engine.h
        src/input.cpp
        src/time.cpp
        src/window.cpp
        src/engine.cpp
)

#====LIB TARGET DIR=======
target_include_directories(beet_core
        PUBLIC inc
        PRIVATE inc/beet_core
)

find_package(Vulkan REQUIRED)
target_link_libraries(beet_core
        Vulkan::Vulkan
        beet_math
        beet_shared
)

#====LIB INSTALL RULES====
install(TARGETS beet_core
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

set_target_properties(beet_core PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

#====DEBUG================
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(beet_core PUBLIC BEET_DEBUG=1)
    target_compile_definitions(beet_core PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(beet_core PUBLIC NDEBUG=1)
endif ()
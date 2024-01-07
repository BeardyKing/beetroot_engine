cmake_minimum_required(VERSION 3.15)

#====LIB SOURCE===========
project(beet_shared)
add_library(beet_shared
        inc/beet_shared/assert.h
        inc/beet_shared/log.h
        src/log.cpp
)

#====LIB TARGET DIR=======
target_include_directories(beet_shared
        PUBLIC inc
        PRIVATE inc/beet_shared
)

set_target_properties(beet_shared PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

#====DEBUG================
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(beet_shared PUBLIC BEET_DEBUG=1)
    target_compile_definitions(beet_shared PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(beet_shared PUBLIC NDEBUG=1)
endif ()
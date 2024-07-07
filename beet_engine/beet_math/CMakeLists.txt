cmake_minimum_required(VERSION 3.15)

#====LIB SOURCE===========
project(beet_math)
add_library(beet_math
        inc/beet_math/utilities.h
        src/utilities.cpp

        inc/beet_math/vec2.h
        inc/beet_math/vec3.h
        inc/beet_math/vec4.h
        inc/beet_math/mat4.h
        inc/beet_math/quat.h
        src/vec4.cpp
        src/mat4.cpp
)

#====LIB GLM==============
add_subdirectory(third/glm)

#====LIB DEPENDENCIES=====
target_link_libraries(beet_math
        glm
)

#====LIB TARGET DIR=======
target_include_directories(beet_math
        PUBLIC inc
        PRIVATE inc/beet_math
)

#====DEBUG================
if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(beet_math PUBLIC BEET_DEBUG=1)
    target_compile_definitions(beet_math PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(beet_math PUBLIC NDEBUG=1)
endif ()
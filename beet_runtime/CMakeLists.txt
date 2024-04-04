cmake_minimum_required(VERSION 3.15)

project(beet_runtime)

#====EXE SOURCE===========
add_executable(beet_runtime
        main.cpp
        inc/runtime/entity_builder.h
        inc/runtime/script_camera.h
        src/script_camera.cpp
        src/entity_builder.cpp
)

target_include_directories(beet_runtime
        PUBLIC inc
)

#====PROPERTIES===========
set_target_properties(beet_runtime PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
set_target_properties(beet_runtime PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist/runtime/bin")
set_target_properties(beet_runtime PROPERTIES OUTPUT_NAME beet_runtime)

target_link_libraries(beet_runtime
        beet_shared
        beet_math
        beet_gfx
        beet_core
)
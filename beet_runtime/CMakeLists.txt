cmake_minimum_required(VERSION 3.15)

project(beet_runtime)

#====EXE SOURCE===========
add_executable(beet_runtime
        main.cpp
        inc/runtime/entity_builder.h
        inc/runtime/script_camera.h
        src/script_camera.cpp
        src/entity_builder.cpp
        inc/runtime/widget_db.h
        src/widget_db.cpp
        inc/runtime/widget_manager.h
        src/widget_manager.cpp
        inc/runtime/widget_hotloader.h
        src/widget_hotloader.cpp
        inc/runtime/widget_manipulate.h
        src/widget_manipulate.cpp
)

target_include_directories(beet_runtime
        PUBLIC inc
)

#====PROPERTIES===========
set_target_properties(beet_runtime PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
set_target_properties(beet_runtime PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist/runtime")
set_target_properties(beet_runtime PROPERTIES OUTPUT_NAME beet_runtime)

target_link_libraries(beet_runtime
        beet_shared
        beet_math
        beet_gfx
        beet_core
)
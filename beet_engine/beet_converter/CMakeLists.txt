project(beet_converter)

add_library(beet_converter
        inc/beet_converter/converter_interface.h
        src/converter_shader.cpp
        src/converter_cache.cpp
        src/converter_backend.cpp
        inc/beet_converter/converter_types.h
        src/converter_texture.cpp
)

set_target_properties(beet_converter PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

target_include_directories(beet_converter
        PUBLIC inc
        PRIVATE src
)

target_link_libraries(beet_converter
        beet_shared
        beet_gfx
)
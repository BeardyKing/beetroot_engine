project(beet_converter)

add_subdirectory(third/SPIRV-Headers)
#not including <spirv-dir>/external/googletes
set(Headers_SOURCE_DIR, third/SPIRV-Headers/include/)
add_subdirectory(third/SPIRV-Tools)
add_subdirectory(third/glslang)

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
        PUBLIC third/glslang
)

target_link_libraries(beet_converter
        beet_shared
        beet_gfx
        glslang-default-resource-limits
        SPIRV
        glslang
)
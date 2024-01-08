project(beet_pipeline)

add_executable(beet_pipeline
    main.cpp
)

set(ISPC_COMPILER_LOCATION $ENV{ISPC_COMPILER_LOCATION})
set(ISPC_COMPILER_TARGET "avx2")
add_subdirectory(third/ispc_texture_compressor_cmake)

target_link_libraries(beet_pipeline
        ispc_texture_compressor
        beet_shared
)
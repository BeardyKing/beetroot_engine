project(beet_pipeline)

add_executable(beet_pipeline
    main.cpp
)

#===C_GLTF
project(c_gltf)
add_library(c_gltf INTERFACE)
target_include_directories(c_gltf INTERFACE third/cgltf/)

#===ISPC COMPRESSOR
set(ISPC_COMPILER_LOCATION $ENV{ISPC_COMPILER_LOCATION})
set(ISPC_COMPILER_TARGET "avx2")
add_subdirectory(third/ispc_texture_compressor_cmake)

target_link_libraries(beet_pipeline
        ispc_texture_compressor
        c_gltf
        beet_shared
)

##====SHADER DEPT=========//
##https://gist.github.com/evilactually/a0d191701cb48f157b05be7f74d79396
if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL AMD64)
    set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}\\Bin\\glslangValidator.exe")
else ()
    set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}\\Bin32\\glslangValidator.exe")
endif ()

## INFO: string replace "\\" with "/" for environment variables due to cmake
## not correctly parsing the $ENV path correctly on windows :^(
string(REPLACE "\\" "/" GLSL_VALIDATOR_WINDOWS_PATH_FIX ${GLSL_VALIDATOR})
target_compile_definitions(beet_pipeline PUBLIC GLSL_VALIDATOR_EXE_PATH="${GLSL_VALIDATOR_WINDOWS_PATH_FIX}")

##===PROPERTIES===========//
set_target_properties(beet_pipeline PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)
set_target_properties(beet_pipeline PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist/pipeline/bin")
set_target_properties(beet_pipeline PROPERTIES OUTPUT_NAME beet_pipeline)

##===DEBUG================//
target_compile_definitions(beet_pipeline PUBLIC BEET_PIPELINE=1)
target_compile_definitions(beet_pipeline PUBLIC "BEET_CMAKE_PIPELINE_ASSETS_DIR=\"${BEET_CMAKE_ROOT_DIR}/assets/\"")
target_compile_definitions(beet_pipeline PUBLIC "BEET_CMAKE_RUNTIME_ASSETS_DIR=\"${CMAKE_BINARY_DIR}/dist/beet_runtime/assets/\"")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(beet_pipeline PUBLIC BEET_DEBUG=1)
    target_compile_definitions(beet_pipeline PUBLIC _DEBUG=1)
else ()
    target_compile_definitions(beet_pipeline PUBLIC NDEBUG=1)
endif ()

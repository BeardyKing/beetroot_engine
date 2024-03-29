#ifndef BEETROOT_PIPELINE_DEFINES_H
#define BEETROOT_PIPELINE_DEFINES_H

//===project=================
// "BEET_CMAKE_PIPELINE_ASSETS_DIR" & "BEET_CMAKE_RUNTIME_ASSETS_DIR" defined in beet_pipeline/CMakeLists.txt
#define PIPELINE_ASSRT_DIR BEET_CMAKE_PIPELINE_ASSETS_DIR
#define CLIENT_RUNTIME_ASSET_DIR BEET_CMAKE_RUNTIME_ASSETS_DIR

//===shader compile==========
#define PIPELINE_SHADER_DIR PIPELINE_ASSRT_DIR "shaders/"
#define CLIENT_RUNTIME_SHADER_DIR CLIENT_RUNTIME_ASSET_DIR "shaders/"

//===textures================
#define PIPELINE_TEXTURE_DIR PIPELINE_ASSRT_DIR "textures/"
#define CLIENT_RUNTIME_TEXTURE_DIR CLIENT_RUNTIME_ASSET_DIR "textures/"

#endif //BEETROOT_PIPELINE_DEFINES_H

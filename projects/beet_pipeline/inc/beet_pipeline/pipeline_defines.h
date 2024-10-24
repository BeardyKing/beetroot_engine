#ifndef BEETROOT_PIPELINE_DEFINES_H
#define BEETROOT_PIPELINE_DEFINES_H

//===PROJECT============================================================================================================
// "BEET_CMAKE_PIPELINE_ASSETS_DIR" & "BEET_CMAKE_RUNTIME_ASSETS_DIR" defined in beet_pipeline/CMakeLists.txt
#define PIPELINE_ASSRT_DIR BEET_CMAKE_PIPELINE_ASSETS_DIR
#define CLIENT_RUNTIME_ASSET_DIR BEET_CMAKE_RUNTIME_ASSETS_DIR
//======================================================================================================================

//===SHADER_COMPILE=====================================================================================================
#define PIPELINE_SHADER_DIR PIPELINE_ASSRT_DIR "shaders/"
#define CLIENT_RUNTIME_SHADER_DIR CLIENT_RUNTIME_ASSET_DIR "shaders/"
//======================================================================================================================

//===TEXTURES===========================================================================================================
#define PIPELINE_TEXTURE_DIR PIPELINE_ASSRT_DIR "textures/"
#define CLIENT_RUNTIME_TEXTURE_DIR CLIENT_RUNTIME_ASSET_DIR "textures/"
//======================================================================================================================

#endif //BEETROOT_PIPELINE_DEFINES_H

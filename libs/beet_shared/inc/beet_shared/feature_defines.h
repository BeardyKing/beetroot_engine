#ifndef BEETROOT_FEATURE_DEFINES_H
#define BEETROOT_FEATURE_DEFINES_H

#define BEET_TRUE 1
#define BEET_FALSE !BEET_TRUE

#define FEATURE_ON ==
#define FEATURE_OFF !=
#define CHECK_FEATURE(featureState) (BEET_TRUE featureState BEET_TRUE)

#if defined(_WIN32)
#define FEATURE_PLATFORM_WINDOWS FEATURE_ON
#define FEATURE_PLATFORM_LINUX   FEATURE_OFF
#define FEATURE_PLATFORM_APPLE   FEATURE_OFF
#elif defined(__linux__)
#define FEATURE_PLATFORM_WINDOWS FEATURE_OFF
#define FEATURE_PLATFORM_LINUX   FEATURE_ON
#define FEATURE_PLATFORM_APPLE   FEATURE_OFF
#elif defined(__APPLE__)
#define FEATURE_PLATFORM_APPLE   FEATURE_OFF
#define FEATURE_PLATFORM_LINUX   FEATURE_OFF
#define FEATURE_PLATFORM_WINDOWS FEATURE_ON
#endif

//===DEBUG_FEATURES=====================================================================================================
#if BEET_DEBUG
#define FEATURE_GFX_IMGUI       FEATURE_ON
#define FEATURE_MEMORY_TRACKING FEATURE_ON
//======================================================================================================================
#else
//===RELEASE_FEATURES===================================================================================================
#define FEATURE_GFX_IMGUI       FEATURE_OFF
#define FEATURE_MEMORY_TRACKING FEATURE_OFF
#endif //BEET_DEBUG
//======================================================================================================================

//===IN_DEV_FEATURES====================================================================================================
#define FEATURE_IN_DEV_RUNTIME_GLTF_LOADING FEATURE_ON
//======================================================================================================================

#endif //BEETROOT_FEATURE_DEFINES_H
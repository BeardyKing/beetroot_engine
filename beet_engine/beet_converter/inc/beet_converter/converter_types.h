#ifndef BEETROOT_CONVERTER_TYPES_H
#define BEETROOT_CONVERTER_TYPES_H

#include <string>

//===PUBLIC_STRUCTS=====================================================================================================
struct ConverterLocations {
    std::string rawAssetDir = {};
    std::string targetAssetDir = {};

    std::string vulkanSDKDir = {};
    std::string compressonatorSDKDir = {};

    std::string compressonatorCLI = {};
    std::string glslValidatorCLI = {};
};

struct ConverterOptions {
    bool ignoreConvertCache = {false};
};
//======================================================================================================================

#endif //BEETROOT_CONVERTER_TYPES_H

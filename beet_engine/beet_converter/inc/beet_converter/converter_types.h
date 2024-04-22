#ifndef BEETROOT_CONVERTER_TYPES_H
#define BEETROOT_CONVERTER_TYPES_H

#include <string>

//===PUBLIC_STRUCTS=====================================================================================================
struct ConverterFileLocations {
    std::string rawAssetDir = {};
    std::string targetAssetDir = {};
};

struct ConverterOptions {
    bool ignoreConvertCache = {false};
};
//======================================================================================================================

#endif //BEETROOT_CONVERTER_TYPES_H

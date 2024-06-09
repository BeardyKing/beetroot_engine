#include <beet_shared/base_64.h>
#include <beet_shared/assert.h>

#include <cstring>

//===INTERNAL_STRUCTS===================================================================================================
constexpr int32_t base64LUT[] = {
        62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58,
        59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5,
        6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
        21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28,
        29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
        43, 44, 45, 46, 47, 48, 49, 50, 51
};
//======================================================================================================================

//===INTERNAL_FUNCTIONS=================================================================================================
static bool base64_is_valid_data(const char *inData, size_t inDataLength) {
    for (size_t i = 0; i < inDataLength; ++i) {
        const char c = inData[i];
        if (!((c >= '0' && c <= '9') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c == '+' || c == '/' || c == '='))) {
            return false;
        }
    }
    return true;
}
//======================================================================================================================

//===API================================================================================================================
size_t base64_decode_size(const char *inData, const size_t &inDataSize) {
    ASSERT(inData != nullptr && inDataSize != 0)
    size_t outDecodeSize = inDataSize / 4 * 3;

    for (size_t i = inDataSize; i > 0; --i) {
        if (inData[i] == '=') {
            outDecodeSize--;
        } else {
            break;
        }
    }
    return outDecodeSize;
}

void base64_decode(const char *inData, uint8_t *outData) {
    ASSERT(inData != nullptr && outData != nullptr)

    const size_t inDataSize = strlen(inData);
    ASSERT(inDataSize != 0)
    ASSERT(base64_is_valid_data(inData, inDataSize))

    for (size_t i = 0, j = 0; i < inDataSize; i += 4, j += 3) {
        int32_t v = base64LUT[inData[i] - 43];
        v = (v << 6) | base64LUT[inData[i + 1] - 43];
        v = inData[i + 2] == '=' ? v << 6 : (v << 6) | base64LUT[inData[i + 2] - 43];
        v = inData[i + 3] == '=' ? v << 6 : (v << 6) | base64LUT[inData[i + 3] - 43];

        outData[j] = (v >> 16) & 0xFF;
        if (inData[i + 2] != '=')
            outData[j + 1] = (v >> 8) & 0xFF;
        if (inData[i + 3] != '=')
            outData[j + 2] = v & 0xFF;
    }
}
//======================================================================================================================
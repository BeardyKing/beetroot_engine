#ifndef BEETROOT_BASE_64_H
#define BEETROOT_BASE_64_H

#include <cstdint>

//===API================================================================================================================
size_t base64_decode_size(const char *inData, const size_t &inDataSize);
void base64_decode(const char *inData, uint8_t *outData);
//======================================================================================================================

#endif //BEETROOT_BASE_64_H

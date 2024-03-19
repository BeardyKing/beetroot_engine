#ifndef BEETROOT_C_STRING_H
#define BEETROOT_C_STRING_H

#include <cstdint>

bool c_str_empty(const char *inStr);
bool c_str_equal(const char *lhs, const char *rhs);
bool c_str_n_equal(const char *lhs, const char *rhs, size_t count);

const char *c_str_search_reverse(const char *src, const char *subStr);
const char *c_str_n_search_reverse(const char *src, int32_t srcLen, const char *subStr, int32_t subStrLen);

#endif //BEETROOT_C_STRING_H

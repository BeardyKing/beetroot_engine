#include <beet_shared/c_string.h>
#include <cstring>

//===API================================================================================================================
bool c_str_empty(const char *inStr) {
    return (inStr == nullptr) || (inStr[0] == '\0');
}

bool c_str_equal(const char *lhs, const char *rhs) {
    return (strcmp(lhs, rhs) == 0);
}

bool c_str_n_equal(const char *lhs, const char *rhs, const size_t count) {
    return (strncmp(lhs, rhs, count) == 0);
}

const char *c_str_n_search_reverse(const char *src, const int32_t srcLen, const char *subStr, const int32_t subStrLen) {
    const int32_t itrStart = srcLen - subStrLen;

    for (int32_t i = itrStart; i >= 0; i--) {
        if (c_str_n_equal(&src[i], subStr, subStrLen)) {
            return &src[i];
        }
    }
    return nullptr;
}

const char *c_str_search_reverse(const char *src, const char *subStr) {
    const int32_t srcLen = (int32_t) strlen(src);
    const int32_t subStrLen = (int32_t) strlen(subStr);
    const int32_t itrStart = srcLen - subStrLen;

    return c_str_n_search_reverse(src, srcLen, subStr, subStrLen);
}
//======================================================================================================================


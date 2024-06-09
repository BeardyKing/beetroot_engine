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

    return c_str_n_search_reverse(src, srcLen, subStr, subStrLen);
}

char *c_str_n_search_reverse(char *src, const int32_t srcLen, const char *subStr, const int32_t subStrLen) {
    const int32_t itrStart = srcLen - subStrLen;

    for (int32_t i = itrStart; i >= 0; i--) {
        if (c_str_n_equal(&src[i], subStr, subStrLen)) {
            return &src[i];
        }
    }
    return nullptr;
}

char *c_str_search_reverse(char *src, const char *subStr) {
    const int32_t srcLen = (int32_t) strlen(src);
    const int32_t subStrLen = (int32_t) strlen(subStr);

    return c_str_n_search_reverse(src, srcLen, subStr, subStrLen);
}

bool c_str_replace_after_delim_reverse(char *existingPath, const char *replaceTarget, const char *subStr) {
    if (char *target = c_str_search_reverse(existingPath, subStr)) {
        memset((target + 1), '\0', strlen(target + 1));
        strcpy(target + 1, replaceTarget);
        return true;
    }
    return false;
}
//======================================================================================================================


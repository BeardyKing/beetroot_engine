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

bool c_str_replace_extension(char *existingPath, const char *newExtension) {
    const char *extension = (*newExtension == '.') ? newExtension + 1 : newExtension;
    if (char *dot = c_str_search_reverse(existingPath, ".")) {
        memset(dot + 1, '\0', strlen(dot + 1));
        strcpy(dot + 1, extension);
        return true;
    }
    return false;
}

bool c_str_remove_file_from_path(const char *inPath, char *outPath) {
    if (c_str_empty(inPath) || outPath == nullptr) {
        return false;
    }

    strcpy(outPath, inPath);

    char *lastSlash = c_str_search_reverse(outPath, "/");
    if (lastSlash == nullptr) {
        lastSlash = c_str_search_reverse(outPath, "\\");
    }

    if (lastSlash) {
        *(lastSlash + 1) = '\0';
    } else {
        *outPath = '\0';
    }

    return true;
}

bool c_str_extract_file_name(const char *inPath, char *outFilename) {
    if (c_str_empty(inPath) || outFilename == nullptr) {
        return false;
    }

    const char *lastSlash = c_str_search_reverse(inPath, "/");
    if (lastSlash == nullptr) {
        lastSlash = c_str_search_reverse(inPath, "\\");
        if (lastSlash == nullptr) {
            return false;
        }
    }

    strcpy(outFilename, lastSlash + 1);

    return true;
}
//======================================================================================================================


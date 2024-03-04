#include <beet_shared/c_string.h>
#include <cstring>

bool c_str_empty(const char *inStr) {
    return (inStr == nullptr) || (inStr[0] == '\0');
}

bool c_str_equal(const char *lhs, const char *rhs) {
    return (strcmp(lhs, rhs) == 0);
}
#include <beet_shared/string_utils.h>
#include <cstring>

bool string_equal(const char* lhs, const char* rhs){
    return strcmp(lhs, rhs) == 0;
}
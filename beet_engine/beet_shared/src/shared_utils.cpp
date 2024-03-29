#include <beet_shared/shared_utils.h>

uint32_t count_set_bits(const uint32_t n) {
    uint32_t target = n;
    uint32_t count = 0;
    while (target) {
        count++;
        target >>= 1;
    }
    return count;
}
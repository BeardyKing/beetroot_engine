#include <beet_math/utilities.h>

#define PI 3.14159265358979323846264338327950288f
#define PI_FLOAT 3.1415927f
#define PI_DOUBLE 3.141592653589793f

float as_degrees(float radians) {
    return radians * (180.0f / PI_FLOAT);
}

double as_degrees(double radians) {
    return radians * (180.0 / PI_DOUBLE);
}

float as_radians(float degrees) {
    return degrees * (PI_DOUBLE / 180.0f);
}

double as_radians(double degrees) {
    return degrees * (PI_DOUBLE / 180.0);
}
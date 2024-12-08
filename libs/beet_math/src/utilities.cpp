#include <beet_math/utilities.h>
#include <beet_math/vec3.h>
#include <beet_math/vec4.h>

//===INTERNAL_STRUCTS===================================================================================================
#define PI 3.14159265358979323846264338327950288f
#define PI_FLOAT 3.1415927f
#define PI_DOUBLE 3.141592653589793f
//======================================================================================================================

//===API================================================================================================================
float as_degrees_f(float radians) {
    return radians * (180.0f / PI_FLOAT);
}

float as_radians_f(float degrees) {
    return degrees * (PI_DOUBLE / 180.0f);
}

double as_degrees_d(double radians) {
    return radians * (180.0 / PI_DOUBLE);
}

double as_radians_d(double degrees) {
    return degrees * (PI_DOUBLE / 180.0);
}

uint32_t kelvin_to_rgba(uint32_t kelvin) {
    // https://tannerhelland.com/2012/09/18/convert-temperature-rgb-algorithm-code.html
    const float KtoRGB_Temperature = kelvin / 100;
    vec3f outColour;
    //RED
    if (KtoRGB_Temperature <= 66) {
        outColour.r = 255;
    } else {
        outColour.r = 329.698727446 * pow(KtoRGB_Temperature - 60, -0.1332047592);
    }

    if (outColour.r < 0) {
        outColour.r = 0;
    }

    if (outColour.r > 255) {
        outColour.r = 255;
    }

    //GREEN
    if (KtoRGB_Temperature <= 66) {
        outColour.g = 99.4708025861 * log(KtoRGB_Temperature) - 161.1195681661;
    } else {
        outColour.g = 288.1221695283 * pow(KtoRGB_Temperature - 60, -0.0755148492);
    }

    if (outColour.g < 0) {
        outColour.g = 0;
    }
    if (outColour.g > 255) {
        outColour.g = 255;
    }

    //BLUE
    if (KtoRGB_Temperature >= 66) {
        outColour.b = 255;
    } else {
        if (KtoRGB_Temperature <= 19) {
            outColour.b = 0;
        } else {
            outColour.b = 138.5177312231 * log(KtoRGB_Temperature - 10) - 305.0447927307;
        }

    }

    if (outColour.b < 0) {
        outColour.b = 0;
    }
    if (outColour.b > 255) {
        outColour.b = 255;
    }

    return pack_vec4f_to_uint32_t(vec4f(outColour.r / 255.0f, outColour.g / 255.0f, outColour.b / 255.0f, 1.0f));
}
//======================================================================================================================

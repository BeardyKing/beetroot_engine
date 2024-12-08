#ifndef BEETROOT_UTILITIES_H
#define BEETROOT_UTILITIES_H

#include <cstdint>

//===API================================================================================================================
float as_degrees_f(float radians);
float as_radians_f(float degrees);

double as_degrees_d(double radians);
double as_radians_d(double degrees);

uint32_t kelvin_to_rgba(uint32_t kelvin);
//======================================================================================================================

#endif //BEETROOT_UTILITIES_H

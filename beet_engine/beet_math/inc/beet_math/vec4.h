#ifndef BEETROOT_VEC4_H
#define BEETROOT_VEC4_H

#include <cstdint>
#include <glm/vec4.hpp>

//===PUBLIC_STRUCTS=====================================================================================================
using namespace glm;
using vec4i = vec<4, int32_t>;
using vec4u = vec<4, uint32_t>;
using vec4f = vec<4, float>;
using vec4d = vec<4, double>;
//======================================================================================================================

//===API================================================================================================================
uint32_t pack_vec4f_to_uint32_t(vec4f &vec);
vec4f unpack_uint32_t_to_vec4f(uint32_t);
//======================================================================================================================

#endif //BEETROOT_VEC4_H

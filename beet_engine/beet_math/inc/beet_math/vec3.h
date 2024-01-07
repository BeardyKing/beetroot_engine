#ifndef BEETROOT_VEC3_H
#define BEETROOT_VEC3_H

#include <cstdint>
#include <glm/vec3.hpp>

using namespace glm;

using vec3i = vec<3, int32_t>;
using vec3u = vec<3, uint32_t>;
using vec3f = vec<3, float>;
using vec3d = vec<3, double>;

#define WORLD_UP        vec3f{0, 1, 0}
#define WORLD_FORWARD   vec3f{0, 0,-1}
#define WORLD_RIGHT     vec3f{-1, 0, 0}

#endif //BEETROOT_VEC3_H

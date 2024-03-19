#ifndef BEETROOT_DB_TYPES_H
#define BEETROOT_DB_TYPES_H

#include <beet_math/vec3.h>

struct Transform {
    vec3f position{0.0f, 0.0f, 0.0f};
    vec3f rotation{0.0f, 0.0f, 0.0f};
    vec3f scale{1.0f, 1.0f, 1.0f};
};

struct Camera {
    float fov{60.0f};
    float zNear{0.1f};
    float zFar{800.0f};
};

#endif //BEETROOT_DB_TYPES_H

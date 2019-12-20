#pragma once

#include <glm/vec3.hpp>
#include <math.h>

class ONB {
    ONB() =delete;

public:
    glm::vec3 U, V, N;

    // from Pixar (shrug)
    ONB(const glm::vec3 &_N) : N(_N){
        float sign = copysignf(1.0f, N.z);
        const float a = -1.0f / (sign + N.z);
        const float b = N.x * N.y * a;
        U = glm::vec3(1.0f + sign * N.x * N.x * a, sign * b, -sign * N.x);
        V = glm::vec3(b, sign + N.y * N.y * a, -N.y);
    }

    glm::vec3 world2local(const glm::vec3& v) const;
    
    glm::vec3 local2world(const glm::vec3& v) const;


};
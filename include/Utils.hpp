#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace utils {
    float powerHeuristic(int nf, float fPDF, int ng, float gPDF);
    void uniformSampleTriangle(const glm::vec2& uv, glm::vec2& bg);
    float rand01() {return (float) rand() / (float) RAND_MAX;}
    glm::vec2 rand01vec2() {return glm::vec2(rand01(), rand01());}

    float avg(const glm::vec3& v) {return (v[0] + v[1] + v[2]) * (float) (1/3);}
    Color lerp(const Color& a, const Color& b, const float t) {return (1.0f - t) * a + t * b;}
    inline glm::vec3 cosineSampleHemisphere(const glm::vec2& uv);
    inline glm::vec2 concentricSampleDisk(const glm::vec2& uv);
}
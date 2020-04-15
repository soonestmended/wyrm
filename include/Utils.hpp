#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace utils {
    float powerHeuristic(int nf, float fPDF, int ng, float gPDF);
    void uniformSampleTriangle(const glm::vec2& uv, glm::vec2& bg);

    //glm::vec3 transformNormal(const glm::mat4 &xform, const glm::vec3 N);
}
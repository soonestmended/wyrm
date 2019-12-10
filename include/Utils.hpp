#pragma once

#include <glm/vec2.hpp>

namespace utils {
    float powerHeuristic(int nf, float fPDF, int ng, float gPDF);
    void uniformSampleTriangle(const glm::vec2& uv, glm::vec2& bg);
}
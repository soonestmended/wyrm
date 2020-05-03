#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include <sstream>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "Color.hpp"

namespace utils {
    float powerHeuristic(int nf, float fPDF, int ng, float gPDF);
    void uniformSampleTriangle(const glm::vec2& uv, glm::vec2& bg);
    float rand01();
    glm::vec2 rand01vec2();
    float avg(const glm::vec3& v);
    Color lerp(const Color& a, const Color& b, const float t);
    std::string v2s(const glm::vec3& v);
    Color clamp(const Color& c, const float min, const float max);
    float clamp(const float f, const float min, const float max);
    glm::vec3 sameSide(const glm::vec3& testVec, const glm::vec3& v);
}
#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <string>
#include <sstream>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "common.hpp"
#include "Color.hpp"

namespace utils {
    Real powerHeuristic(int nf, Real fPDF, int ng, Real gPDF);
    void uniformSampleTriangle(const Vec2& uv, Vec2& bg);
    Real avg(const Vec3& v);
    Color lerp(const Color& a, const Color& b, const Real t);
    std::string v2s(const Vec3& v);
    Color clamp(const Color& c, const Real min, const Real max);
    Real clamp(const Real f, const Real min, const Real max);
    Vec3 sameSide(const Vec3& testVec, const Vec3& v);
  void winsorize(std::vector <Color>& v, Real amt);
  std::string format_duration( std::chrono::milliseconds ms );
}

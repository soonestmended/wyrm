#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <string>
#include <variant>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#define POS_INF 9999999999.0
//#define DEBUG
#define HIGH_PRECISION

#ifdef HIGH_PRECISION
#define EPSILON .0000001
typedef double Real;
typedef glm::dvec4 Vec4;
typedef glm::dvec3 Vec3;
typedef glm::dvec2 Vec2;
typedef glm::dmat4 Mat4;
#else
#define EPSILON .01
typedef float Real;
typedef glm::vec4 Vec4;
typedef glm::vec3 Vec3;
typedef glm::vec2 Vec2;
typedef glm::mat4 Mat4;

#endif
inline std::ostream& operator<<(std::ostream& os, const Mat4& c) {
  os << "<" << c[0][0] << ", " << c[0][1] << ", " << c[0][2] << ", " << c[0][3] << ">" << std::endl;
  os << "<" << c[1][0] << ", " << c[1][1] << ", " << c[1][2] << ", " << c[1][3] << ">" << std::endl;
  os << "<" << c[2][0] << ", " << c[2][1] << ", " << c[2][2] << ", " << c[2][3] << ">" << std::endl;
  os << "<" << c[3][0] << ", " << c[3][1] << ", " << c[3][2] << ", " << c[3][3] << ">" << std::endl;

  return os;
}


inline std::ostream& operator<<(std::ostream& os, const Vec4& c) {
  os << "<" << c.x << ", " << c.y << ", " << c.z << ", " << c.w << ">";
    return os;
}


inline std::ostream& operator<<(std::ostream& os, const Vec3& c) {
    os << "<" << c.x << ", " << c.y << ", " << c.z << ">";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Vec2& c) {
    os << "<" << c.x << ", " << c.y << ">";
    return os;
}




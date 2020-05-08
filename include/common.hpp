#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>


#define POS_INF 9999999999.0
//#define DEBUG
#define HIGH_PRECISION

#ifdef HIGH_PRECISION
#define EPSILON .0001
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
inline std::ostream& operator<<(std::ostream& os, const glm::vec3& c) {
    os << "<" << c.x << ", " << c.y << ", " << c.z << ">";
    return os;
}

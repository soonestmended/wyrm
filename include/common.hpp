#pragma once

#include <iostream>
#include <glm/vec3.hpp>

#define EPSILON 0.002
#define POS_INF 9999999999.0
//#define DEBUG

inline std::ostream& operator<<(std::ostream& os, const glm::vec3& c) {
    os << "<" << c.x << ", " << c.y << ", " << c.z << ">";
    return os;
}
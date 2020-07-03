#pragma once

#include <glm/vec3.hpp>

//#include "Accelerator.hpp"
class Accelerator;

class VisibilityTester {
public:
    Vec3 p, q;
    VisibilityTester() {p = q = Vec3(0.0);}
    VisibilityTester(const Vec3& _p, const Vec3& _q) : p(_p), q(_q) {}
    bool testVisible(const Accelerator& a) const;
};

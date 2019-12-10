#pragma once

#include <glm/vec3.hpp>

//#include "Accelerator.hpp"
class Accelerator;

class VisibilityTester {
public:
    glm::vec3 p, q;
    VisibilityTester() {p = q = glm::vec3(0.0);}
    VisibilityTester(const glm::vec3& _p, const glm::vec3& _q) : p(_p), q(_q) {}
    bool testVisibile(const Accelerator& a) const;
};
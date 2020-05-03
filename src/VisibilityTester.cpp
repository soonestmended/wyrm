#include "common.hpp"

#include "Accelerator.hpp"
#include "VisibilityTester.hpp"
#include "Ray.hpp"

bool VisibilityTester::testVisibile(const Accelerator& a) const {
    Ray r(p, q - p);
    r.normalize();
    return !a.intersectionYN(r, .001, glm::length(q - p) - .0001);
}
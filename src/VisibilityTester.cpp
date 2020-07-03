#include "common.hpp"

#include "Accelerator.hpp"
#include "VisibilityTester.hpp"
#include "Ray.hpp"

bool VisibilityTester::testVisible(const Accelerator& a) const {
    Ray r(p, q - p);
    r.normalize();
    return !a.intersectionYN(r, EPSILON, glm::distance(q, p) - .1);
}

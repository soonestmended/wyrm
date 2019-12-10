#include "IntersectRec.hpp"
#include "Light.hpp"

#include <glm/gtx/norm.hpp> 

#include <iostream>

using namespace std;

Color PointLight::sample(const glm::vec2& uv, const IntersectRec& ir, glm::vec3& wi, float& pdf, VisibilityTester& vt) const {
    vt = VisibilityTester(ir.isectPoint, this->P);
    wi = this->P - ir.isectPoint;
    float distSquared = glm::length2(wi);
    float dist = sqrtf(distSquared);
    wi /= dist;
    pdf = 1.f;
    return this->color * this->power / distSquared;
}

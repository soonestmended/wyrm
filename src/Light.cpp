#include "IntersectRec.hpp"
#include "Light.hpp"
#include "Primitive.hpp"

#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp> 

#include <iostream>

using namespace std;

const Color PointLight::sample(const glm::vec2& uv, const IntersectRec& ir, glm::vec3& wi, float& pdf, VisibilityTester& vt) const {
    vt = VisibilityTester(ir.isectPoint, this->P);
    wi = this->P - ir.isectPoint;
    float distSquared = glm::length2(wi);
    float dist = sqrtf(distSquared);
    wi /= dist;
    pdf = 1.f;
    return this->color * this->power / distSquared;
}

const Color GeometricLight::sample(const glm::vec2& uv, const IntersectRec& ir, glm::vec3& wi_world, float* pdf, VisibilityTester& vt) const {
    glm::vec3 pointOnLight, directionFromLight;
    prim->getRandomPointAndDirection(glm::vec2(rand(), rand()), pointOnLight, directionFromLight, pdf);
    vt = VisibilityTester(ir.isectPoint, pointOnLight);
    wi_world = glm::normalize(pointOnLight - ir.isectPoint);
    //float distSquared = glm::length2(wi);
    //float dist = sqrtf(distSquared);
    //wi /= dist;
    // pdf assumes that shape is sampled uniformly with respect to surface area.
    // pdf returned is with respect to solid angle. 
    //pdf = distSquared / (prim->getSurfaceArea() * abs(glm::dot(directionFromLight, -wi)));
    return this->getColor() * this->getPower();
}
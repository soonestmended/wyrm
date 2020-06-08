#include "common.hpp"
#include "IntersectRec.hpp"
#include "Light.hpp"
#include "Primitive.hpp"

#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp> 

#include <iostream>

using namespace std;

const Color PointLight::sample(const Vec2& uv, const IntersectRec& ir, Vec3& wi, Real* pdf, VisibilityTester& vt) const {
    vt = VisibilityTester(ir.isectPoint, this->P);
    wi = this->P - ir.isectPoint;
    Real distSquared = glm::length2(wi);
    Real dist = sqrtf(distSquared);
    wi /= dist;
    *pdf = 1;
    return this->color / distSquared;
}

const Color DirectionalLight::sample(const Vec2& uv, const IntersectRec& ir, Vec3& wi, Real* pdf, VisibilityTester& vt) const {
  vt = VisibilityTester(ir.isectPoint, ir.isectPoint + 999999. * (-dir));
  wi = -dir;
  *pdf = 1;
  return this->color;
}


const Color GeometricLight::sample(const Vec2& uv, const IntersectRec& ir, Vec3& wi_world, Real* pdf, VisibilityTester& vt) const {
    Vec3 pointOnLight, directionFromLight;
    prim->getRandomPointAndDirection(uv, pointOnLight, directionFromLight, pdf);
    //cout << (Color) pointOnLight << " pdf: " << *pdf << endl;

    vt = VisibilityTester(ir.isectPoint, pointOnLight);
    wi_world = glm::normalize(pointOnLight - ir.isectPoint);
    //Real distSquared = glm::length2(wi);
    //Real dist = sqrtf(distSquared);
    //wi /= dist;
    // pdf assumes that shape is sampled uniformly with respect to surface area.
    // pdf returned is with respect to solid angle. 
    //pdf = distSquared / (prim->getSurfaceArea() * abs(glm::dot(directionFromLight, -wi)));
    IntersectRec lightIR;
    Ray r{ir.isectPoint, wi_world};
    if (!prim->intersect(r, 0, POS_INF, lightIR)) return Color::Black();
    //prim->intersect(r, .001, POS_INF, lightIR);
    //cout << "Random light point: " << pointOnLight << endl;
    Real dist2 = glm::length2(pointOnLight - ir.isectPoint);
    *pdf = (dist2 / abs(glm::dot(lightIR.normal, -r.d))) * (*pdf);
    //cout << "pdf: " << *pdf << endl;
    return this->getColor();
}

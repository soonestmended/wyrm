#include "common.hpp"
#include "IntersectRec.hpp"
#include "Light.hpp"
#include "Primitive.hpp"

#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp> 

#include <iostream>

using namespace std;

InfiniteLight::InfiniteLight(const std::shared_ptr <ImageTexture> _texPtr, const Mat4& _worldToLight = Mat4{1}, const Color& _scaleColor = Color{1}) :
    texPtr (_texPtr), worldToLight (_worldToLight), scaleColor (_scaleColor), lumImage (texPtr->image.width(), texPtr->image.height()), d2d (lumImage) {
    // first scale texture image by scaleColor
    for (int row = 0; row < texPtr->image.height(); ++row) {
        for (int col = 0; col < texPtr ->image.width(); ++col) {
            texPtr->image(col, row) *= scaleColor;
            lumImage(col, row) = texPtr->image(col, row).luminance();
        }
    }

}

const Color InfiniteLight::sample(const Vec2& uv, const IntersectRec& ir, Vec3& wi, Real* pdf, VisibilityTester& vt) const {
  // (importance) sample point on ImageTexture
  Vec2 phiTheta = Vec2{2*M_PI, M_PI} * d2d.sampleContinuous(uv, pdf);

  // transform pdf
  *pdf /= 2 * M_PI * M_PI * std::sin(phiTheta[1]); // convert to solid angle using Jacobian

  // create Vec3 to that point
  Vec3 dir = worldToLight.inverseTransformVector(ONB::sphericalToCartesian(phiTheta));
  vt = VisibilityTester(ir.isectPoint, ir.isectPoint + 999999. * (dir));
  wi = dir;

  return this->texPtr->eval(phiTheta[0], phiTheta[1]);
  
}

Real InfiniteLight::pdf(const IntersectRec& ir, const Vec3& wi_local) const {
  // intersect wi_local with ImageTexture and return pdf of having chosen that point?

}

const Color InfiniteLight::LInf(const Ray& r) const {
  // convert ray to spherical coordinates and return value from ImageTexture

  // transform ray direction by infinite light transformation matrix
  Vec3 dir = glm::normalize(worldToLight.transformVector(r.d));

  // convert to spherical coordinates
  Vec2 phiTheta = ONB::cartesianToSpherical(dir);

  // sample texture
  return texPtr->eval(phiTheta[0], phiTheta[1]);
}

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

#include "common.hpp"
#include "IntersectRec.hpp"
#include "Light.hpp"
#include "Primitive.hpp"

#include <glm/vec3.hpp>
#include <glm/gtx/norm.hpp> 

#include <iostream>

using namespace std;

// For texture coordinates: (0, 0) is at upper left corner of image.
// Same for theta (0 at top, 1 at bottom)

Vec2 sphericalToTexcoords(Vec2& phiTheta) {
  return Vec2{phiTheta[0] / (2. * M_PI), phiTheta[1] / M_PI};
}

Vec2 texcoordsToSpherical(Vec2& tc) {
  return Vec2{tc[0] * 2 * M_PI, tc[1] * M_PI};
}

InfiniteLight::InfiniteLight(const std::shared_ptr <ImageTexture> _texPtr, const Mat4& _lightToWorld = Mat4{1}, const Color& _scaleColor = Color{1}) :
    texPtr (_texPtr), lightToWorld (_lightToWorld), scaleColor (_scaleColor), lumImage (texPtr->image.width(), texPtr->image.height()) {
  texPtr->image.boxBlur(3, 1);
  // first scale texture image by scaleColor
    for (int row = 0; row < texPtr->image.height(); ++row) {
        for (int col = 0; col < texPtr ->image.width(); ++col) {
            texPtr->image(col, row) *= scaleColor;
            lumImage(col, row) = texPtr->image(col, row).luminance();
        }
    }

    // blur lum image
    //lumImage.blur(3, 1.0);
    for (int row = 0; row < texPtr->image.height(); ++row)
        for (int col = 0; col < texPtr ->image.width(); ++col)
            lumImage(col, row) *= sin(M_PI * (((Real) row + .5) / (Real) lumImage.height()));
    this->d2d = Distribution2D(lumImage);
}

const Color InfiniteLight::sample(const Vec2& uv, const IntersectRec& ir, Vec3& wi_world, Real* pdf, VisibilityTester& vt) const {
  // (importance) sample point on ImageTexture
  Vec2 tc = d2d.sampleContinuous(uv, pdf);
  //cout << "Sampled tc: " << tc << endl;
  Vec2 phiTheta = texcoordsToSpherical(tc); // {tc[0] * 2 * M_PI, (1-tc[1]) * M_PI};
  // transform pdf
  Real sinTheta = sin(phiTheta[1]);
  if (sinTheta == 0) *pdf = 0;
  else *pdf /= 2 * M_PI * M_PI * sinTheta; // convert to solid angle using Jacobian
  // create Vec3 to that point
  Vec3 dir = glm::normalize(lightToWorld.transformVector(ONB::sphericalToCartesian(phiTheta)));
  //cout << "Dir pre-transform: " << dir << endl;
  // dir = glm::normalize(lightToWorld.transformVector(dir));
  //cout << "Dir: " << dir << endl;
 //Vec3 dir = Vec3{sinTheta*cosPhi, sinTheta*sinPhi, cosTheta};
 //dir.z = -dir.z;
  vt = VisibilityTester(ir.isectPoint, ir.isectPoint + 9999. * (dir));
  wi_world = dir;
  Color ans = this->texPtr->eval(Vec2{tc[0], tc[1]});
  int ix = utils::clamp(tc[0] * this->texPtr->image.width(), 0, this->texPtr->image.width() - 1);
  int iy = utils::clamp(tc[1] * this->texPtr->image.height(), 0, this->texPtr->image.height() - 1);
  //cout << "Sampled color at (" << ix << ", " << iy << "): " << ans;
  //cout << "\tpdf: " << *pdf << endl;

  return ans;
}

Real InfiniteLight::pdf(const IntersectRec& ir, const Vec3& wi_local) const {
  // intersect wi_local with ImageTexture and return pdf of having chosen that point?
  Vec3 dir = glm::normalize(lightToWorld.inverseTransformVector(ir.onb.local2world(wi_local)));
  //Vec3 dir = glm::normalize(ir.onb.local2world(wi_local));
  Vec2 phiTheta = ONB::cartesianToSpherical(dir);
  Real sinTheta = sin(phiTheta[1]);
  if (sinTheta == 0) return 0;
  return d2d.continuousPdf(sphericalToTexcoords(phiTheta)) / (2 * M_PI * M_PI * sinTheta);

}

const Color InfiniteLight::LInf(const Ray& r) const {
  // convert ray to spherical coordinates and return value from ImageTexture

  // transform ray direction by infinite light transformation matrix
  Vec3 dir = glm::normalize(lightToWorld.inverseTransformVector(r.d));

  // convert to spherical coordinates
  Vec2 phiTheta = ONB::cartesianToSpherical(dir);

  // sample texture
  Vec2 tc = sphericalToTexcoords(phiTheta);
  return texPtr->eval(tc);
}

const Color PointLight::sample(const Vec2& uv, const IntersectRec& ir, Vec3& wi_world, Real* pdf, VisibilityTester& vt) const {
    vt = VisibilityTester(ir.isectPoint, this->P);
    wi_world = this->P - ir.isectPoint;
    Real distSquared = glm::length2(wi_world);
    Real dist = sqrtf(distSquared);
    wi_world /= dist;
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

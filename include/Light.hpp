#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <memory>

#include "common.hpp"
#include "Color.hpp"
#include "Primitive.hpp"
#include "VisibilityTester.hpp"

class IntersectRec;

class Light {
public:
    virtual const Color getColor() const = 0;
    virtual const Real getPower() const = 0;
    virtual const bool isDeltaLight() const {return false;}
    //virtual void getRandomPoint(const Vec2& uv, Vec3& p) const = 0;
    //virtual void getRandomPointAndDirection(const Vec2& uv, Vec3& p, Vec3& d) const = 0;

    virtual const Color sample(const Vec2& uv, const IntersectRec& ir, Vec3& wi, Real* pdf, VisibilityTester& vt) const = 0;
    virtual const Real pdf(const IntersectRec& ir, const Vec3& wi_local) const {return 0.0f;}
    virtual const Primitive *getPrim() const {return nullptr;}
    virtual const Color LInf(const Ray& r) const {return Color::Black();} // this is for directional lights to contribute to rays leaving scene
};

class PointLight : public Light {
public:
    PointLight(const Vec3& p, const Color& c, const Real power_) : P (p), color (c), power (power_) {};

    const Color getColor() const {return color;}
    const Real getPower() const {return power;}
    const bool isDeltaLight() const {return true;}

    //void getRandomPoint(const Vec2& uv, Vec3& p) const {p = P;}
    //void getRandomPointAndDirection(const Vec2& uv, Vec3& p, Vec3& d) const {
    //    p = P;
    //};
    const Color sample(const Vec2& uv, const IntersectRec& ir, Vec3& wi, Real* pdf, VisibilityTester& vt) const;

private:
    Vec3 P;
    Color color;
    Real power;
};

class DirectionalLight : public Light {
public:
  DirectionalLight(const Vec3 _dir, const Color _color) : dir (_dir), color (_color) {}
  const Color getColor() const {return color;}
  const Real getPower() const {return 1;} // not yet implemented
  const bool isDeltaLight() const {return true;} // no chance of choosing the direction to the light

  const Color sample(const Vec2& uv, const IntersectRec& ir, Vec3& wi, Real* pdf, VisibilityTester& vt) const;
  //const Real pdf(const IntersectRec& ir, const Vec3& wi_local) const;
  const Color LInf(const Ray& r) const {return Color::Black();} // this is for infinite lights to contribute to rays leaving scene
  
  Vec3 dir;
  Color color;
};
class SpotLight;
class GeometricLight : public Light {
public: 
    GeometricLight(const std::shared_ptr <Primitive> _prim) : prim (_prim) {}

    const Color getColor() const {return prim->getMaterial()->getEmission();}
    const Real getPower() const {return prim->getSurfaceArea();}

    const Color sample(const Vec2& uv, const IntersectRec& ir, Vec3& wi, Real* pdf, VisibilityTester& vt) const;
    const Real pdf(const IntersectRec& ir, const Vec3& wi_local) const {
        if (!prim->intersectYN(Ray(ir.isectPoint, ir.onb.local2world(wi_local)), EPSILON, POS_INF)) {
            return 0;
        }
        return prim->getRandomPointPdf(ir, wi_local);
    }

    const Primitive *getPrim() const {return prim.get();}

private:
    const std::shared_ptr <Primitive> prim;
};

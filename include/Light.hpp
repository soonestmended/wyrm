#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <memory>

#include "Color.hpp"
#include "Primitive.hpp"
#include "VisibilityTester.hpp"

class IntersectRec;

class Light {
public:
    virtual const Color getColor() const = 0;
    virtual const float getPower() const = 0;

    //virtual void getRandomPoint(const glm::vec2& uv, glm::vec3& p) const = 0;
    //virtual void getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d) const = 0;

    virtual Color sample(const glm::vec2& uv, const IntersectRec& ir, glm::vec3& wi, float& pdf, VisibilityTester& vt) const = 0;
};

class PointLight : public Light {
public:
    PointLight(const glm::vec3& p, const Color& c, const float power_) : P (p), color (c), power (power_) {};

    const Color getColor() const {return color;}
    const float getPower() const {return power;}

    //void getRandomPoint(const glm::vec2& uv, glm::vec3& p) const {p = P;}
    //void getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d) const {
    //    p = P;
    //};
    Color sample(const glm::vec2& uv, const IntersectRec& ir, glm::vec3& wi, float& pdf, VisibilityTester& vt) const;

private:
    glm::vec3 P;
    Color color;
    float power;
};

class DirectionalLight;
class SpotLight;
class GeometricLight {
public: 
    GeometricLight(const std::shared_ptr <Primitive> _prim) : prim (_prim) {}

    const Color getColor() const {return color;}
    const float getPower() const {return power;}

    Color sample(const glm::vec2& uv, const IntersectRec& ir, glm::vec3& wi, float& pdf, VisibilityTester& vt) const;

private:
    Color color;
    float power;
    const std::shared_ptr <Primitive> prim;
};
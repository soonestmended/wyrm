#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Color.hpp"

class Light {
public:
    virtual const Color getColor() const = 0;
    virtual const float getPower() const = 0;

    virtual void getRandomPoint(const glm::vec2& uv, glm::vec3& p) const = 0;
    virtual void getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d) const = 0;

};

class PointLight : public Light {
public:
    PointLight(const glm::vec3& p, const Color& c) : P (p), color (c) {};

    const Color getColor() const {return color;}
    const float getPower() const {return power;}

    void getRandomPoint(const glm::vec2& uv, glm::vec3& p) const {p = P;}
    void getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d) const {
        p = P;
        // TODO: randomly sample unit sphere and return direction in d.
    };

private:
    glm::vec3 P;
    Color color;
    float power;
};

class DirectionalLight;
class SpotLight;
class GeometricLight;
#pragma once

#include <memory>

#include "BBox.hpp"
#include "Material.hpp"

struct Tri {
    int v[3];   // vertex indices
    int vt[3];  // texCoord indices
    int vn[3];  // vertex normal indices
    int m;      // MTLmaterial index
    int s;      // smoothing group
};

class Primitive {
protected:
    BBox bbox_;
    const Material& material_;

public:
    explicit Primitive(const Material &m) : material_ (m) {}
    virtual bool intersect() const = 0;
    virtual bool intersectYN() const = 0;
    const Material& getMaterial() const;
    const BBox& getBBox() const;

};

class Triangle : public Primitive {
private:
    glm::vec3 v_[3];
    glm::vec3 N_;
    std::shared_ptr <glm::vec3> uvw;

public:
    Triangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &N, const Material &m) 
    : Primitive(m), v_ ({v0, v1, v2}), N_ (N) {
        bbox_.enclose(v0);
        bbox_.enclose(v1);
        bbox_.enclose(v2);

    }
};
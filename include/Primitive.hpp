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
    bool explicitNormals = false;
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
protected:
    glm::vec3 v_[3];
    glm::vec3 N_;
    std::shared_ptr <glm::vec3> uvw;

public:
    Triangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &N, const Material &m) 
    : Primitive(m), v_ {v0, v1, v2}, N_ (N) {
        bbox_.enclose(v0);
        bbox_.enclose(v1);
        bbox_.enclose(v2);
    }

    bool intersect() const;
    bool intersectYN() const;
};

// This is a triangle with different normal at each vertex -- for normal smoothing in meshes
class TriangleWarp : public Triangle {
protected:
    glm::vec3 N1_, N2_;

public:
    TriangleWarp (const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &N0, const glm::vec3 &N1, const glm::vec3 &N2, const Material &m)
    : Triangle(v0, v1, v2, N0, m), N1_ (N1), N2_ (N2) {} 
};

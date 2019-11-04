#pragma once

#include <memory>

#include "BBox.hpp"
#include "IntersectRec.hpp"
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
    BBox bbox;
    const Material& material;

public:
    explicit Primitive(const Material &m) : material (m) {}
    virtual const bool intersect(const Ray& ray, const float tmin, const float tmax, IntersectRec& ir) const = 0;
	virtual const bool intersectYN(const Ray& ray, const float tmin, const float tmax) const = 0;
    const Material& getMaterial() const;
    const BBox& getBBox() const;

};

class Triangle : public Primitive {
protected:
    glm::vec3 v[3];
    glm::vec3 N;
    glm::vec3 e0, e1;
    std::shared_ptr <glm::vec3> uvw;

public:
    Triangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &N_, const Material &m_) 
    : Primitive(m_), v {v0, v1, v2}, N (N_) {
        bbox.enclose(v0);
        bbox.enclose(v1);
        bbox.enclose(v2);
        e0 = v1 - v0;
        e1 = v2 - v0;
    }

    const bool intersect(const Ray& ray, const float tmin, const float tmax, IntersectRec& ir) const;
	const bool intersectYN(const Ray& ray, const float tmin, const float tmax) const;
};

// This is a triangle with different normal at each vertex -- for normal smoothing in meshes
class TriangleWarp : public Triangle {
protected:
    glm::vec3 N1, N2;

public:
    TriangleWarp (const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &N0, const glm::vec3 &N1, const glm::vec3 &N2, const Material &m)
    : Triangle(v0, v1, v2, N0, m), N1 (N1), N2 (N2) {} 
};

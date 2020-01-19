#pragma once

#include <memory>
#include <glm/vec2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_inverse.hpp>

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
    const std::shared_ptr <Material> material;

public:
    explicit Primitive(const std::shared_ptr <Material> m) : material (m) {}
    virtual const bool intersect(const Ray& ray, const float tmin, const float tmax, IntersectRec& ir) const = 0;
	virtual const bool intersectYN(const Ray& ray, const float tmin, const float tmax) const = 0;
    const std::shared_ptr <Material> getMaterial() const;
    const BBox& getBBox() const;
    
    virtual void getRandomPoint(const glm::vec2& uv, glm::vec3& p) const = 0;
    virtual void getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d) const = 0;
    virtual const float getSurfaceArea() const = 0;

};

class Box : public Primitive {
public:
    Box(const glm::vec3 &size, const std::shared_ptr <Material> m) : Primitive(m) {
        bbox = BBox(-size/2.f, size/2.f);
    }
    const bool intersect(const Ray& ray, const float tmin, const float tmax, IntersectRec& ir) const;
	const bool intersectYN(const Ray& ray, const float tmin, const float tmax) const;

    void getRandomPoint(const glm::vec2& uv, glm::vec3& p) const;
    void getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d) const;
    const float getSurfaceArea() const;
};

class Triangle : public Primitive {
protected:
    glm::vec3 v[3];
    glm::vec3 N;
    glm::vec3 e0, e1;
    std::shared_ptr <glm::vec3> uvw;

public:
    Triangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &N_, const std::shared_ptr <Material> m_) 
    : Primitive(m_), v {v0, v1, v2}, N (N_) {
        bbox.enclose(v0);
        bbox.enclose(v1);
        bbox.enclose(v2);
        e0 = v1 - v0;
        e1 = v2 - v0;
    }

    const bool intersect(const Ray& ray, const float tmin, const float tmax, IntersectRec& ir) const;
	const bool intersectYN(const Ray& ray, const float tmin, const float tmax) const;

    void getRandomPoint(const glm::vec2& uv, glm::vec3& p) const;
    void getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d) const;
    const float getSurfaceArea() const;

};

// This is a triangle with different normal at each vertex -- for normal smoothing in meshes
class TriangleWarp : public Triangle {
protected:
    glm::vec3 N1, N2;

public:
    TriangleWarp (const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &N0, const glm::vec3 &N1, const glm::vec3 &N2, const std::shared_ptr <Material> m)
    : Triangle(v0, v1, v2, N0, m), N1 (N1), N2 (N2) {} 

    const bool intersect(const Ray& ray, const float tmin, const float tmax, IntersectRec& ir) const;

    void getRandomPoint(const glm::vec2& uv, glm::vec3& p) const;
    void getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d) const;

};

class TransformedPrimitive : public Primitive {
public:
    TransformedPrimitive(const glm::mat4 &xf, const shared_ptr <Primitive> p, const shared_ptr <Material> m) :
    Primitive (m), xform (xf), prim (p) {
        inv_xform = glm::inverse(xf);
    }

    shared_ptr <Primitive> prim;
    glm::mat4 xform;
    glm::mat4 inv_xform;

    const bool intersect(const Ray& ray, const float tmin, const float tmax, IntersectRec& ir) const;
        // transform ray to local coordinates
        // compute intersection
        // transform intersection components to world coordinates
        // put utility functions to transform normals, vectors, and points in Utils namespace
    
	const bool intersectYN(const Ray& ray, const float tmin, const float tmax) const;
        // transform ray to local coordinates
        // compute intersectYN

    void getRandomPoint(const glm::vec2& uv, glm::vec3& p) const;
        // get random point from primitive
        // transform to world coordinates

    void getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d) const;
        // get random point and direction from primitive
        // transform both to world coordinates

    const float getSurfaceArea() const;
        // need to scale this by ... xf determinant?
};
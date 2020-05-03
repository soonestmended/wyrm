#pragma once

#include <memory>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/norm.hpp> 

#include "common.hpp"
#include "BBox.hpp"
#include "IntersectRec.hpp"
#include "Material.hpp"
#include "Utils.hpp"
#include "XForm.hpp"

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
    virtual void finishIntersection(IntersectRec& ir) const {
        ir.shadingNormal = ir.normal;
        ir.onb.init(ir.shadingNormal); 
    }
    const std::shared_ptr <Material> getMaterial() const;
    const BBox& getBBox() const;
    
    virtual void getRandomPoint(const glm::vec2& uv, glm::vec3& p, float* pdf) const = 0;
    virtual const float getRandomPointPdf(const IntersectRec& ir, const glm::vec3& wi_local) const {
        IntersectRec lightIR;
        Ray r{ir.isectPoint, ir.onb.local2world(wi_local)};
        if (!intersect(r, EPSILON, POS_INF, lightIR)) return 0.f;
        float dist2 = glm::length2(lightIR.isectPoint - ir.isectPoint);
        return dist2 / (abs(glm::dot(lightIR.normal, -r.d)) * getSurfaceArea());
    }

    virtual void getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d, float* pdf) const = 0;
    virtual const float getSurfaceArea() const = 0;
    virtual const std::string toString() const = 0;
};

class Box : public Primitive {
public:
    Box(const glm::vec3 &size, const std::shared_ptr <Material> m) : Primitive(m) {
        bbox = BBox(-size/2.f, size/2.f);
    }
    Box(const glm::vec3 &llc, const glm::vec3 &urc, const std::shared_ptr <Material> m) : Primitive(m) {
        bbox = BBox(llc, urc);
    }
    const bool intersect(const Ray& ray, const float tmin, const float tmax, IntersectRec& ir) const;
	const bool intersectYN(const Ray& ray, const float tmin, const float tmax) const;

    void getRandomPoint(const glm::vec2& uv, glm::vec3& p, float* pdf) const;
    void getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d, float* pdf) const;
    const float getSurfaceArea() const;
    const std::string toString() const {return "Box";}
};

class Cone : public Primitive {
public:
    Cone(const float _bottomRadius, const float _height, const std::shared_ptr <Material> m) : 
    Primitive (m), bottomRadius (_bottomRadius), height (_height) {
        bbox = BBox(glm::vec3(-bottomRadius, -height/2.0f, -bottomRadius), glm::vec3(bottomRadius, height/2.0f, bottomRadius));
    }
    const bool intersect(const Ray& ray, const float tmin, const float tmax, IntersectRec& ir) const;
	const bool intersectYN(const Ray& ray, const float tmin, const float tmax) const;

    void getRandomPoint(const glm::vec2& uv, glm::vec3& p, float* pdf) const;
    void getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d, float* pdf) const;
    const float getSurfaceArea() const;
    const std::string toString() const {return "Cone";}

    float bottomRadius, height;
};

class Cylinder : public Primitive {
public:
    Cylinder(const float _radius, const float _height, const std::shared_ptr <Material> m) : 
    Primitive (m), radius (_radius), height (_height) {
        bbox = BBox(glm::vec3(-radius, -height/2.0f, -radius), glm::vec3(radius, height/2.0f, radius));
        std::cout << "Cyl bbox in constructor: ";
        std::cout << bbox.toString() << std::endl;
    }
    const bool intersect(const Ray& ray, const float tmin, const float tmax, IntersectRec& ir) const;
	const bool intersectYN(const Ray& ray, const float tmin, const float tmax) const;

    void getRandomPoint(const glm::vec2& uv, glm::vec3& p, float* pdf) const;
    void getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d, float* pdf) const;
    const float getSurfaceArea() const;    
    const std::string toString() const {return "Cylinder";}
    float radius, height;
};

class Sphere : public Primitive {
public:
    Sphere(const float _radius, const std::shared_ptr <Material> m) : 
    Primitive (m), radius (_radius) {
        bbox = BBox(glm::vec3(-radius), glm::vec3(radius));
    }
    const bool intersect(const Ray& ray, const float tmin, const float tmax, IntersectRec& ir) const;
	const bool intersectYN(const Ray& ray, const float tmin, const float tmax) const;

    void getRandomPoint(const glm::vec2& uv, glm::vec3& p, float* pdf) const;
    void getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d, float* pdf) const;
    const float getSurfaceArea() const;    
    const std::string toString() const {return "Sphere";}

    float radius;
};


class Triangle : public Primitive {
protected:
    glm::vec3 v[3];
    glm::vec3 N;
    glm::vec3 e0, e1;
    std::shared_ptr <glm::vec3> uvw;

public:
    Triangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const std::shared_ptr <Material> m_) 
    : Primitive(m_), v {v0, v1, v2} {
        bbox.enclose(v0);
        bbox.enclose(v1);
        bbox.enclose(v2);
        e0 = v1 - v0;
        e1 = v2 - v0;
        N = glm::normalize(glm::cross(e1, e0));
    }

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

    void getRandomPoint(const glm::vec2& uv, glm::vec3& p, float* pdf) const;
    void getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d, float* pdf) const;
    const float getSurfaceArea() const;
    const std::string toString() const {return "Triangle";}

};

// This is a triangle with different normal at each vertex -- for normal smoothing in meshes
class TriangleWarp : public Triangle {
protected:
    glm::vec3 N1, N2;

public:
    TriangleWarp (const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &N0, const glm::vec3 &N1, const glm::vec3 &N2, const std::shared_ptr <Material> m)
    : Triangle(v0, v1, v2, N0, m), N1 (N1), N2 (N2) {} 

    //const bool intersect(const Ray& ray, const float tmin, const float tmax, IntersectRec& ir) const;
    void finishIntersection(IntersectRec& ir) const {
        ir.shadingNormal = ir.uvw[0] * N + ir.uvw[1] * N1 + ir.uvw[2] * N2;
        ir.onb.init(ir.shadingNormal); 
    }
    void getRandomPoint(const glm::vec2& uv, glm::vec3& p, float* pdf) const;
    void getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d, float* pdf) const;
    const std::string toString() const {return "TriangleWarp";}

};

class TransformedPrimitive : public Primitive {
public:
    TransformedPrimitive(const glm::mat4 &xf, const std::shared_ptr <Primitive> p) :
    Primitive (p->getMaterial()), localToWorld (xf), prim (p) {
        bbox = localToWorld.transformBBox(p->getBBox());
    }

    std::shared_ptr <Primitive> prim;
    XForm localToWorld;
    
    void finishIntersection(IntersectRec& ir) const {
        // unforunately have to redo some work here
        prim->finishIntersection(ir);
        ir.shadingNormal = glm::normalize(localToWorld.transformNormal(ir.shadingNormal));
        ir.onb.init(ir.shadingNormal);
        
    }

    const bool intersect(const Ray& ray, const float tmin, const float tmax, IntersectRec& ir) const;
        // transform ray to local coordinates
        // compute intersection
        // transform intersection components to world coordinates
        // put utility functions to transform normals, vectors, and points in Utils namespace
    
	const bool intersectYN(const Ray& ray, const float tmin, const float tmax) const;
        // transform ray to local coordinates
        // compute intersectYN

    void getRandomPoint(const glm::vec2& uv, glm::vec3& p, float* pdf) const;
        // get random point from primitive
        // transform to world coordinates

    void getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d, float* pdf) const;
        // get random point and direction from primitive
        // transform both to world coordinates

    const float getSurfaceArea() const;
        // need to scale this by ... xf determinant?

    const std::string toString() const {return "Transformed: " + prim->toString();}


};
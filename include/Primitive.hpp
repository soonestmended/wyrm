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
    virtual const bool intersect(const Ray& ray, const Real tmin, const Real tmax, IntersectRec& ir) const = 0;
	virtual const bool intersectYN(const Ray& ray, const Real tmin, const Real tmax) const = 0;
    virtual void finishIntersection(IntersectRec& ir) const {
        ir.shadingNormal = ir.normal;
        ir.onb.init(ir.shadingNormal); 
    }
    const std::shared_ptr <Material> getMaterial() const;
    const BBox& getBBox() const;
    
    virtual void getRandomPoint(const Vec2& uv, Vec3& p, Real* pdf) const = 0;
    virtual const Real getRandomPointPdf(const IntersectRec& ir, const Vec3& wi_local) const {
        IntersectRec lightIR;
        Ray r{ir.isectPoint, ir.onb.local2world(wi_local)};
        if (!intersect(r, EPSILON, POS_INF, lightIR)) return 0;
        Real dist2 = glm::length2(lightIR.isectPoint - ir.isectPoint);
        //std::cout << "\t\tdist2: " << dist2 << std::endl;
        //std::cout << "\t\tSA: " << getSurfaceArea() << std::endl;
        //std::cout << "\t\tdot prod: " << abs(glm::dot(lightIR.normal, -r.d)) << std::endl;
        return dist2 / (abs(glm::dot(lightIR.normal, -r.d)) * getSurfaceArea());
    }

    virtual void getRandomPointAndDirection(const Vec2& uv, Vec3& p, Vec3& d, Real* pdf) const = 0;
    virtual const Real getSurfaceArea() const = 0;
    virtual const std::string toString() const = 0;
};

class Box : public Primitive {
public:
    Box(const Vec3 &size, const std::shared_ptr <Material> m) : Primitive(m) {
        bbox = BBox(-size/2., size/2.);
    }
    Box(const Vec3 &llc, const Vec3 &urc, const std::shared_ptr <Material> m) : Primitive(m) {
        bbox = BBox(llc, urc);
    }
    const bool intersect(const Ray& ray, const Real tmin, const Real tmax, IntersectRec& ir) const;
	const bool intersectYN(const Ray& ray, const Real tmin, const Real tmax) const;

    void getRandomPoint(const Vec2& uv, Vec3& p, Real* pdf) const;
    void getRandomPointAndDirection(const Vec2& uv, Vec3& p, Vec3& d, Real* pdf) const;
    const Real getSurfaceArea() const;
    const std::string toString() const {return "Box";}
};

class Cone : public Primitive {
public:
    Cone(const Real _bottomRadius, const Real _height, const std::shared_ptr <Material> m) : 
    Primitive (m), bottomRadius (_bottomRadius), height (_height) {
        bbox = BBox(Vec3(-bottomRadius, -height/2.0f, -bottomRadius), Vec3(bottomRadius, height/2.0f, bottomRadius));
    }
    const bool intersect(const Ray& ray, const Real tmin, const Real tmax, IntersectRec& ir) const;
	const bool intersectYN(const Ray& ray, const Real tmin, const Real tmax) const;

    void getRandomPoint(const Vec2& uv, Vec3& p, Real* pdf) const;
    void getRandomPointAndDirection(const Vec2& uv, Vec3& p, Vec3& d, Real* pdf) const;
    const Real getSurfaceArea() const;
    const std::string toString() const {return "Cone";}

    Real bottomRadius, height;
};

class Cylinder : public Primitive {
public:
    Cylinder(const Real _radius, const Real _height, const std::shared_ptr <Material> m) : 
    Primitive (m), radius (_radius), height (_height) {
        bbox = BBox(Vec3(-radius, -height/2.0f, -radius), Vec3(radius, height/2.0f, radius));
        std::cout << "Cyl bbox in constructor: ";
        std::cout << bbox.toString() << std::endl;
    }
    const bool intersect(const Ray& ray, const Real tmin, const Real tmax, IntersectRec& ir) const;
	const bool intersectYN(const Ray& ray, const Real tmin, const Real tmax) const;

    void getRandomPoint(const Vec2& uv, Vec3& p, Real* pdf) const;
    void getRandomPointAndDirection(const Vec2& uv, Vec3& p, Vec3& d, Real* pdf) const;
    const Real getSurfaceArea() const;    
    const std::string toString() const {return "Cylinder";}
    Real radius, height;
};

class Sphere : public Primitive {
public:
    Sphere(const Real _radius, const std::shared_ptr <Material> m) : 
    Primitive (m), radius (_radius) {
        bbox = BBox(Vec3(-radius), Vec3(radius));
    }
    const bool intersect(const Ray& ray, const Real tmin, const Real tmax, IntersectRec& ir) const;
	const bool intersectYN(const Ray& ray, const Real tmin, const Real tmax) const;

    void getRandomPoint(const Vec2& uv, Vec3& p, Real* pdf) const;
    void getRandomPointAndDirection(const Vec2& uv, Vec3& p, Vec3& d, Real* pdf) const;
    const Real getSurfaceArea() const;    
    const std::string toString() const {return "Sphere";}

    Real radius;
};


class Triangle : public Primitive {
protected:
    Vec3 v[3];
    Vec3 N;
    Vec3 e0, e1;
    std::shared_ptr <Vec3> uvw;

public:
    Triangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, const std::shared_ptr <Material> m_) 
    : Primitive(m_), v {v0, v1, v2} {
        bbox.enclose(v0);
        bbox.enclose(v1);
        bbox.enclose(v2);
        e0 = v1 - v0;
        e1 = v2 - v0;
        N = glm::normalize(glm::cross(e1, e0));
    }

    Triangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, const Vec3 &N_, const std::shared_ptr <Material> m_) 
    : Primitive(m_), v {v0, v1, v2}, N (N_) {
        bbox.enclose(v0);
        bbox.enclose(v1);
        bbox.enclose(v2);
        e0 = v1 - v0;
        e1 = v2 - v0;
    }

    const bool intersect(const Ray& ray, const Real tmin, const Real tmax, IntersectRec& ir) const;
	const bool intersectYN(const Ray& ray, const Real tmin, const Real tmax) const;

    void getRandomPoint(const Vec2& uv, Vec3& p, Real* pdf) const;
    void getRandomPointAndDirection(const Vec2& uv, Vec3& p, Vec3& d, Real* pdf) const;
    const Real getSurfaceArea() const;
    const std::string toString() const {return "Triangle";}

};

// This is a triangle with different normal at each vertex -- for normal smoothing in meshes
class TriangleWarp : public Triangle {
protected:
    Vec3 N1, N2;

public:
    TriangleWarp (const Vec3 &v0, const Vec3 &v1, const Vec3 &v2, const Vec3 &N0, const Vec3 &N1, const Vec3 &N2, const std::shared_ptr <Material> m)
    : Triangle(v0, v1, v2, N0, m), N1 (N1), N2 (N2) {} 

    //const bool intersect(const Ray& ray, const Real tmin, const Real tmax, IntersectRec& ir) const;
    void finishIntersection(IntersectRec& ir) const {
        ir.shadingNormal = ir.uvw[0] * N + ir.uvw[1] * N1 + ir.uvw[2] * N2;
        ir.onb.init(ir.shadingNormal); 
    }
    void getRandomPoint(const Vec2& uv, Vec3& p, Real* pdf) const;
    void getRandomPointAndDirection(const Vec2& uv, Vec3& p, Vec3& d, Real* pdf) const;
    const std::string toString() const {return "TriangleWarp";}

};

class TransformedPrimitive : public Primitive {
public:
    TransformedPrimitive(const Mat4 &xf, const std::shared_ptr <Primitive> p) :
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

    const bool intersect(const Ray& ray, const Real tmin, const Real tmax, IntersectRec& ir) const;
        // transform ray to local coordinates
        // compute intersection
        // transform intersection components to world coordinates
        // put utility functions to transform normals, vectors, and points in Utils namespace
    
	const bool intersectYN(const Ray& ray, const Real tmin, const Real tmax) const;
        // transform ray to local coordinates
        // compute intersectYN

    void getRandomPoint(const Vec2& uv, Vec3& p, Real* pdf) const;
        // get random point from primitive
        // transform to world coordinates

    void getRandomPointAndDirection(const Vec2& uv, Vec3& p, Vec3& d, Real* pdf) const;
        // get random point and direction from primitive
        // transform both to world coordinates

    const Real getSurfaceArea() const;
        // need to scale this by ... xf determinant?

    const std::string toString() const {return "Transformed: " + prim->toString();}


};
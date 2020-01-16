#include "common.hpp"
#include "Primitive.hpp"
#include "Utils.hpp"

using namespace std;

const shared_ptr <Material> Primitive::getMaterial() const {
    return material;
}

const BBox& Primitive::getBBox() const {
    return bbox;
}

const bool Triangle::intersect(const Ray &ray, float tmin, float tmax, IntersectRec& ir) const {
    glm::vec3 P, Q, T;
    float det, inv_det, u, v;
    float t;

    //Begin calculating determinant - also used to calculate u parameter
    P = glm::cross(ray.d, e1);
 
    //if determinant is near zero, ray lies in plane of triangle
    det = glm::dot(e0, P);

    //NOT CULLING
    if((det > -EPSILON) && (det < EPSILON)) return false;
    inv_det = 1.f / det;
 
    //calculate distance from V0 to ray origin
    T = ray.o - this->v[0];
 
    //Calculate u parameter and test bound
    u = glm::dot(T, P) * inv_det;
  
    //The intersection lies outside of the triangle
    if(u <= 0.f || u >= 1.f) return false;
 
    //Prepare to test v parameter
    Q = glm::cross(T, e0);
 
    //Calculate V parameter and test bound
    v = glm::dot(ray.d, Q) * inv_det;

    //The intersection lies outside of the triangle
    if(v <= 0.f || u + v  >= 1.f) return false;
 
    t = glm::dot(e1, Q) * inv_det;
 
    if(t >= tmin && t <= tmax) { //ray intersection
        ir.t = t;
        ir.normal = this->N;
        ir.material = this->getMaterial();
        ir.isectPoint = ray.o + t * ray.d;
        ir.uvw[1] = u;
        ir.uvw[2] = v;
        ir.uvw[0] = 1.f - u - v;
        //std::cout << ir.normal << std::endl;
        return true;
    }
 
    // No hit, no win
    return false;
}

const bool Triangle::intersectYN(const Ray &ray, float tmin, float tmax) const  {
    glm::vec3 P, Q, T;
    float det, inv_det, u, v;
    float t;

    //Begin calculating determinant - also used to calculate u parameter
    P = glm::cross(ray.d, e1);
 
    //if determinant is near zero, ray lies in plane of triangle
    det = glm::dot(e0, P);

    //NOT CULLING
    if((det > -EPSILON) && (det < EPSILON)) return false;
    inv_det = 1.f / det;
 
    //calculate distance from V0 to ray origin
    T = ray.o - this->v[0];
 
    //Calculate u parameter and test bound
    u = glm::dot(T, P) * inv_det;
  
    //The intersection lies outside of the triangle
    if(u < 0.f || u > 1.f) return false;
 
    //Prepare to test v parameter
    Q = glm::cross(T, e0);
 
    //Calculate V parameter and test bound
    v = glm::dot(ray.d, Q) * inv_det;

    //The intersection lies outside of the triangle
    if(v < 0.f || u + v  > 1.f) return false;
 
    t = glm::dot(e1, Q) * inv_det;
 
    if(t >= tmin && t <= tmax) { //ray intersection
        return true;
    }
 
    // No hit, no win
    return false;
}

const bool TriangleWarp::intersect(const Ray& ray, const float tmin, const float tmax, IntersectRec& ir) const {
    if (Triangle::intersect(ray, tmin, tmax, ir)) {
        ir.normal = ir.uvw[0] * N + ir.uvw[1] * N1 + ir.uvw[2] * N2;
        //ir.normal = N2;
        return true;
    }
    return false;
}


void Triangle::getRandomPoint(const glm::vec2& uv, glm::vec3& p) const {
    glm::vec2 bg;
	utils::uniformSampleTriangle(uv, bg);
	p = bg[0] * v[0] + bg[1] * v[1] + (1.f - bg[0] - bg[1]) * v[2];
}
    
void Triangle::getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d) const {
    this->getRandomPoint(uv, p);
    d = this->N;
}

void TriangleWarp::getRandomPoint(const glm::vec2& uv, glm::vec3& p) const {
    return Triangle::getRandomPoint(uv, p);
}
    
void TriangleWarp::getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d) const {
    return Triangle::getRandomPointAndDirection(uv, p, d);
}

const float Triangle::getSurfaceArea() const {
	// Heron's method
	float a = e0.length();
	float b = e1.length();
	float c = (v[2]-v[1]).length();
	float s = 0.5 * (a+b+c);
	return sqrtf(s*(s-a)*(s-b)*(s-c));
}

const bool Box::intersectYN(const Ray& ray, const float tmin, const float tmax) const {
    return bbox.intersectYN(ray, tmin, tmax);
}

const bool Box::intersect(const Ray& r, float t0, float t1, IntersectRec& ir) const {
	float tmin, tmax, tymin, tymax, tzmin, tzmax;
	enum Side {TOP, BOTTOM, LEFT, RIGHT, NEAR, FAR};
    Side hitSide = r.sign[0] ? RIGHT : LEFT;

	const glm::vec3 *bounds[2] = {&bbox.min, &bbox.max};

	tmin = ((*bounds[r.sign[0]])[0] - r.o[0]) * r.inv_d[0];
	tmax = ((*bounds[1-r.sign[0]])[0] - r.o[0]) * r.inv_d[0];

	tymin = ((*bounds[r.sign[1]])[1] - r.o[1]) * r.inv_d[1];
	tymax = ((*bounds[1-r.sign[1]])[1] - r.o[1]) * r.inv_d[1];

	if ( (tmin > tymax) || (tymin > tmax) )
		return false;

	if (tymin > tmin) {
		tmin = tymin;
        hitSide = r.sign[1] ? BOTTOM : TOP;
    }

	if (tymax < tmax) 
		tmax = tymax;

	tzmin = ((*bounds[r.sign[2]])[2] - r.o[2]) * r.inv_d[2];
	tzmax = ((*bounds[1-r.sign[2]])[2] - r.o[2]) * r.inv_d[2];

	if ( (tmin > tzmax) || (tzmin > tmax) )
		return false;

	if (tzmin > tmin) {
		tmin = tzmin;
        hitSide = r.sign[2] ? FAR : NEAR;
    }

	if (tzmax < tmax)
		tmax = tzmax;

	if ( (tmin < t1) && (tmax > t0) ) {
        // hit
        ir.material = this->material;
        ir.t = tmin;
        ir.isectPoint = r.o + ir.t * r.d;
        if (hitSide == LEFT)
            ir.normal = glm::vec3(-1, 0, 0);
        else if (hitSide == RIGHT)
            ir.normal = glm::vec3(1, 0, 0);
        else if (hitSide == BOTTOM)
            ir.normal = glm::vec3(0, -1, 0);
        else if (hitSide == TOP)
            ir.normal = glm::vec3(0, 1, 0);
        else if (hitSide == NEAR) 
            ir.normal = glm::vec3(0, 0, -1);
        else
            ir.normal = glm::vec3(0, 0, 1);
        return true;
    }
    else {
        return false;
    }
}

const bool TransformedPrimitive::intersect(const Ray& ray, const float tmin, const float tmax, IntersectRec& ir) const {
    // transform ray to local coordinates
    // compute intersection
    // transform intersection components to world coordinates
    // put utility functions to transform normals, vectors, and points in Utils namespace
    
}

const bool TransformedPrimitive::intersectYN(const Ray& ray, const float tmin, const float tmax) const {
    // transform ray to local coordinates
    // compute intersectYN
}

void TransformedPrimitive::getRandomPoint(const glm::vec2& uv, glm::vec3& p) const {
    // get random point from primitive
    // transform to world coordinates
}

void TransformedPrimitive::getRandomPointAndDirection(const glm::vec2& uv, glm::vec3& p, glm::vec3& d) const {
    // get random point and direction from primitive
    // transform both to world coordinates
}

const float TransformedPrimitive::getSurfaceArea() const {
    // need to scale this by ... xf determinant?
}
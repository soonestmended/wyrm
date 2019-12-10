#include "common.hpp"
#include "Primitive.hpp"
#include "Utils.hpp"

const Material& Primitive::getMaterial() const {
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
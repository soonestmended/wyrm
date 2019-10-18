#include "common.hpp"
#include "Primitive.hpp"

const Material& Primitive::getMaterial() const {
    return material;
}

const BBox& Primitive::getBBox() const {
    return bbox;
}

// TODO: Implement intersection.

const bool Triangle::intersect(const Ray &ray, IntersectRec &ir, float tmin, float tmax) const {
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
        ir.t = t;
        ir.normal = this->N;
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
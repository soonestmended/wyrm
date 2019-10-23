#include "BBox.hpp"

void BBox::enclose(const glm::vec3& v) {
    min.x = min.x < v.x ? min.x : v.x;
    min.y = min.y < v.y ? min.y : v.y;
    min.z = min.z < v.z ? min.z : v.z;

    max.x = max.x > v.x ? max.x : v.x;
    max.y = max.y > v.y ? max.y : v.y;
    max.z = max.z > v.z ? max.z : v.z;
    
}

void BBox::enclose(const BBox& bbox) {
    min.x = min.x < bbox.min.x ? min.x : bbox.min.x;
    min.y = min.y < bbox.min.y ? min.y : bbox.min.y;
    min.z = min.z < bbox.min.z ? min.z : bbox.min.z;

    max.x = max.x > bbox.max.x ? max.x : bbox.max.x;
    max.y = max.y > bbox.max.y ? max.y : bbox.max.y;
    max.z = max.z > bbox.max.z ? max.z : bbox.max.z;
}
	
bool BBox::intersect(const Ray &r, float &a, float &b) const {
    return false;
}
	
bool BBox::intersectYN(const Ray &r, const float t0, const float t1) const {
    return false;
}

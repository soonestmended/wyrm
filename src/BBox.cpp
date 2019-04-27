#include "BBox.hpp"

void BBox::enclose(const glm::vec3& v) {
}

void BBox::enclose(const BBox& bbox) {

}
	
bool BBox::intersect(const Ray &r, float &a, float &b) const {
    return false;
}
	
bool BBox::intersectYN(const Ray &r, const float t0, const float t1) const {
    return false;
}

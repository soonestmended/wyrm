#pragma once

#include <limits>

#include <glm/vec3.hpp>

#include "Ray.hpp"

class BBox {
public:
    glm::vec3 min;
    glm::vec3 max;

    BBox() : min (std::numeric_limits<float>::max()), max (std::numeric_limits<float>::min()) {}
    BBox(const glm::vec3 &min_, const glm::vec3 &max_) : min (min_), max (max_) {}

    const glm::vec3 getCentroid() const {
        return glm::vec3({.5*(max[0] + min[0]), .5*(max[1] + min[1]), .5*(max[2] + min[2])});
    }

	void enclose(const glm::vec3& v);
	void enclose(const BBox& bbox);
	
	bool intersect(const Ray &r, float &a, float &b) const;
	bool intersectYN(const Ray &r, const float t0, const float t1) const;

};
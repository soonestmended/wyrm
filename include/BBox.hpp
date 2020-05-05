#pragma once

#include <limits>
#include <string>
#include <sstream>

#include <glm/vec3.hpp>
#include "common.hpp"
#include "Ray.hpp"

class BBox {
public:
    Vec3 min;
    Vec3 max;

    BBox() : min (std::numeric_limits<Real>::max()), max (-std::numeric_limits<Real>::max()) {}
    BBox(const Vec3 &min_, const Vec3 &max_) : min (min_), max (max_) {}

    const Vec3 getCentroid() const {
        return Vec3({.5*(max[0] + min[0]), .5*(max[1] + min[1]), .5*(max[2] + min[2])});
    }

    const std::string toString() const {
        std::stringstream ss;
        ss << "(" << min.x << ", " << min.y << ", " << min.z << ") --> (" << max.x << ", " << max.y << ", " << max.z << ")";
        return ss.str();
    }

	void enclose(const Vec3& v);
	void enclose(const BBox& bbox);
	
	bool intersect(const Ray &r, Real &a, Real &b) const;
	bool intersectYN(const Ray &r, const Real t0, const Real t1) const;

	friend std::ostream& operator<<(std::ostream& os, const BBox& bbox) {
		os << "[" << bbox.min[0] << ", " << bbox.min[1] << ", " << bbox.min[2] << "] - [";
		os << bbox.max[0] << ", " << bbox.max[1] << ", " << bbox.max[2] << "]" << std::endl;

		return os;
	}
};
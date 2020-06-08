#pragma once

#include <iostream>

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

#include "common.hpp"

class Ray {
public:
	Ray() {}

	Ray(const Vec3& o_, const Vec3& d_) : o (o_), d (d_) {
      //		inv_d = Vec3({1./d[0], 1./d[1], 1./d[2]});
		sign[0] = (d[0] < 0);
		sign[1] = (d[1] < 0);
		sign[2] = (d[2] < 0);
	}

	const Ray operator- (void) const {
		return Ray(this->o, -this->d);
	} 
	
	void normalize() {
		*this = Ray(o, glm::normalize(d));
	}

  Vec3 o, d; //, inv_d;
	unsigned char sign[3];
    double pad;
};

std::ostream& operator<<(std::ostream& os, const Ray& ray);	

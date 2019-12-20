#pragma once

#include <glm/vec3.hpp>

#include "Material.hpp"

class Primitive;

class IntersectRec {
public:
	IntersectRec() {}
	glm::vec3 isectPoint;
	glm::vec3 normal;
	glm::vec3 uvw; // parameters at intersection point
	float t;
	
	glm::mat3 worldToLocal;
	glm::mat3 localToWorld;
	
	std::shared_ptr <Primitive> primitive;
	std::shared_ptr <Material> material;

	void init() {
		ivoid branchlessONB(const Vec3f &n, Vec3f &b1, Vec3f &b2)
{
float sign = copysignf(1.0f, n.z);
const float a = -1.0f / (sign + n.z);
const float b = n.x * n.y * a;
b1 = Vec3f(1.0f + sign * n.x * n.x * a, sign * b, -sign * n.x);
b2 = Vec3f(b, sign + n.y * n.y * a, -n.y);
}
	}

};
#pragma once

#include <glm/vec3.hpp>
#include "ONB.hpp"
#include "Material.hpp"

class Primitive;

class IntersectRec {
public:
	IntersectRec() {}
	glm::vec3 isectPoint;
	glm::vec3 normal;
	glm::vec3 uvw; // parameters at intersection point
	float t;
	
	Primitive* primitive;
	std::shared_ptr <Material> material;
};
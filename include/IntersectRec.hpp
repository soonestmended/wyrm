#pragma once

#include <memory>
#include <glm/vec3.hpp>
#include "ONB.hpp"

class Material;
class Primitive;

class IntersectRec {
public:
	IntersectRec() {}
	glm::vec3 isectPoint;
	glm::vec3 normal, shadingNormal;
	glm::vec3 uvw; // parameters at intersection point
	float t;
	ONB onb;
	Primitive* primitive;
	std::shared_ptr <Material> material;
};
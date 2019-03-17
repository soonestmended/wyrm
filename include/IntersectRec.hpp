#pragma once

#include <glm/vec3.hpp>

#include "Material.hpp"
#include "Primitive.hpp"

class IntersectRec {
public:
	glm::vec3 isectPoint;
	glm::vec3 normal;
	float t;

	const Primitive& primitive_;
	const Material& material_;
};
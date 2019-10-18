#pragma once

#include <glm/vec3.hpp>

#include "Material.hpp"

class Primitive;

class IntersectRec {
public:
	glm::vec3 isectPoint;
	glm::vec3 normal;
	float t;

	const Primitive& primitive;
	const Material& material;
};
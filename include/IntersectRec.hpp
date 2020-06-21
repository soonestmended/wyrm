#pragma once

#include <memory>
#include <glm/vec3.hpp>
#include "common.hpp"
#include "ONB.hpp"

class Material;
class Primitive;

class IntersectRec {
public:
	IntersectRec() {}
	Vec3 isectPoint;
	Vec3 normal, shadingNormal;
	Vec3 uvw; // parameters at intersection point; used in finishIntersection to compute smooth normals / tex coords
  Vec3 tc;
	Real t;
	ONB onb;
	Primitive* primitive;
	std::shared_ptr <Material> material;
};

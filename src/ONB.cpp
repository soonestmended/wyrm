#include "ONB.hpp"

#include <glm/geometric.hpp>

inline glm::vec3 ONB::world2local(const glm::vec3& v) const {
	return glm::vec3(glm::dot(v, U), glm::dot(v, V), glm::dot(v, N));
}

inline glm::vec3 ONB::local2world(const glm::vec3& v) const {
	return glm::vec3(U[0]*v[0] + V[0]*v[1] + N[0]*v[2],
				     U[1]*v[0] + V[1]*v[1] + N[1]*v[2],
			     	 U[2]*v[0] + V[2]*v[1] + N[2]*v[2]);				
}
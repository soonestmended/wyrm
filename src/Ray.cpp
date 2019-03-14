#include <glm/gtx/string_cast.hpp>

#include "Ray.hpp"

std::ostream& operator<<(std::ostream& os, const Ray& ray) {
	os << glm::to_string(ray.o_) << " --> " << glm::to_string(ray.d_);
	return os;
}
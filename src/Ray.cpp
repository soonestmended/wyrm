#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/string_cast.hpp>

#include "Ray.hpp"

std::ostream& operator<<(std::ostream& os, const Ray& ray) {
	os << glm::to_string(ray.o) << " --> " << glm::to_string(ray.d);
	return os;
}

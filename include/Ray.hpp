#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

class Ray {
public:
	Ray() {}

	Ray(const glm::vec3& o, const glm::vec3& d) : o_ (o), d_ (d) {
		inv_d = glm::vec3({1./d[0], 1./d[1], 1./d[2]});
		sign[0] = (inv_d[0] < 0);
		sign[1] = (inv_d[1] < 0);
		sign[2] = (inv_d[2] < 0);
	}

	const Ray operator- (void) const {
		return Ray(this->o, -this->d);
	} 
	
	void normalize() {
		*this = Ray(o, glm::normalize(d));
	}

	glm::vec3 o, d, inv_d;
	unsigned char sign[3];
};

std::ostream& operator<<(std::ostream& os, const Ray& ray);	

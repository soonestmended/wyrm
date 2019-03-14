#include <iostream>

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>


class Ray {
public:
	Ray() {}

	Ray(const glm::vec3& o, const glm::vec3& d) : o_ (o), d_ (d) {
		inv_d_ = glm::vec3({1./d_[0], 1./d_[1], 1./d_[2]});
		sign_[0] = (inv_d_[0] < 0);
		sign_[1] = (inv_d_[1] < 0);
		sign_[2] = (inv_d_[2] < 0);
	}

	const Ray operator- (void) const {
		return Ray(this->o_, -this->d_);
	} 
	
	void normalize() {
		*this = Ray(o_, glm::normalize(d_));
	}

	glm::vec3 o_, d_, inv_d_;
	unsigned char sign_[3];
};

std::ostream& operator<<(std::ostream& os, const Ray& ray);	

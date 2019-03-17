#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

#include "Ray.hpp"

class Camera {
public:
	Camera() {}

	Camera(const glm::vec3& _eye, const glm::vec3& _look, const glm::vec3& _up, const glm::vec3& _right, float _s) :
		eye (_eye), look (_look), up (_up), right (_right), s (_s) {}

	// Generate ray through the image plane, which goes from -cam.right/2 --> +cam.right/2
	// width of image plane is cam.right.magnitude; height is cam.up.magnitude
	// whole image plane is covered by sampling [-0.5, 0.5] x [-0.5, 0.5]
	virtual Ray getRay(const glm::vec2& center) const {
		return Ray(eye, glm::normalize(center[0] * right + center[1] * up + s * look));
	}

protected:
	glm::vec3 eye, look, up, right;
	float s;	// distance to image plane
								
};
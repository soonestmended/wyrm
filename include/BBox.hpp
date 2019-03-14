#include <glm/vec3.hpp>

#include <limits>

class BBox {
public:
    glm::vec3 min_;
    glm::vec3 max_;

    BBox() : min_ (std::numeric_limits<float>::max()), max_ (std::numeric_limits<float>::min()) {}
    BBox(const glm::vec3 &min, const glm::vec3 &max) : min_ (min), max_ (max) {}

    const glm::vec3 getCentroid() const {
        return glm::vec3({.5*(max_[0] + min_[0]), .5*(max_[1] + min_[1]), .5*(max_[2] + min_[2])});
    }

	void enclose(const glm::vec3& v);
	void enclose(const BBox& bbox);
	
	bool intersect(const Ray &r, float &a, float &b) const;
	bool intersectYN(const Ray &r, const float t0, const float t1) const;

}
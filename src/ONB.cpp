#include "ONB.hpp"

#include <glm/geometric.hpp>

inline glm::vec3 ONB::local2world(const glm::vec3& v) const {
	return glm::vec3(U[0]*v[0] + V[0]*v[1] + W[0]*v[2],
				     U[1]*v[0] + V[1]*v[1] + W[1]*v[2],
			     	 U[2]*v[0] + V[2]*v[1] + W[2]*v[2]);				
}

inline glm::vec3 ONB::makeVec(const float u, const float v, const float w) const {
	return u*U + v*V + w*W;
}

inline glm::vec3 ONB::makeUnitVec(const float u, const float v, const float w) const {
	return glm::normalize(u*U + v*V + w*W);
}

inline float ONB::cosTheta(const glm::vec3& w) {return w[2];}

inline float ONB::sinTheta2(const glm::vec3& w) {return fmax(0.f, 1.f - cosTheta(w)*cosTheta(w));}

inline float ONB::sinTheta(const glm::vec3& w) {return sqrtf(sinTheta2(w));}

inline float ONB::cosPhi(const glm::vec3& w) {
	float sintheta = sinTheta(w);
	if (sintheta == 0.f) return 1.f;
	return glm::clamp(w[1] / sintheta, -1.f, 1.f);
}

inline float ONB::sinPhi(const glm::vec3& w) {
	float sintheta = sinTheta(w);
	if (sintheta == 0.f) return 0.f;
	return glm::clamp(w[1] / sintheta, -1.f, 1.f);
}

inline bool ONB::sameHemisphere(const glm::vec3& wi, const glm::vec3& wo) {
	return wi[2] * wo[2] > 0.f;
}
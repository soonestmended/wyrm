#pragma once

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <math.h>

class ONB {

public:
    glm::vec3 U, V, W;

    ONB() {}

    ONB(const glm::vec3& _N) {
        init(_N);
    }

    // from Pixar (shrug)
    void init(const glm::vec3 &_N) {
        W = _N;
        float sign = copysignf(1.0f, W.z);
        const float a = -1.0f / (sign + W.z);
        const float b = W.x * W.y * a;
        U = glm::vec3(1.0f + sign * W.x * W.x * a, sign * b, -sign * W.x);
        V = glm::vec3(b, sign + W.y * W.y * a, -W.y);
    }

    glm::vec3 world2local(const glm::vec3& v) const {
        return glm::vec3(glm::dot(v, U), glm::dot(v, V), glm::dot(v, W));
    }
    
    glm::vec3 local2world(const glm::vec3& v) const;

    static glm::vec3 uniformSampleHemisphere(float u, float v);
	static float uniformSampleHemispherePDF();

	static const glm::vec3 cosineSampleHemisphere(const glm::vec2& uv);
	static inline float cosineSampleHemispherePDF(float costheta, float phi);

	static void uniformSampleDisk(float u1, float u2, float *x, float *y);
	static inline const glm::vec2 concentricSampleDisk(const glm::vec2& uv);

	static bool sameHemisphere(const glm::vec3& wi, const glm::vec3& wo);
	glm::vec3 makeVec(const float u, const float v, const float w) const;
	glm::vec3 makeUnitVec(const float u, const float v, const float w) const;
	
    
	static float cosTheta(const glm::vec3& w);
	static float absCosTheta(const glm::vec3& w) {
        return fabsf(w[2]);
    }
	static float sinTheta2(const glm::vec3& w);
	static float sinTheta(const glm::vec3& w);
	static float cosPhi(const glm::vec3& w);
	static float sinPhi(const glm::vec3& w);

};
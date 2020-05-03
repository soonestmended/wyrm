#pragma once

#include <algorithm>
#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
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
    

    glm::vec3 local2world(const glm::vec3& v) const {
        return glm::vec3(U[0]*v[0] + V[0]*v[1] + W[0]*v[2],
                        U[1]*v[0] + V[1]*v[1] + W[1]*v[2],
                        U[2]*v[0] + V[2]*v[1] + W[2]*v[2]);				
    }

    glm::vec3 makeVec(const float u, const float v, const float w) const {
        return u*U + v*V + w*W;
    }

    glm::vec3 makeUnitVec(const float u, const float v, const float w) const {
        return glm::normalize(u*U + v*V + w*W);
    }

    static float cosTheta(const glm::vec3& w) {return w[2];}

    static float sinTheta2(const glm::vec3& w) {return fmax(0.f, 1.f - cosTheta(w)*cosTheta(w));}

    static float sinTheta(const glm::vec3& w) {return sqrtf(sinTheta2(w));}

    static float cosPhi(const glm::vec3& w) {
        float sintheta = sinTheta(w);
        if (sintheta == 0.f) return 1.f;
        return glm::clamp(w[1] / sintheta, -1.f, 1.f);
    }

    static float sinPhi(const glm::vec3& w) {
        float sintheta = sinTheta(w);
        if (sintheta == 0.f) return 0.f;
        return glm::clamp(w[1] / sintheta, -1.f, 1.f);
    }


    static const glm::vec2 concentricSampleDisk(const glm::vec2& uv) {
        glm::vec2 offset = 2.f * uv - glm::vec2(1.0);
        if (offset.x == 0.f && offset.y == 0.f) {
            return offset;
        }
        float theta, r;
        if (abs(offset.x) > abs(offset.y)) {
            r = offset.x;
            theta = M_PI_4 * (offset.y / offset.x);
        } else {
            r = offset.y;
            theta = M_PI_2 - M_PI_4 * (offset.x / offset.y);
        }
        return r * glm::vec2(cos(theta), sin(theta));
    }

    static const glm::vec3 cosineSampleHemisphere(const glm::vec2& uv) {
        glm::vec2 d = concentricSampleDisk(uv);
        float z = sqrt(std::max(0.f, 1.0f - d.x*d.x -d.y*d.y));
        return glm::vec3(d.x, d.y, z);
    }

    static float cosineSampleHemispherePDF(float cosTheta) {
        return cosTheta * M_1_PI;
    }

    static glm::vec3 uniformSampleHemisphere(const glm::vec2& uv) {
        float z = uv[0];
        float r = sqrt(std::max(0.f, 1.0f - z*z));
        float phi = 2. * M_PI * uv[1];
        return glm::vec3(r * cos(phi), r * sin(phi), z);
    }

	static float uniformSampleHemispherePDF() {
        return 1. / (2. * M_PI);
    }


	static void uniformSampleDisk(float u1, float u2, float *x, float *y);

    static bool sameHemisphere(const glm::vec3& wi, const glm::vec3& wo) {
        return wi[2] * wo[2] > 0.f;
    }
    
    static float absCosTheta(const glm::vec3& w) {
        return fabsf(w[2]);
    }

};
#pragma once

#include <algorithm>
#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <math.h>
#include <cmath>

#include "Utils.hpp"
#include "common.hpp"

class ONB {

public:
    Vec3 U, V, W;

    ONB() {}

    ONB(const Vec3& _N) {
        init(_N);
    }

    // from Pixar (shrug)
    void init(const Vec3 &_N) {
        W = _N;
        Real sign = copysignf(1.0f, W.z);
        const Real a = -1.0f / (sign + W.z);
        const Real b = W.x * W.y * a;
        U = Vec3(1.0f + sign * W.x * W.x * a, sign * b, -sign * W.x);
        V = Vec3(b, sign + W.y * W.y * a, -W.y);
    }

    Vec3 world2local(const Vec3& v) const {
        return Vec3(glm::dot(v, U), glm::dot(v, V), glm::dot(v, W));
    }
    

    Vec3 local2world(const Vec3& v) const {
        return Vec3(U[0]*v[0] + V[0]*v[1] + W[0]*v[2],
                        U[1]*v[0] + V[1]*v[1] + W[1]*v[2],
                        U[2]*v[0] + V[2]*v[1] + W[2]*v[2]);				
    }

    Vec3 makeVec(const Real u, const Real v, const Real w) const {
        return u*U + v*V + w*W;
    }

    Vec3 makeUnitVec(const Real u, const Real v, const Real w) const {
        return glm::normalize(u*U + v*V + w*W);
    }

    static Real cosTheta(const Vec3& w) {return w[2];}

    static Real sinTheta2(const Vec3& w) {return std::max(0., 1. - cosTheta(w)*cosTheta(w));}

    static Real sinTheta(const Vec3& w) {return sqrtf(sinTheta2(w));}

    static Real cosPhi(const Vec3& w) {
        Real sintheta = sinTheta(w);
        if (sintheta == (Real) 0) return 1;
        return glm::clamp(w[1] / sintheta, -1., 1.);
    }

    static Real sinPhi(const Vec3& w) {
        Real sintheta = sinTheta(w);
        if (sintheta == 0) return 0;
        return glm::clamp(w[1] / sintheta, -1., 1.);
    }


    static const Vec2 concentricSampleDisk(const Vec2& uv) {
        Vec2 offset = 2. * uv - Vec2(1.0);
        if (offset.x == 0 && offset.y == 0) {
            return offset;
        }
        Real theta, r;
        if (abs(offset.x) > abs(offset.y)) {
            r = offset.x;
            theta = M_PI_4 * (offset.y / offset.x);
        } else {
            r = offset.y;
            theta = M_PI_2 - M_PI_4 * (offset.x / offset.y);
        }
        return r * Vec2(cos(theta), sin(theta));
    }

    static const Vec3 cosineSampleHemisphere(const Vec2& uv) {
        Vec2 d = concentricSampleDisk(uv);
        Real z = sqrt(std::max(0., 1. - d.x*d.x -d.y*d.y));
        return Vec3(d.x, d.y, z);
    }

    static Real cosineSampleHemispherePDF(Real cosTheta) {
        return cosTheta * M_1_PI;
    }

    static Vec3 uniformSampleHemisphere(const Vec2& uv) {
        Real z = uv[0];
        Real r = sqrt(std::max(0., 1. - z*z));
        Real phi = 2. * M_PI * uv[1];
        return Vec3(r * cos(phi), r * sin(phi), z);
    }

	static Real uniformSampleHemispherePDF() {
        return 1. / (2. * M_PI);
    }
  
	static void uniformSampleDisk(Real u1, Real u2, Real *x, Real *y);

    static bool sameHemisphere(const Vec3& wi, const Vec3& wo) {
        return wi[2] * wo[2] > 0.;
    }
    
    static Real absCosTheta(const Vec3& w) {
        return std::abs(w[2]);
    }

  static Vec2 cartesianToSpherical(const Vec3& v) { // returns (phi, theta)
    Real theta = std::acos(utils::clamp(v.z, -1, 1));
    Real phi = std::atan2(v.y, v.x);
    if (phi < 0) phi += 2 * M_PI;
    return Vec2{phi, theta};
  }

  static Vec3 sphericalToCartesian(const Vec2& phiTheta) {
    Real sinTheta = std::sin(phiTheta[1]);
    Real z = std::cos(phiTheta[1]);
    Real x = sinTheta * std::cos(phiTheta[0]);
    Real y = sinTheta * std::sin(phiTheta[0]);
    return Vec3{x, y, z};
  }

        static Vec3 halfVector(const Vec3& v1, const Vec3& v2) {
            return glm::normalize(v1 + v2);
        }
};

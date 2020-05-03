#include "Utils.hpp"

namespace utils {

    float powerHeuristic(int nf, float fPDF, int ng, float gPDF) {
        float f = nf * fPDF, g = ng * gPDF;
	    return (f*f) / (f*f + g*g);
    }

    void uniformSampleTriangle(const glm::vec2& uv, glm::vec2& bg) {
        //std::cout << "uv: " << uv[0] << " " << uv[1] << std::endl;
        float sqrtu = sqrtf(uv[0]);
	    bg[0] = 1.f - sqrtu;
	    bg[1] = uv[1] * sqrtu;
    }
    float rand01() {return (float) rand() / (float) RAND_MAX;}
    glm::vec2 rand01vec2() {return glm::vec2(rand01(), rand01());}

    float avg(const glm::vec3& v) {return (v[0] + v[1] + v[2]) * (float) (1./3.);}
    Color lerp(const Color& a, const Color& b, const float t) {return (1.0f - t) * a + t * b;}
    std::string v2s(const glm::vec3& v) {
        std::stringstream ss;
        ss << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return ss.str();
    }
    Color clamp(const Color& c, const float min, const float max) {
        Color ans = c;
        for (int i = 0; i < 3; ++i) {
            ans[i] = std::max(c[i], min);
            ans[i] = std::min(c[i], max);
        }
        return ans;
    }
    float clamp(const float f, const float min, const float max) {
        if (f < min) return min;
        if (f > max) return max;
        return f;
    }
    glm::vec3 sameSide(const glm::vec3& testVec, const glm::vec3& v) {
        if (glm::dot(testVec, v) < 0)
            return -testVec;
        return testVec;
    }

}
#include <cmath>

#include "Utils.hpp"

namespace utils {
    float powerHeuristic(int nf, float fPDF, int ng, float gPDF) {
        float f = nf * fPDF, g = ng * gPDF;
	    return (f*f) / (f*f + g*g);
    }

    void uniformSampleTriangle(const glm::vec2& uv, glm::vec2& bg) {
        float sqrtu = sqrtf(uv[0]);
	    bg[0] = 1.f - sqrtu;
	    bg[1] = uv[1] * sqrtu;
    }

    glm::vec3 transformPoint(const glm::mat4 &xform, const glm::vec3 p) {

    }
    glm::vec3 transformVector(const glm::mat4 &xform, const glm::vec3 v) {

    }
    glm::vec3 transformNormal(const glm::mat4 &xform, const glm::vec3 N) {
        
    }


}
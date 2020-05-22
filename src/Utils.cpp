#include "common.hpp"
#include "Utils.hpp"

namespace utils {

    Real powerHeuristic(int nf, Real fPDF, int ng, Real gPDF) {
        Real f = (Real) nf * fPDF, g = (Real) ng * gPDF;
	    return (f*f) / (f*f + g*g);
    }

    void uniformSampleTriangle(const Vec2& uv, Vec2& bg) {
        //std::cout << "uv: " << uv[0] << " " << uv[1] << std::endl;
        Real sqrtu = sqrtf(uv[0]);
	    bg[0] = 1 - sqrtu;
	    bg[1] = uv[1] * sqrtu;
    }
    Real rand01() {return (Real) rand() / (Real) RAND_MAX;}
    Vec2 rand01vec2() {return Vec2(rand01(), rand01());}

    Real avg(const Vec3& v) {return (v[0] + v[1] + v[2]) * (Real) (1./3.);}
    Color lerp(const Color& a, const Color& b, const Real t) {return (1 - t) * a + t * b;}
    std::string v2s(const Vec3& v) {
        std::stringstream ss;
        ss << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return ss.str();
    }
    Color clamp(const Color& c, const Real min, const Real max) {
        Color ans = c;
        for (int i = 0; i < 3; ++i) {
            ans[i] = std::max(c[i], min);
            ans[i] = std::min(c[i], max);
        }
        return ans;
    }
    Real clamp(const Real f, const Real min, const Real max) {
        if (f < min) return min;
        if (f > max) return max;
        return f;
    }
    Vec3 sameSide(const Vec3& testVec, const Vec3& v) {
        if (glm::dot(testVec, v) < 0)
            return -testVec;
        return testVec;
    }
 
  void winsorize(std::vector <Color>& v, Real amt) {
    std::sort(v.begin(), v.end(), [](const Color& a, const Color& b) {
                                    return avg(a) < avg(b);
                                  });
    int numToClip = (int) (.5 * (1. - amt) * v.size());
    for (int i = 0; i < numToClip; ++i)
      v[i] = v[numToClip];
    for (int i = v.size() - 1; i > v.size() - numToClip; --i)
      v[i] = v[v.size()-numToClip];
  }
  

}

#pragma once

#include <cmath>
#include <glm/vec3.hpp>

#include "common.hpp"
#include "Color.hpp"
#include "ONB.hpp"
#include "Utils.hpp"

class BxDF {
public:
    virtual const Color f(const Vec3& wo_local, const Vec3& wi_local, bool* isSpecular) const {return Color::White();};
    virtual const void sample_f(const Vec3& wo_local, Vec3& wi_local, Color* bsdf, Real* pdf, bool* isSpecular) const {
        // default is cosine sampling
      //        *isSpecular = false;
        wi_local = ONB::cosineSampleHemisphere(utils::rand01vec2());
        if (wo_local.z < 0) wi_local.z *= -1; // flip wi if wo is in opposite hemisphere
        *pdf = this->pdf(wo_local, wi_local);
        *bsdf = this->f(wo_local, wi_local, isSpecular);
    }

    virtual const Real pdf(const Vec3& wo_local, const Vec3& wi_local) const {
        return ONB::sameHemisphere(wo_local, wi_local) ? ONB::absCosTheta(wi_local) * M_1_PI : (Real) 0;
    }

    virtual Color rho(int nSamples) const {
        Color r(0);
        for (int i = 0; i < nSamples; ++i) {
            // Estimate one term of $\rho_\roman{hh}$
            Vec3 wo_local, wi_local;
            wo_local = ONB::uniformSampleHemisphere(Vec2(utils::rand01(), utils::rand01()));
            Real pdfo = ONB::uniformSampleHemispherePDF(), pdfi = 0.;
            Color f; bool isSpecular;
            sample_f(wo_local, wi_local, &f, &pdfi, &isSpecular);
            if (pdfi > 0) {
                r += f * ONB::absCosTheta(wi_local) * ONB::absCosTheta(wo_local) / (pdfo * pdfi);
            }
        }
        return r / ((Real) M_PI * (Real) nSamples);
    }

    static Real fresnelDielectric(Real cosTheta, Real etaI, Real etaT);
    static Real fresnelConductor(Real cosTheta, Real etaI, Real etaT);
    static Real Schlick(Real cosThetaI, Real etaI, Real etaT);

};

class Lambertian_BRDF : public BxDF {
public:

    Lambertian_BRDF(const Color& _color) : color (_color) {}

    const Color f(const Vec3& wo_local, const Vec3& wi_local, bool* isSpecular) const {
        *isSpecular = false;
        return color * (Real) M_1_PI;
    }
    Color rho(int nSamples) const {return color;}

    const Color color;
};

class GGX_BRDF : public BxDF {

};

class GGX_BTDF : public BxDF {

};


class OrenNayar_BRDF : public BxDF {
    //const Color f(const Vec3& wo_local, const Vec3& wi_local, bool* isSpecular) const;
};

class SpecularDielectric_BRDF : public BxDF {
public:
    SpecularDielectric_BRDF(const Color& _color, const Real _etaI, const Real _etaT) : etaI(_etaI), etaT(_etaT), color (_color) {}
    const Color f(const Vec3& wo_local, const Vec3& wi_local, bool* isSpecular) const {return Color::Black();}
    const Real pdf(const Vec3& wo_local, const Vec3& wi_local) const {return 0.;}
    const void sample_f(const Vec3& wo_local, Vec3& wi_local, Color* bsdf, Real* pdf, bool* isSpecular) const;
private:
    Real etaI, etaT;
    Color color;
};

class SpecularDielectric_BTDF : public BxDF {
public:
    SpecularDielectric_BTDF(const Color& _color, const Real _etaA, const Real _etaB) : etaA(_etaA), etaB(_etaB), color (_color) {}
    const Color f(const Vec3& wo_local, const Vec3& wi_local, bool* isSpecular) const {return Color::Black();}
    const Real pdf(const Vec3& wo_local, const Vec3& wi_local) const {return 0.;}
    const void sample_f(const Vec3& wo_local, Vec3& wi_local, Color* bsdf, Real* pdf, bool* isSpecular) const;

private:
    Real etaA, etaB; // A above surface (same side as normal points)
    Color color;
};

class Dielectric_BSDF : public BxDF {
public:
  Dielectric_BSDF(const Color& _R, const Color& _T, const Real _etaA, const Real _etaB) : etaA(_etaA), etaB(_etaB), R (_R), T (_T) {}
    const Color f(const Vec3& wo_local, const Vec3& wi_local, bool* isSpecular) const {
      *isSpecular = true;
      return Color::Black();
    }
    const Real pdf(const Vec3& wo_local, const Vec3& wi_local) const {return 0.;}
    const void sample_f(const Vec3& wo_local, Vec3& wi_local, Color* bsdf, Real* pdf, bool* isSpecular) const;

private:
    Real etaA, etaB; // A above surface (same side as normal points)
  Color R, T;
};

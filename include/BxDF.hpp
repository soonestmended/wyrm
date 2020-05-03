#pragma once

#include <cmath>
#include <glm/vec3.hpp>

#include "common.hpp"
#include "Color.hpp"
#include "ONB.hpp"
#include "Utils.hpp"

class BxDF {
public:
    virtual const Color f(const glm::vec3& wo_local, const glm::vec3& wi_local, bool* isSpecular) const {return Color::White();};
    virtual const void sample_f(const glm::vec3& wo_local, glm::vec3& wi_local, Color* bsdf, float* pdf, bool* isSpecular) const {
        // default is cosine sampling
        wi_local = ONB::cosineSampleHemisphere(utils::rand01vec2());
        if (wo_local.z < 0) wi_local.z *= -1; // flip wi if wo is in opposite hemisphere
        *pdf = this->pdf(wo_local, wi_local);
        *bsdf = this->f(wo_local, wi_local, isSpecular);
    }

    virtual const float pdf(const glm::vec3& wo_local, const glm::vec3& wi_local) const {
        return ONB::sameHemisphere(wo_local, wi_local) ? ONB::absCosTheta(wi_local) * M_1_PI : 0.f;
    }

    virtual Color rho(int nSamples) const {
        Color r(0.f);
        for (int i = 0; i < nSamples; ++i) {
            // Estimate one term of $\rho_\roman{hh}$
            glm::vec3 wo_local, wi_local;
            wo_local = ONB::uniformSampleHemisphere(glm::vec2(utils::rand01(), utils::rand01()));
            float pdfo = ONB::uniformSampleHemispherePDF(), pdfi = 0.;
            Color f; bool isSpecular;
            sample_f(wo_local, wi_local, &f, &pdfi, &isSpecular);
            if (pdfi > 0) {
                r += f * ONB::absCosTheta(wi_local) * ONB::absCosTheta(wo_local) / (pdfo * pdfi);
            }
        }
        return r / ((float) M_PI * (float) nSamples);
    }

    static float fresnelDielectric(float cosTheta, float etaI, float etaT);
    static float fresnelConductor(float cosTheta, float etaI, float etaT);
    static float Schlick(float cosThetaI, float etaI, float etaT);

};

class Lambertian_BRDF : public BxDF {
public:

    Lambertian_BRDF(const Color& _color) : color (_color) {}

    const Color f(const glm::vec3& wo_local, const glm::vec3& wi_local, bool* isSpecular) const {
        *isSpecular = false;
        return color * (float) M_1_PI;
    }
    Color rho(int nSamples) const {return color * (float) M_1_PI;}

    const Color color;
};

class GGX_BRDF : public BxDF {

};

class GGX_BTDF : public BxDF {

};


class OrenNayar_BRDF : public BxDF {
    //const Color f(const glm::vec3& wo_local, const glm::vec3& wi_local, bool* isSpecular) const;
};

class SpecularDielectric_BRDF : public BxDF {
public:
    SpecularDielectric_BRDF(const Color& _color, const float _etaI, const float _etaT) : etaI(_etaI), etaT(_etaT), color (_color) {}
    const Color f(const glm::vec3& wo_local, const glm::vec3& wi_local, bool* isSpecular) const {return Color::Black();}
    const float pdf(const glm::vec3& wo_local, const glm::vec3& wi_local) const {return 0.;}
    const void sample_f(const glm::vec3& wo_local, glm::vec3& wi_local, Color* bsdf, float* pdf, bool* isSpecular) const;
private:
    float etaI, etaT;
    Color color;
};

class SpecularDielectric_BTDF : public BxDF {
public:
    SpecularDielectric_BTDF(const Color& _color, const float _etaA, const float _etaB) : etaA(_etaA), etaB(_etaB), color (_color) {}
    const Color f(const glm::vec3& wo_local, const glm::vec3& wi_local, bool* isSpecular) const {return Color::Black();}
    const float pdf(const glm::vec3& wo_local, const glm::vec3& wi_local) const {return 0.;}
    const void sample_f(const glm::vec3& wo_local, glm::vec3& wi_local, Color* bsdf, float* pdf, bool* isSpecular) const;

private:
    float etaA, etaB; // A above surface (same side as normal points)
    Color color;
};

class Dielectric_BSDF : public BxDF {
public:
    Dielectric_BSDF(const Color& _color, const float _etaA, const float _etaB) : etaA(_etaA), etaB(_etaB), color (_color) {}
    const Color f(const glm::vec3& wo_local, const glm::vec3& wi_local, bool* isSpecular) const {return Color::Black();}
    const float pdf(const glm::vec3& wo_local, const glm::vec3& wi_local) const {return 0.;}
    const void sample_f(const glm::vec3& wo_local, glm::vec3& wi_local, Color* bsdf, float* pdf, bool* isSpecular) const;

private:
    float etaA, etaB; // A above surface (same side as normal points)
    Color color;
};
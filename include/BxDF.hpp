#pragma once

#include <glm/vec3.hpp>

#include "common.hpp"
#include "Color.hpp"
#include "Material.hpp"
#include "ONB.hpp"
#include "Utils.hpp"

class BxDF {
public:
    virtual const Color f(const glm::vec3& wo_local, const glm::vec3& wi_local, bool* isSpecular) const;
    virtual const void sample_f(const glm::vec3& wo_local, glm::vec3& wi_local, Color* bsdf, float* pdf, bool* isSpecular) const {
        // default is cosine sampling
        wi_local = utils::cosineSampleHemisphere(utils::rand01vec2());
        if (wo_local.z < 0) wi_local.z *= -1; // flip wi if wo is in opposite hemisphere
        *pdf = this->pdf(wo_local, wi_local);
        *bsdf = this->f(wo_local, wi_local, isSpecular);
    }

    virtual const float pdf(const glm::vec3& wo_local, const glm::vec3& wi_local) const {
        return ONB::sameHemisphere(wo_local, wi_local) ? ONB::absCosTheta(wi_local) * M_INV_PI : 0.f;
    }
    
    virtual Color BxDF::rho(int nSamples) const {
        Color r(0.f);
        for (int i = 0; i < nSamples; ++i) {
            // Estimate one term of $\rho_\roman{hh}$
            glm::vec3 wo_local, wi_local;
            wo_local = ONB::uniformSampleHemisphere(utils::rand01(), utils::rand01());
            float pdfo = ONB::uniformSampleHemispherePDF(), pdfi = 0.;
            Color f; bool isSpecular;
            sample_f(wo_local, wi_local, &f, &pdfi, &isSpecular);
            if (pdfi > 0) {
                r += f * ONB::absCosTheta(wi_local) * ONB::absCosTheta(wo_local) / (pdfo * pdfi);
            }
        }
        return r / ((float) M_PI * (float) nSamples);
    }
};

class Lambertian_BRDF : public BxDF {
public:

    Lambertian_BRDF(const Color& _color) : color (_color) {}

    const Color f(const glm::vec3& wo_local, const glm::vec3& wi_local, bool* isSpecular) const {return color * (float) M_INV_PI;}
    Color rho(int nSamples) const {return color * (float) M_INV_PI;}

    const Color color;
};

class GGX_BRDF : public BxDF {

};

class GGX_BTDF : public BxDF {

};


class OrenNayar_BRDF : public BxDF {

};

class PureSpecular_BRDF : public BxDF {

};

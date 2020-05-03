#include <algorithm>
#include <glm/vec3.hpp>
#include <iostream>
#include "BxDF.hpp"
#include "common.hpp"

using namespace std;

float BxDF::fresnelDielectric(float cosThetaI, float etaI, float etaT) {
    cosThetaI = utils::clamp(cosThetaI, -1.f, 1.f);
    bool entering = cosThetaI > 0.f;
    if (!entering) {
        std::swap(etaI, etaT);
        cosThetaI = std::abs(cosThetaI);
    }

    float sinThetaI = std::sqrt(std::max(0.f, 1.f - cosThetaI * cosThetaI));
    float sinThetaT = etaI / etaT * sinThetaI;

    if (sinThetaT >= 1)
        return 1;
    float cosThetaT = std::sqrt(std::max((float)0, 1.f - sinThetaT * sinThetaT));

    float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
                  ((etaT * cosThetaI) + (etaI * cosThetaT));
    float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
                  ((etaI * cosThetaI) + (etaT * cosThetaT));
    return (Rparl * Rparl + Rperp * Rperp) / 2.f;
    //return .5;
}

float BxDF::Schlick(float cosThetaI, float etaI, float etaT) {
    cosThetaI = utils::clamp(cosThetaI, -1.f, 1.f);
    bool entering = cosThetaI > 0.f;
    if (!entering) {
        std::swap(etaI, etaT);
        cosThetaI = std::abs(cosThetaI);
    }
    float omc = 1.f - cosThetaI;
    float R0 = (etaI - etaT) / (etaI + etaT);
    R0 *= R0;
    return R0 + (1.f - R0) * omc * omc * omc * omc * omc;
}

inline bool refract(glm::vec3& wt, const glm::vec3& wi, const glm::vec3& n, float eta) {
    float cosThetaI = glm::dot(n, wi);
    float sin2ThetaI = std::max(0.f, 1.f - cosThetaI * cosThetaI);
    float sin2ThetaT = eta * eta * sin2ThetaI;
    if (sin2ThetaT >= 1) return false;

    float cosThetaT = std::sqrt(1.f - sin2ThetaT);

    wt = eta * -wi + (eta * cosThetaI - cosThetaT) * n;
    return true;
}

const void SpecularDielectric_BRDF::sample_f(const glm::vec3& wo_local, glm::vec3& wi_local, Color* bsdf, float* pdf, bool* isSpecular) const {
    *isSpecular = true;
    wi_local = glm::vec3{-wo_local.x, -wo_local.y, wo_local.z};
    *pdf = 1.f;
    *bsdf = color * Schlick(ONB::cosTheta(wo_local), etaI, etaT);
}

const void SpecularDielectric_BTDF::sample_f(const glm::vec3& wo_local, glm::vec3& wi_local, Color* bsdf, float* pdf, bool* isSpecular) const {
    *isSpecular = true;
    bool entering = ONB::cosTheta(wo_local) > 0;
    float etaI = entering ? etaA : etaB;
    float etaT = entering ? etaB : etaA;
    if (!refract(wi_local, wo_local, utils::sameSide(glm::vec3(0, 0, 1), wo_local), etaI / etaT)) {
        *bsdf = Color::Black();
        return;
    }
    *pdf = 1;
    *isSpecular = true;
    *bsdf = color * (1.0f - BxDF::Schlick(ONB::cosTheta(wo_local), etaI, etaT)) / ONB::absCosTheta(wi_local);
}
const void Dielectric_BSDF::sample_f(const glm::vec3& wo_local, glm::vec3& wi_local, Color* bsdf, float* pdf, bool* isSpecular) const {

    float fresnel = BxDF::Schlick(ONB::cosTheta(wo_local), etaA, etaB);
    *isSpecular = true;
    //cout << fresnel << " ";
    if (utils::rand01() < fresnel) {
        // do reflection
        wi_local = glm::vec3{-wo_local.x, -wo_local.y, wo_local.z};
        *pdf = fresnel;
        *bsdf = color * fresnel;
#ifdef DEBUG
        cout << "Reflected ray: ";
#endif
    }
    else {
        bool entering = ONB::cosTheta(wo_local) > 0;
        float etaI = entering ? etaA : etaB;
        float etaT = entering ? etaB : etaA;
        if (!refract(wi_local, wo_local, utils::sameSide(glm::vec3(0, 0, 1), wo_local), etaI / etaT)) {
            *bsdf = Color::Black();
            return;
        }
        //cout << "wi_local " << (Color) wi_local << endl;
        *pdf = 1.f - fresnel;
        *bsdf = color * (1.0f - fresnel) / ONB::absCosTheta(wi_local);

        //cout << "bsdf: " << *bsdf << endl;
#ifdef DEBUG
        cout << "Refracted ray: ";
#endif

    }    
}


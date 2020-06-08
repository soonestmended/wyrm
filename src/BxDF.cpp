#include <algorithm>
#include <glm/vec3.hpp>
#include <iostream>
#include "BxDF.hpp"
#include "common.hpp"

using namespace std;

Real BxDF::fresnelDielectric(Real cosThetaI, Real etaI, Real etaT) {
    cosThetaI = utils::clamp(cosThetaI, -1, 1);
    bool entering = cosThetaI > 0;
    if (!entering) {
        std::swap(etaI, etaT);
        cosThetaI = std::abs(cosThetaI);
    }

    Real sinThetaI = std::sqrt(std::max((Real) 0, 1 - cosThetaI * cosThetaI));
    Real sinThetaT = etaI / etaT * sinThetaI;

    if (sinThetaT >= 1)
        return 1;
    Real cosThetaT = std::sqrt(std::max((Real)0, 1. - sinThetaT * sinThetaT));

    Real Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
                  ((etaT * cosThetaI) + (etaI * cosThetaT));
    Real Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
                  ((etaI * cosThetaI) + (etaT * cosThetaT));
    return (Rparl * Rparl + Rperp * Rperp) / 2.;
    //return .5;
}

Real BxDF::Schlick(Real cosThetaI, Real etaI, Real etaT) {
    cosThetaI = utils::clamp(cosThetaI, -1, 1);
    bool entering = cosThetaI > 0;
    if (!entering) {
        std::swap(etaI, etaT);
        cosThetaI = std::abs(cosThetaI);
    }
    Real omc = 1. - cosThetaI;
    Real R0 = (etaI - etaT) / (etaI + etaT);
    R0 *= R0;
    return R0 + (1. - R0) * omc * omc * omc * omc * omc;
}

inline bool refract(Vec3& wt, const Vec3& wi, const Vec3& n, Real eta) {
    Real cosThetaI = glm::dot(n, wi);
    Real sin2ThetaI = std::max((Real) 0, 1 - cosThetaI * cosThetaI);
    Real sin2ThetaT = eta * eta * sin2ThetaI;
    if (sin2ThetaT >= 1) return false;

    Real cosThetaT = std::sqrt(1 - sin2ThetaT);

    wt = eta * -wi + (eta * cosThetaI - cosThetaT) * n;
    return true;
}

void SpecularDielectric_BRDF::sample_f(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, Color* bsdf, Real* pdf, bool* isSpecular) const {
    *isSpecular = true;
    wi_local = Vec3{-wo_local.x, -wo_local.y, wo_local.z};
    *pdf = 1;
    *bsdf = R(ir) * Schlick(ONB::cosTheta(wi_local), etaI, etaT);
}

void SpecularDielectric_BTDF::sample_f(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, const Vec2& uv, Color* bsdf, Real* pdf, bool* isSpecular) const {
    *isSpecular = true;
    bool entering = ONB::cosTheta(wo_local) > 0;
    Real etaI = entering ? etaA : etaB;
    Real etaT = entering ? etaB : etaA;
    if (!refract(wi_local, wo_local, utils::sameSide(Vec3(0, 0, 1), wo_local), etaI / etaT)) {
        *bsdf = Color::Black();
        return;
    }
    *pdf = 1;
    *isSpecular = true;
    // changed wo_local to wi_local in the line below
    *bsdf = T(ir) * (1. - BxDF::Schlick(ONB::cosTheta(wi_local), etaI, etaT)) / ONB::absCosTheta(wi_local);
}
void Dielectric_BSDF::sample_f(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, const Vec2& uv, Color* bsdf, Real* pdf, bool* isSpecular) const {

    Real fresnel = BxDF::Schlick(ONB::cosTheta(wo_local), etaA, etaB);
    *isSpecular = true;
    //cout << fresnel << " ";
    if (uv[0] < fresnel) {
        // do reflection
        wi_local = Vec3{-wo_local.x, -wo_local.y, wo_local.z};
        *pdf = fresnel;
        *bsdf = R(ir) * fresnel / ONB::absCosTheta(wi_local);
        //        *bsdf = Color::Black();
#ifdef DEBUG
        cout << "Reflected ray: ";
#endif
    }
    else {
        bool entering = ONB::cosTheta(wo_local) > 0;
        Real etaI = entering ? etaA : etaB;
        Real etaT = entering ? etaB : etaA;
        if (!refract(wi_local, wo_local, utils::sameSide(Vec3(0, 0, 1), wo_local), etaI / etaT)) {
            *bsdf = Color::Black();
            return;
        }
        //cout << "wi_local " << (Color) wi_local << endl;
        *pdf = 1 - fresnel;
        *bsdf = T(ir) * (1 - fresnel) / ONB::absCosTheta(wi_local);
        //*bsdf = Color::Black();
        //cout << "bsdf: " << *bsdf << endl;
#ifdef DEBUG
        cout << "Refracted ray: ";
#endif

    }    
}


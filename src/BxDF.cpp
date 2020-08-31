#include <algorithm>
#include <glm/vec3.hpp>
#include <iostream>
#include <typeinfo>
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
    // Do I need to account for total internal refraction here?
   
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

Color SchlickFunction::eval(Real cosThetaI, const Color& R0, const Real etaI, const Real etaT) const {
    cosThetaI = utils::clamp(cosThetaI, -1, 1);
//    bool entering = cosThetaI > 0;
//    if (!entering) {
//        std::swap(etaI, etaT);
        cosThetaI = std::abs(cosThetaI);
//    }
    Real omc = 1. - cosThetaI;
//    Real R0 = (etaI - etaT) / (etaI + etaT);
    return R0 * R0 + (Color{1.} - R0) * omc * omc * omc * omc * omc;
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

Real GGXDistribution::D(const Vec3& m_local, const IntersectRec& ir) const {
    Real alpha = texAlpha->eval(ir.tc);
    Real aSq  = alpha * alpha;
    if (m_local.z <= 0) return 0;
    Real cosThetaM = ONB::cosTheta(m_local);
    Real tanThetaM = tan(acos(cosThetaM));
    Real parens = aSq + tanThetaM * tanThetaM;
    Real denominator = M_PI * cosThetaM * cosThetaM * cosThetaM * cosThetaM * parens * parens;
    return aSq / denominator;
}
    //void sample_D(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, Real* pdf) const;

Real GGXDistribution::G(const Vec3& m_local, const Vec3& v_local, const IntersectRec& ir) const {
    if (glm::dot(v_local, m_local) / ONB::cosTheta(v_local) <= 0) return 0;
    Real alpha = texAlpha->eval(ir.tc);
    Real  tanThetaV = tan(acos(ONB::cosTheta(v_local)));
    return 2 / (1 + sqrt(1 + alpha*alpha*tanThetaV*tanThetaV));
}

void Microfacet_BRDF::init(const string& fresnel_name, const string& dist_name) {
    if (fresnel_name == "schlick") {
        fresnel = make_shared <SchlickFunction> ();
    }

    if (dist_name == "beckmann") {
        mfd = make_shared <BeckmannDistribution> (texAlpha);
    }
    else if (dist_name == "phong") {
        mfd = make_shared <PhongDistribution> (texAlpha);
    }
    else {
        mfd = make_shared <GGXDistribution> (texAlpha);
    }
}

Microfacet_BRDF::Microfacet_BRDF(const std::shared_ptr <Texture <Color>>& _R0, const std::string& fresnel_name, const std::string& dist_name, const std::shared_ptr <Texture <Real>>& _alpha) :
    R0 (_R0), texAlpha (_alpha) {
    init(fresnel_name, dist_name);
}

Microfacet_BRDF::Microfacet_BRDF(const std::shared_ptr <Texture <Color>>& _texEta, const std::shared_ptr <Texture <Color>>& _texK, const std::string& fresnel_name, const std::string& dist_name, const std::shared_ptr <Texture <Real>>& _alpha) :
    texEta (_texEta), texK (_texK), texAlpha (_alpha) {
    Texture <Color>* te = texEta.get();
    Texture <Color>* tk = texK.get();
    ConstantTexture <Color>* cte = dynamic_cast <ConstantTexture<Color>*> (te);
    ConstantTexture <Color>* ctk = dynamic_cast <ConstantTexture<Color>*> (tk);

    if (cte && ctk) {
        // if both eta and k textures are constant, we can compute R0 and discard texEta / texK
        Color etaT = cte->eval(Vec3(0));
        Color k = ctk->eval(Vec3(0));
        Color num = etaT - Color(1.00029);
        num = num*num + k*k;
        Color denom = etaT + Color(1.00029);
        denom = denom*denom + k*k;

        R0 = make_shared <ConstantTexture <Color>> (num / denom);
        texEta = texK = nullptr;
    }
    init(fresnel_name, dist_name);
}

//Microfacet_BRDF(const std::string& metal_name, const std::string& texture_name);

/*
Microfacet_BRDF::Microfacet_BRDF(const std::shared_ptr <Texture <Color>>& _texColor, const std::string& fresnel_name, const std::string& dist_name, const std::shared_ptr <Texture <Real>>& _texAlpha, const std::shared_ptr <Texture <Color>>& _texEta) :
    texColor (_texColor), texAlpha(_texAlpha), texEta (_texEta) {
    if (fresnel_name == "schlick") {
        fresnel = make_shared <SchlickFunction> ();
    }
    if (dist_name == "beckmann") {
        mfd = make_shared <BeckmannDistribution> (_texAlpha);
    }
    else if (dist_name == "phong") {
        mfd = make_shared <PhongDistribution> (_texAlpha);
    }
    else {
        mfd = make_shared <GGXDistribution> (_texAlpha);
    }

}
*/


const Color Microfacet_BRDF::f(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir, bool* isSpecular) const {
    // NOTE : need to deal with how metal reflectances are specified.
    // I think the parser should take _either_ separate textures for eta and k OR a texture for R0 directly.
    *isSpecular = false;
    Real cosThetaI = ONB::cosTheta(wi_local);
    Real cosThetaO = ONB::cosTheta(wo_local);
    //bool entering = cosThetaO > 0;
    //Real etaB = texEta->eval(ir.tc);
    //Real etaI = entering ? etaA : etaB;
    //Real etaT = entering ? etaB : etaA;
    if (cosThetaI == 0 || cosThetaO == 0) return Color{0};
    Vec3 m_local = ONB::halfVector(wo_local, wi_local);
    //Color k = texColor->eval(ir.tc);
    //Color n =
    Color F = fresnel->eval(cosThetaI, R0->eval(ir.tc), 1, 1); // ones are dummy arguments here, I guess??
    Real D = mfd->D(m_local, ir);
    Real G = mfd->G(m_local, wi_local, ir) * mfd->G(m_local, wo_local, ir);
    Color ans = (F * D * G) / (4. * cosThetaI * cosThetaO);
  /*
    if (ans.luminance() > 1.0) {
        cout << "High microfacet f: " << ans << endl;
        cout << "\tF: " << F << endl;
        cout << "\tD: " << D << endl;
        cout << "\tG: " << G << endl;
        cout << "\tcosThetaI: " << cosThetaI << endl;
        cout << "\tcosThetaO: " << cosThetaO << endl;
   }
*/

    return ans;
}

/*
Real Microfacet_BRDF::pdf(const Vec3& wo_local, const Vec3& wi_local,  const IntersectRec& ir) const {
   
}

void Microfacet_BRDF::sample_f(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, Color* bsdf, Real* pdf, bool* isSpecular) const {

}
*/
Microfacet_BTDF::Microfacet_BTDF(const shared_ptr <Texture <Color>>& _R0, const std::string& fresnel_name, const std::string& dist_name, const std::shared_ptr <Texture <Real>>& _texAlpha, const std::shared_ptr <Texture <Real>>& _texEta) :
    R0 (_R0), texAlpha(_texAlpha),  texEta (_texEta) {
    if (fresnel_name == "schlick") {
        fresnel = make_shared <SchlickFunction> ();
    }
    if (dist_name == "beckmann") {
        mfd = make_shared <BeckmannDistribution> (_texAlpha);
    }
    else if (dist_name == "phong") {
        mfd = make_shared <PhongDistribution> (_texAlpha);
    }
    else {
        mfd = make_shared <GGXDistribution> (_texAlpha);
    }

}

const Color Microfacet_BTDF::f(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir, bool* isSpecular) const {
    if (ONB::sameHemisphere(wo_local, wi_local)) return Color{0};
    bool entering = ONB::cosTheta(wo_local) > 0;
    Real etaB = texEta->eval(ir.tc);
    Real etaI = entering ? etaA : etaB;
    Real etaO = entering ? etaB : etaA;
    Vec3 ht = -glm::normalize(etaI * wi_local + etaO * wo_local);
    Real iDotHt = glm::dot(wi_local, ht);
    Real iDotN = ONB::cosTheta(wi_local);
    Real oDotHt = glm::dot(wo_local, ht);
    Real oDotN = ONB::cosTheta(wo_local);
    Real term1 = abs((iDotHt * oDotHt) / (iDotN * oDotN));
    Color F = fresnel->eval(iDotN, R0->eval(ir.tc), etaI, etaO);
    Color numerator = etaO * etaO * (Color{1} - F) * mfd->G(ht, wi_local, ir) * mfd->D(ht, ir);
    Real denominator = etaI * iDotHt + etaO * oDotHt;
    denominator *= denominator;
    return term1 * numerator / denominator;
}

#pragma once

#include <cmath>
#include <glm/vec3.hpp>

#include "common.hpp"
#include "Color.hpp"
#include "IntersectRec.hpp"
#include "ONB.hpp"
#include "SampleGenerator.hpp"
#include "Utils.hpp"
#include "Texture.hpp"

class BxDF {
public:
  virtual const Color f(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir, bool* isSpecular) const {return Color::White();};
  virtual void sample_f(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, const Vec2& uv, Color* bsdf, Real* pdf, bool* isSpecular) const {
        // default is cosine sampling
      //        *isSpecular = false;
    wi_local = ONB::cosineSampleHemisphere(uv);
    if (wo_local.z < 0) wi_local.z *= -1; // flip wi if wo is in opposite hemisphere
    *pdf = this->pdf(wo_local, wi_local, ir);
    *bsdf = this->f(wo_local, wi_local, ir, isSpecular);
  }
  
  virtual Real pdf(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir) const {
    return ONB::sameHemisphere(wo_local, wi_local) ? ONB::absCosTheta(wi_local) * M_1_PI : (Real) 0;
  }

  virtual Color rho(int nSamples, SampleGenerator* sg,  const IntersectRec& ir) const {
    Color r(0);
    for (int i = 0; i < nSamples; ++i) {
      // Estimate one term of $\rho_\roman{hh}$

      Vec3 wo_local, wi_local;
      wo_local = ONB::uniformSampleHemisphere(sg->next());
      Real pdfo = ONB::uniformSampleHemispherePDF(), pdfi = 0.;
      Color f; bool isSpecular;
      sample_f(wo_local, wi_local, ir, sg->next(), &f, &pdfi, &isSpecular);
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

  Lambertian_BRDF(const std::shared_ptr <Texture <Color>>& _texKd) : texKd (_texKd) {}

  const Color f(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir, bool* isSpecular) const {
    *isSpecular = false;
    return R(ir) * (Real) M_1_PI;
  }
  Color rho(int nSamples, const SampleGenerator* sg, const IntersectRec& ir) const {return R(ir);}
  virtual Color R(const IntersectRec& ir) const {return texKd->eval(ir.tc);}

protected:
  const std::shared_ptr <Texture<Color>> texKd;
};

class FresnelFunction {
  public:
    virtual Color eval(Real cosThetaI, const Color& R0, const Real etaI, const Real etaT) const {return R0;}
};

class SchlickFunction : public FresnelFunction {
  public:
    Color eval(Real cosThetaI, const Color& R0, const Real etaI, const Real etaT) const;
};

class MicrofacetDistribution {
  public:
    MicrofacetDistribution(const std::shared_ptr <Texture <Real>>& _texAlpha) : texAlpha (_texAlpha) {}
    virtual Real D(const Vec3& m_local, const IntersectRec& ir) const {return 1;}
    virtual void sample_D(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, Real* pdf) const {}
    virtual Real G(const Vec3& m_local, const Vec3& wi_local, const IntersectRec& ir) const {return 1;}
    virtual Real pdf_D(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir) const {return 1;}

    const std::shared_ptr <Texture <Real>> texAlpha;

};

class BeckmannDistribution : public MicrofacetDistribution {
  public:
    BeckmannDistribution(const std::shared_ptr <Texture <Real>>& _alpha) : MicrofacetDistribution(_alpha) {}
    //Real D(const Vec3& m_local) const;
    // void sample_D(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, Real* pdf) const;
    //Real G(const Vec3& m_local, const Vec3& wi_local) const;
    // Real pdf_D(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir) const;
};

class PhongDistribution : public MicrofacetDistribution {
  public:
    PhongDistribution(const std::shared_ptr <Texture <Real>>& _alpha) : MicrofacetDistribution(_alpha) {}
    //Real D(const Vec3& m_local) const;
    //void sample_D(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, Real* pdf) const;
    //Real G(const Vec3& m_local, const Vec3& wi_local) const;
    //Real pdf_D(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir) const;
};

class GGXDistribution : public MicrofacetDistribution {
    public:
    GGXDistribution(const std::shared_ptr <Texture <Real>>& _alpha) : MicrofacetDistribution(_alpha) {}
    Real D(const Vec3& m_local, const IntersectRec& ir) const;
    //void sample_D(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, Real* pdf) const;
    Real G(const Vec3& m_local, const Vec3& wi_local, const IntersectRec& ir) const;
    //Real pdf_D(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir) const;
};

class Microfacet_BRDF : public BxDF {
  public:
    Microfacet_BRDF(const std::shared_ptr <Texture <Color>>& R0, const std::string& fresnel_name, const std::string& dist_name, const std::shared_ptr <Texture <Real>>& _alpha);
    Microfacet_BRDF(const std::shared_ptr <Texture <Color>>& texEta, const std::shared_ptr <Texture <Color>>& texK, const std::string& fresnel_name, const std::string& dist_name, const std::shared_ptr <Texture <Real>>& _alpha);
//    Microfacet_BRDF(const std::string& metal_name, const std::string& texture_name);
    // add etas to constructor? Could compute R0 in constructor & pass to fast Schlick for use with microfacet dielectrics
    const Color f(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir, bool* isSpecular) const;
//    Real pdf(const Vec3& wo_local, const Vec3& wi_local,  const IntersectRec& ir) const;
//    void sample_f(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, Color* bsdf, Real* pdf, bool* isSpecular) const;

  protected:
    Real etaA = 1.00029;
    std::shared_ptr <Texture <Color>> R0 = nullptr;
    const std::shared_ptr <Texture <Real>> texAlpha = nullptr;
    std::shared_ptr <Texture <Color>> texEta = nullptr;
    std::shared_ptr <Texture <Color>> texK = nullptr;
    std::shared_ptr <FresnelFunction> fresnel;
    std::shared_ptr <MicrofacetDistribution> mfd;
    void init(const std::string& fresnel_name, const std::string& dist_name);
};

class Microfacet_BTDF : public BxDF {
  public:
    Microfacet_BTDF(const std::shared_ptr <Texture <Color>>& R0, const std::string& fresnel_name, const std::string& dist_name, const std::shared_ptr <Texture <Real>>& _alpha, const std::shared_ptr <Texture <Real>>& _texEta = nullptr);

    // add etas to constructor? Could compute R0 in constructor & pass to fast Schlick for use with microfacet dielectrics
    const Color f(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir, bool* isSpecular) const;
    // Real pdf(const Vec3& wo_local, const Vec3& wi_local,  const IntersectRec& ir) const;
    // void sample_f(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, Color* bsdf, Real* pdf, bool* isSpecular) const;

  protected:
     Real etaA = 1.00029;
    std::shared_ptr <Texture <Color>> R0;
    std::shared_ptr <Texture <Real>> texAlpha;
    std::shared_ptr <Texture <Real>> texEta;
    // const std::shared_ptr <Texture <Color>> texK;
    std::shared_ptr <FresnelFunction> fresnel;
    std::shared_ptr <MicrofacetDistribution> mfd;

};

class OrenNayar_BRDF : public BxDF {
  //const Color f(const Vec3& wo_local, const Vec3& wi_local, bool* isSpecular) const;

};

class SpecularDielectric_BRDF : public BxDF {
public:
  SpecularDielectric_BRDF(const std::shared_ptr <Texture <Color>>& _tex, const Real _etaI, const Real _etaT) : etaI(_etaI), etaT(_etaT), tex (_tex) {}
  const Color f(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir, bool* isSpecular) const {return Color::Black();}
  Real pdf(const Vec3& wo_local, const Vec3& wi_local,  const IntersectRec& ir) const {return 0.;}
  void sample_f(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, Color* bsdf, Real* pdf, bool* isSpecular) const;
	virtual Color R(const IntersectRec& ir) const {return tex->eval(ir.tc);}

protected:
  Real etaI, etaT;
  const std::shared_ptr <Texture<Color>> tex;
};

class SpecularDielectric_BTDF : public BxDF {
public:
  SpecularDielectric_BTDF(const std::shared_ptr <Texture <Color>>& _tex, const Real _etaA, const Real _etaB) : etaA(_etaA), etaB(_etaB), tex (_tex) {}
  const Color f(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir, bool* isSpecular) const {return Color::Black();}
  Real pdf(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir) const {return 0.;}
  void sample_f(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, const Vec2& uv, Color* bsdf, Real* pdf, bool* isSpecular) const;
	virtual Color T(const IntersectRec& ir) const {return tex->eval(ir.tc);}
private:
  Real etaA, etaB; // A above surface (same side as normal points)
  std::shared_ptr <Texture <Color>> tex;
};

class Dielectric_BSDF : public BxDF {
public:
  Dielectric_BSDF(const std::shared_ptr <Texture <Color>>& _texR, const std::shared_ptr <Texture <Color>>& _texT, const Real _etaA, const Real _etaB) : etaA(_etaA), etaB(_etaB), texR (_texR), texT (_texT) {}
  const Color f(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir, bool* isSpecular) const {
    *isSpecular = true;
    return Color::Black();
  }
  Real pdf(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir) const {return 0.;}
  void sample_f(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, const Vec2& uv, Color* bsdf, Real* pdf, bool* isSpecular) const;
  virtual Color R(const IntersectRec& ir) const {return texR->eval(ir.tc);}
  virtual Color T(const IntersectRec& ir) const {return texT->eval(ir.tc);}

protected:
  Real etaA, etaB; // A above surface (same side as normal points)
  std::shared_ptr <Texture <Color>> texR, texT;
};

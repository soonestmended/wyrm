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

  Lambertian_BRDF(const Color& _color) : color (_color) {}

  const Color f(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir, bool* isSpecular) const {
    *isSpecular = false;
    return R(ir) * (Real) M_1_PI;
  }
  Color rho(int nSamples, const SampleGenerator* sg, const IntersectRec& ir) const {return R(ir);}
  virtual Color R(const IntersectRec& ir) const {return color;} 

protected:
  const Color color;
};

class TexturedLambertian_BRDF : public Lambertian_BRDF {
public:
	TexturedLambertian_BRDF(const std::shared_ptr <Texture>& _texture) : Lambertian_BRDF(Color::White()), texture (_texture) {}
	Color R(const IntersectRec& ir) const {
		return texture->eval(ir.uvw[0], ir.uvw[1], ir.uvw[2]);
	}
	std::shared_ptr <Texture> texture;
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
  const Color f(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir, bool* isSpecular) const {return Color::Black();}
  Real pdf(const Vec3& wo_local, const Vec3& wi_local,  const IntersectRec& ir) const {return 0.;}
  void sample_f(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, Color* bsdf, Real* pdf, bool* isSpecular) const;
	virtual Color R(const IntersectRec& ir) const {return color;}

protected:
  Real etaI, etaT;
  Color color;
};

class SpecularDielectric_BTDF : public BxDF {
public:
  SpecularDielectric_BTDF(const Color& _color, const Real _etaA, const Real _etaB) : etaA(_etaA), etaB(_etaB), color (_color) {}
  const Color f(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir, bool* isSpecular) const {return Color::Black();}
  Real pdf(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir) const {return 0.;}
  void sample_f(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, const Vec2& uv, Color* bsdf, Real* pdf, bool* isSpecular) const;
	virtual Color T(const IntersectRec& ir) const {return color;}
private:
  Real etaA, etaB; // A above surface (same side as normal points)
  Color color;
};

class Dielectric_BSDF : public BxDF {
public:
  Dielectric_BSDF(const Color& _R, const Color& _T, const Real _etaA, const Real _etaB) : etaA(_etaA), etaB(_etaB), Rc (_R), Tc (_T) {}
  const Color f(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir, bool* isSpecular) const {
    *isSpecular = true;
    return Color::Black();
  }
  Real pdf(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir) const {return 0.;}
  void sample_f(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, const Vec2& uv, Color* bsdf, Real* pdf, bool* isSpecular) const;
  virtual Color R(const IntersectRec& ir) const {return Rc;}
  virtual Color T(const IntersectRec& ir) const {return Tc;}

protected:
  Real etaA, etaB; // A above surface (same side as normal points)
  Color Rc, Tc;
};

#pragma once

#include <glm/vec3.hpp>
#include <memory>
#include <string>

#include "common.hpp"
#include "BxDF.hpp"
#include "Color.hpp"
#include "IntersectRec.hpp"
#include "SampleGenerator.hpp"
#include "Texture.hpp"


struct MTLMat {
    std::string name;    
    Vec3 Ka;        // ambient
    Vec3 Kd;        // diffuse
    Vec3 Ks;        // specular
    Vec3 Ke;        // emissive; extension to original
    Real Ns;       // specular coefficient
    Real Ni;       // index of refraction
    Real Pr;       // roughness; extension
    Real d;        // dissolve (transparency; d = 1.0 means fully opaque)
    std::string map_Ka;  // these are textures
    std::string map_Kd;
    std::string map_Ks;
    std::string map_Ke;
    std::string map_Ns;
    std::string map_d;
    std::string map_bump;
    MTLMat() : name (""), Ka ({0., 0., 0.}), Kd ({0., 0., 0.}), Ks ({0., 0., 0.}), Ke ({0., 0., 0.}), Ns (0.), Ni (1.0), Pr (0.0), d(1.0), map_Ka (""), map_Kd (""), map_Ks (""), map_Ke (""), map_Ns(""), map_d(""), map_bump("") {}
};

class Material {
public:
    std::string name;
    int id;
    Material() : id(currentID++) {}

    Material(int id_) : id(id_) {}
    Material(const std::string& name_) : name (name_), id (currentID++) {}
    Material(const std::string& name_, int id_) : name (name_), id (id_) {}

    virtual const Color brdf(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir, bool *isSpecular) const = 0;
    virtual Real pdf(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir) const = 0;
 	virtual void sample_f(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, const Vec2& uv, Color* bsdf, Real* pdf, bool* isSpecular) const = 0;
 
    virtual const Color getEmission() const {return Color::Black();}
    static int currentID;  
};

using TPTR = std::shared_ptr <Texture <Color>>;

class DiffuseMaterial : public Material {
    public:
        DiffuseMaterial(const std::string& name, const TPTR& texKd, Color emit) : Material (name), emissionColor (emit) {
            lbrdf = new Lambertian_BRDF(texKd);
        }

        DiffuseMaterial(const std::string& name, const TPTR& texKd) : Material (name), emissionColor (Color::Black()) {
            lbrdf = new Lambertian_BRDF(texKd);
        }

        DiffuseMaterial(const std::string& name, const Color& Kd) : Material (name) {
            TPTR texKd = std::make_shared <ConstantTexture <Color>> (Kd);
            lbrdf = new Lambertian_BRDF(texKd);
        }

        DiffuseMaterial(const std::string& name, const Color& Kd, Color emit) : Material (name), emissionColor (emit) {
            TPTR texKd = std::make_shared <ConstantTexture <Color>> (Kd);
            lbrdf = new Lambertian_BRDF(texKd);
        }

        ~DiffuseMaterial() {
            if (lbrdf) delete(lbrdf);
        }

        const Color brdf(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir, bool *isSpecular) const {
            return lbrdf->f(wo_local, wi_local, ir, isSpecular);
        }
        Real pdf(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir) const {
            return lbrdf->pdf(wo_local, wi_local, ir);
        }
        void sample_f(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, const Vec2& uv, Color* bsdf, Real* pdf, bool* isSpecular) const {
            lbrdf->sample_f(wo_local, wi_local, ir, uv, bsdf, pdf, isSpecular);
            *isSpecular = false;
        }

        const Color getEmission() const {return emissionColor;}
  
        Color emissionColor;
        Lambertian_BRDF* lbrdf;
};

class GlassMaterial : public Material {
    public:
        GlassMaterial() =delete;
        GlassMaterial(const std::string& name, const TPTR& texR, const TPTR& texT, Real IOR) : Material(name) {
            dBSDF = new Dielectric_BSDF(texR, texT, 1.00029, IOR);
        }
        GlassMaterial(const std::string& name, const Color& R, const Color& T, Real IOR) : Material(name) {
            TPTR texR = std::make_shared <ConstantTexture <Color>> (R),
                texT = std::make_shared <ConstantTexture <Color>> (T);
            dBSDF = new Dielectric_BSDF(texR, texT, 1.00029, IOR);
        }

        ~GlassMaterial() {
            if (dBSDF) delete (dBSDF);
        }
        const Color brdf(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir, bool *isSpecular) const {
            *isSpecular = true;
            return Color::Black();
        }
        Real pdf(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir) const {return 0;}
        void sample_f(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, const Vec2& uv, Color* bsdf, Real* pdf, bool* isSpecular) const {
            dBSDF->sample_f(wo_local, wi_local, ir, uv, bsdf, pdf, isSpecular);
        }

        Dielectric_BSDF* dBSDF = nullptr;
};

class MetalMaterial : public Material {
    public:
        MetalMaterial() =delete;
//        MetalMaterial(const std::string& name, const TPTR& texColor, const std::shared_ptr <Texture<Real>>& texAlpha, const std::shared_ptr <Texture <Color>>& texEta) : Material(name) {
//            mfBRDF = new Microfacet_BRDF(texColor, "schlick", "GGX", texAlpha, texEta);
//        }
        MetalMaterial(const std::string& name, const TPTR& R0, const std::shared_ptr <Texture<Real>>& texAlpha) : Material(name) {
            mfBRDF = new Microfacet_BRDF(R0, "schlick", "GGX", texAlpha);
        }
       
        MetalMaterial(const std::string& name, const TPTR& texEta, const TPTR& texK, const std::shared_ptr <Texture <Real>>& texAlpha) : Material (name) {
            mfBRDF = new Microfacet_BRDF(texEta, texK, "schlick", "GGX", texAlpha);
        }

       ~MetalMaterial() {
            if (mfBRDF) delete (mfBRDF);
        }
        const Color brdf(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir, bool *isSpecular) const {
            *isSpecular = false;
            return mfBRDF->f(wo_local, wi_local, ir, isSpecular);
        }
        Real pdf(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir) const {return 0;}
        void sample_f(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, const Vec2& uv, Color* bsdf, Real* pdf, bool* isSpecular) const {
            mfBRDF->sample_f(wo_local, wi_local, ir, uv, bsdf, pdf, isSpecular);
        }

        Microfacet_BRDF* mfBRDF;
};

class ADMaterial : public Material {
public:
    ADMaterial() =delete;
    ADMaterial( const std::string& name,
                const TPTR& _opacity,
                const Real _coat,
                const TPTR& _coat_color,
                const Real _coat_roughness,
                const Real _coat_IOR,
                const Vec3& _coat_normal,
                const Real _emission,
                const Color& _emission_color,
                const Real _metalness,
                const Real _base,
                const TPTR& _base_color,
                const Real _specular,
                const TPTR& _specular_color,
                const Real _specular_roughness,
                const Real _specular_IOR,
                const Real _diffuse_roughness,
                const Real _transmission,
                const TPTR& _transmission_color) :
                Material(name),
                opacity(_opacity),
                coat (_coat),
                coat_color (_coat_color),
                coat_roughness (_coat_roughness),
                coat_IOR (_coat_IOR),
                coat_normal (_coat_normal),
                emission (_emission),
                emission_color (_emission_color),
                metalness (_metalness),
                base (_base),
                base_color (_base_color),
                specular (_specular),
                specular_color (_specular_color),
                specular_roughness (_specular_roughness),
                specular_IOR (_specular_IOR),
                diffuse_roughness (_diffuse_roughness),
                transmission (_transmission),
                transmission_color (_transmission_color) {
                    init();
                }

  ADMaterial(const MTLMat& mat) : Material (mat.name), 
                                  opacity (std::make_shared <ConstantTexture <Color>> (mat.d)),
                                  base_color (std::make_shared <ConstantTexture <Color>> (mat.Kd)),
                                  specular_color (std::make_shared <ConstantTexture <Color>> (mat.Ks)),
                                  emission_color (mat.Ke),
                                  diffuse_roughness (mat.Pr), specular_roughness (mat.Pr) {
        init();
    }  

    ~ADMaterial() {
        if (coat_brdf) delete coat_brdf;
        if (metal_brdf) delete metal_brdf;
        if (specular_brdf) delete specular_brdf;
        if (specular_btdf) delete specular_btdf;
        if (diffuse_brdf) delete diffuse_brdf;
    }

    void init() {
      int rhoSamples = 81;
      SampleGenerator sg(Vec2(0), Vec2(1), rhoSamples*2);
      IntersectRec ir;
      if (coat > 0.) {
            coat_brdf = new Microfacet_BRDF(coat_color, "fresnel_dielectric_schlick", "GGX", std::make_shared <ConstantTexture <Real>> (coat_roughness*coat_roughness)); // need to fill this in
            coat_brdf_reflectance = coat_brdf->rho(rhoSamples, &sg, ir);
        }
        if (metalness > 0.) {
            metal_brdf = new Microfacet_BRDF(base_color, "fresnel_conductor_schlick", "GGX", std::make_shared <ConstantTexture <Real>> (specular_roughness*specular_roughness)); // need to fill this in
        }
        if (specular > 0.) {
            specular_brdf = new Microfacet_BRDF(specular_color, "fresnel_dielectric_schlick", "GGX", std::make_shared <ConstantTexture <Real>> (specular_roughness*specular_roughness));
            specular_brdf_reflectance = specular_brdf->rho(rhoSamples, &sg, ir);
        }
        if (transmission >0.) {
            specular_btdf = new Microfacet_BTDF(specular_color, "fresnel_dielectric_schlick", "GGX", std::make_shared <ConstantTexture <Real>> (specular_roughness*specular_roughness), std::make_shared <ConstantTexture <Color>> (specular_IOR));
        }
        if (diffuse_roughness > 0.) {
            diffuse_brdf = new OrenNayar_BRDF();
        }
        else {
            diffuse_brdf = new Lambertian_BRDF(base_color);
        }
    }
    const Color brdf(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir, bool *isSpecular) const;
    Real pdf(const Vec3& wo_local, const Vec3& wi_local, const IntersectRec& ir) const;
 	void sample_f(const Vec3& wo_local, Vec3& wi_local, const IntersectRec& ir, const Vec2& uv, Color* bsdf, Real* pdf, bool* isSpecular) const;
 
    const Color getEmission() const {return this->emission_color * this->emission;}

    static std::shared_ptr <ADMaterial> makeDiffuse(const std::string& name, const Color& Kd, const Real roughness) {
        TPTR w = std::make_shared <ConstantTexture <Color>> (Color::White());
        TPTR texKd = std::make_shared <ConstantTexture <Color>> (Kd);
        return std::make_shared <ADMaterial> (name, w, 0, w, 0, 0, Vec3(0), 0, 
            Color::White(), 0, 1.0, texKd, 0, w, 0, 0, roughness, 0, w);
    }

    // member variables
    const TPTR opacity = std::make_shared <ConstantTexture <Color>> (Color::White());
    const Real coat = 0.0f;
    const TPTR coat_color = std::make_shared <ConstantTexture <Color>> (Color::White());
    const Real coat_roughness = 0.1f;
    const Real coat_IOR = 1.5f;
    const Vec3 coat_normal = Vec3(0.0);
    const Real emission = 0.0f;
    const Color emission_color = Color::White();
    const Real metalness = 0.0f;
    const Real base = 0.8f;
    const TPTR base_color = std::make_shared <ConstantTexture <Color>> (Color::White());
    const Real specular = 0.0f;
    const TPTR specular_color = std::make_shared <ConstantTexture <Color>> (Color::White());
    const Real specular_roughness = 0.2f;
    const Real specular_IOR = 1.5f; 
    const Real diffuse_roughness = 0.0f;
    const Real transmission = 0.0f;
    const TPTR transmission_color = std::make_shared <ConstantTexture <Color>> (Color::White());

    Color coat_brdf_reflectance;
    Color specular_brdf_reflectance;

    BxDF* coat_brdf = nullptr;
    BxDF* metal_brdf = nullptr;
    BxDF* specular_brdf = nullptr;
    BxDF* specular_btdf = nullptr;
    BxDF* diffuse_brdf = nullptr;
  
private:

};

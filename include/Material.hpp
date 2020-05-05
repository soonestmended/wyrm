#pragma once

#include <glm/vec3.hpp>
#include <string>

#include "BxDF.hpp"
#include "Color.hpp"
#include "IntersectRec.hpp"

struct MTLMat {
    std::string name;    
    glm::vec3 Ka;        // ambient
    glm::vec3 Kd;        // diffuse
    glm::vec3 Ks;        // specular
    glm::vec3 Ke;        // emissive; extension to original
    float Ns;       // specular coefficient
    float Ni;       // index of refraction
    float Pr;       // roughness; extension
    float d;        // dissolve (transparency; d = 1.0 means fully opaque)
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
    int id;
    std::string name;
    Material() : id(currentID++) {}
    Material(int id_) : id(id_) {}
    Material(const std::string& name_) : name (name_), id (currentID++) {}
    Material(const std::string& name_, int id_) : name (name_), id (id_) {}

    virtual const Color brdf(const glm::vec3& wo_local, const glm::vec3& wi_local, const IntersectRec& ir, bool *isSpecular) const = 0;
    virtual const float pdf(const glm::vec3& wo_local, const glm::vec3& wi_local, const IntersectRec& ir) const = 0;
    virtual const void sample_f(const glm::vec3& wo_local, glm::vec3& wi_local, Color* bsdf, float* pdf, bool* isSpecular) const = 0;
 
    virtual const Color getEmission() const {return Color::Black();}
    static int currentID;  
};

class DiffuseMaterial : public Material {
public:
    DiffuseMaterial(const std::string& name, Color& color) : Material (name) {
        lbrdf = new Lambertian_BRDF(color);
    }

    ~DiffuseMaterial() {
        if (lbrdf) delete(lbrdf);
    }

    const Color brdf(const glm::vec3& wo_local, const glm::vec3& wi_local, const IntersectRec& ir, bool *isSpecular) const {
        bool foo = false;
        return lbrdf->f(wo_local, wi_local, &foo);
    }
    const float pdf(const glm::vec3& wo_local, const glm::vec3& wi_local, const IntersectRec& ir) const {
        return lbrdf->pdf(wo_local, wi_local);
    }
    const void sample_f(const glm::vec3& wo_local, glm::vec3& wi_local, Color* bsdf, float* pdf, bool* isSpecular) const {
        lbrdf->sample_f(wo_local, wi_local, bsdf, pdf, isSpecular);
    }

    Lambertian_BRDF* lbrdf;
};

class GlassMaterial : public Material {
public:
    GlassMaterial() =delete;
    GlassMaterial(const std::string& name, Color& color, float IOR) : Material(name) {
        dBSDF = new Dielectric_BSDF(color, 1.00029, IOR);
    }
    ~GlassMaterial() {
        if (dBSDF) delete (dBSDF);
    }
    const Color brdf(const glm::vec3& wo_local, const glm::vec3& wi_local, const IntersectRec& ir, bool *isSpecular) const {
        *isSpecular = true;
        return Color::Black();
    }
    const float pdf(const glm::vec3& wo_local, const glm::vec3& wi_local, const IntersectRec& ir) const {return 0;}
    const void sample_f(const glm::vec3& wo_local, glm::vec3& wi_local, Color* bsdf, float* pdf, bool* isSpecular) const {
        dBSDF->sample_f(wo_local, wi_local, bsdf, pdf, isSpecular);
    }

    Dielectric_BSDF* dBSDF = nullptr;
};

class ADMaterial : public Material {
public:
    ADMaterial() =delete;
    ADMaterial( const std::string& name,
                const Color& _opacity, 
                const float _coat,
                const Color& _coat_color,
                const float _coat_roughness,
                const float _coat_IOR,
                const glm::vec3& _coat_normal,
                const float _emission,
                const Color& _emission_color,
                const float _metalness,
                const float _base,
                const Color& _base_color,
                const float _specular,
                const Color& _specular_color,
                const float _specular_roughness,
                const float _specular_IOR,
                const float _diffuse_roughness,
                const float _transmission,
                const Color& _transmission_color) :
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
    opacity (Color(mat.d)), base_color (mat.Kd), specular_color (mat.Ks), emission_color (mat.Ke), 
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
        if (coat > 0.) {
            coat_brdf = new GGX_BRDF(); // need to fill this in
            coat_brdf_reflectance = coat_brdf->rho(81);
        }
        if (metalness > 0.) {
            metal_brdf = new GGX_BRDF();
        }
        if (specular > 0.) {
            specular_brdf = new SpecularDielectric_BRDF(specular_color, 1.00029f, specular_IOR);
            specular_brdf_reflectance = specular_brdf->rho(81);
        }
        if (transmission >0.) {
            specular_btdf = new GGX_BRDF();
        }
        if (diffuse_roughness > 0.) {
            diffuse_brdf = new OrenNayar_BRDF();
        }
        else {
            diffuse_brdf = new Lambertian_BRDF(base_color);
        }
    }
    const Color brdf(const glm::vec3& wo_local, const glm::vec3& wi_local, const IntersectRec& ir, bool *isSpecular) const;
    const float pdf(const glm::vec3& wo_local, const glm::vec3& wi_local, const IntersectRec& ir) const;
    const void sample_f(const glm::vec3& wo_local, glm::vec3& wi_local, Color* bsdf, float* pdf, bool* isSpecular) const;
 
    const Color getEmission() const {return this->emission_color * this->emission;}

    static std::shared_ptr <ADMaterial> makeDiffuse(const std::string& name, const Color& Kd, const float roughness) {
        Color w = Color::White();
        return std::make_shared <ADMaterial> (name, w, 0, w, 0, 0, glm::vec3(0), 0, 
            w, 0, 1.0, Kd, 0, w, 0, 0, roughness, 0, w);
    }

    // member variables
    const Color opacity = Color::White();
    const float coat = 0.0f;
    const Color coat_color = Color::White();
    const float coat_roughness = 0.1f;
    const float coat_IOR = 1.5f;
    const glm::vec3 coat_normal = glm::vec3(0.0);
    const float emission = 0.0f;
    const Color emission_color = Color::White();
    const float metalness = 0.0f;
    const float base = 0.8f;
    const Color base_color = Color::White();
    const float specular = 0.0f;
    const Color specular_color = Color::White();
    const float specular_roughness = 0.2f;
    const float specular_IOR = 1.5f; 
    const float diffuse_roughness = 0.0f;
    const float transmission = 0.0f;
    const Color transmission_color = Color::White();

    Color coat_brdf_reflectance;
    Color specular_brdf_reflectance;

    BxDF* coat_brdf = nullptr;
    BxDF* metal_brdf = nullptr;
    BxDF* specular_brdf = nullptr;
    BxDF* specular_btdf = nullptr;
    BxDF* diffuse_brdf = nullptr;

private:

};
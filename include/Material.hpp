#pragma once

#include <glm/vec3.hpp>
#include <string>

#include "Color.hpp"

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
    static int currentID;

    
};

class SimpleMaterial : public Material {
public:
    SimpleMaterial(const glm::vec3 &color_) : Material(), color (color_) {}
    SimpleMaterial(const MTLMat& mtlMat) : Material(mtlMat.name), color (mtlMat.Kd) {}
    Color color;
};

class ADMaterial : public Material {
public:
    ADMaterial() =delete;
    ADMaterial( const Color& _opacity, 
                const float _coat,
                const Color& _coat_color,
                const float _coat_roughness,
                const float _coat_IOR,
                const glm::vec3& _coat_normal,
                const float _emission,
                const Color& _emission_color,
                const float _base,
                const Color& _base_color,
                const float _specular,
                const Color& _specular_color,
                const float _specular_roughness,
                const float _specular_IOR,
                const float _diffuse_roughness) :
                opacity(_opacity),
                coat (_coat),
                coat_color (_coat_color),
                coat_roughness (_coat_roughness),
                coat_IOR (_coat_IOR),
                coat_normal (_coat_normal),
                emission (_emission),
                emission_color (_emission_color),
                base (_base),
                base_color (_base_color),
                specular (_specular),
                specular_color (_specular_color),
                specular_roughness (_specular_roughness),
                specular_IOR (_specular_IOR),
                diffuse_roughness (_diffuse_roughness) {}


    const Color opacity;
    const float coat;
    const Color coat_color;
    const float coat_roughness;
    const float coat_IOR;
    const glm::vec3 coat_normal;
    const float emission;
    const Color emission_color;
    const float base;
    const Color base_color;
    const float specular;
    const Color specular_color;
    const float specular_roughness;
    const float specular_IOR;
    const float diffuse_roughness;

private:

};
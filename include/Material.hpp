#pragma once

#include <glm/vec3.hpp>
#include <string>

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
    int id_;
    std::string name_;
    Material() : id_(currentID++) {}
    Material(int id) : id_(id) {}
    Material(const std::string& name) : name_ (name), id_ (currentID++) {}
    Material(const std::string& name, int id) : name_ (name), id_ (id) {}
    static int currentID;
};

class SimpleMaterial : public Material {
public:
    SimpleMaterial(const glm::vec3 &color) : Material(), color_ (color) {}
    SimpleMaterial(const MTLMat& mtlMat) : Material(mtlMat.name), color_ (mtlMat.Kd) {}


    glm::vec3 color_;
};
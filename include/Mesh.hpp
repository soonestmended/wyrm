#pragma once

#include <memory>
#include <vector>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtx/string_cast.hpp>


#include "Material.hpp"
#include "Primitive.hpp"

class Mesh {
protected:
    BBox bbox;
    std::vector <glm::vec4> vertices; 
    std::vector <glm::vec3> texCoords;
    std::vector <glm::vec3> normals;
    std::vector <std::shared_ptr <Material>> materials;
    std::vector <Tri> tris;

public:
    // Makes a Mesh out of data that gets moved to the new object.
    // Principally this is used for reading from obj files.
    Mesh() =delete;

    Mesh(std::vector <glm::vec4> &&vertices_, std::vector <glm::vec3> &&texCoords_, std::vector <glm::vec3> &&normals_, std::vector <std::shared_ptr <Material>> &&materials_, std::vector <Tri> &&tris_) 
    : vertices (std::move(vertices_)), texCoords (std::move(texCoords_)), normals (std::move(normals_)), materials (std::move(materials_)), tris (std::move(tris_)) 
    {
        for (auto& v : vertices) 
            bbox.enclose(v);
        std::cout << "bbmin: " << glm::to_string(bbox.min) << "\tbbmax: " << glm::to_string(bbox.max) << std::endl;

    }

    const BBox& getBBox() const {return bbox;}
    const std::vector <std::shared_ptr<Material>>& getMaterials() const {return materials;}

    std::vector <std::shared_ptr<Primitive>> toPrimitives() const;

    //void recenter(const glm::vec3 &c);
    //void rotate(const glm::vec3 &axis, const float angle);
    //void scale(const float s);
    //void translate(const glm::vec3 &dx);

    friend class MeshInstance;
};

class MeshInstance {

private:
    std::shared_ptr<Mesh> meshPtr;
    std::vector <std::shared_ptr <Material>> materials;
    glm::mat4 xform;
    glm::mat3 inv_xform_T_3x3;

public:
    MeshInstance(std::shared_ptr<Mesh> ptr, glm::mat4 xform_, std::shared_ptr<Material> material_ = nullptr) : meshPtr(ptr), xform(xform_) {
        glm::mat3 tmp(xform_);

        inv_xform_T_3x3 = glm::inverseTranspose(tmp);
        if (material_ != nullptr) {
            this->materials.push_back(material_);
        }
    }
    std::vector <std::shared_ptr<Primitive>> toPrimitives() const;
    const std::vector <std::shared_ptr<Material>>& getMaterials() const {
        if (materials.size() == 0) {
            return meshPtr->getMaterials();
        }
        else {
            return materials;
        }
    }

    // Add BBox and other Mesh functions
};
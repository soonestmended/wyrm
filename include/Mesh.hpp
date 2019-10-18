#pragma once

#include <memory>
#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Material.hpp"
#include "Primitive.hpp"

class Mesh {
private:
    std::vector <glm::vec4> vertices; 
    std::vector <glm::vec3> texCoords;
    std::vector <glm::vec3> normals;
    std::vector <std::shared_ptr <Material>> materials;
    std::vector <Tri> tris;

public:
    // Makes a Mesh out of data that gets moved to the new object.
    // Principally this is used for reading from obj files.

    Mesh(std::vector <glm::vec4> &&vertices_, std::vector <glm::vec3> &&texCoords_, std::vector <glm::vec3> &&normals_, std::vector <std::shared_ptr <Material>> &&materials_, std::vector <Tri> &&tris_) 
    : vertices (std::move(vertices_)), texCoords (std::move(texCoords_)), normals (std::move(normals_)), materials (std::move(materials_)), tris (std::move(tris_)) 
    {}

    std::vector <std::shared_ptr<Primitive>> toPrimitives() const;

};
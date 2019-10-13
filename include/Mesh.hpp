#pragma once

#include <memory>
#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Material.hpp"
#include "Primitive.hpp"

class Mesh {
private:
    std::vector <glm::vec4> vertices_; 
    std::vector <glm::vec3> texCoords_;
    std::vector <glm::vec3> normals_;
    std::vector <std::shared_ptr <Material>> materials_;
    std::vector <Tri> tris_;

public:
    // Makes a Mesh out of data that gets moved to the new object.
    // Principally this is used for reading from obj files.

    Mesh(std::vector <glm::vec4> &&vertices, std::vector <glm::vec3> &&texCoords, std::vector <glm::vec3> &&normals, std::vector <std::shared_ptr <Material>> &&materials, std::vector <Tri> &&tris) 
    : vertices_ (std::move(vertices)), texCoords_ (std::move(texCoords)), normals_ (std::move(normals)), materials_ (std::move(materials)), tris_ (std::move(tris)) 
    {}

    std::vector <std::shared_ptr<Primitive>> toPrimitives() const;

};
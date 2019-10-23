#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "Light.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Primitive.hpp"

class Scene {
private:
    std::vector <std::shared_ptr<Light>> lights;
    std::vector <std::shared_ptr<Material>> materials;
    std::vector <std::shared_ptr<Primitive>> primitives;
    static std::vector <MTLMat> parseMtl(std::string fileName);

public:
    Scene() =delete;
    Scene(std::vector <std::shared_ptr <Light>> &&lights_, std::vector <std::shared_ptr<Material>> &&materials_, std::vector <std::shared_ptr <Primitive>> &&primitives_) :
    lights (std::move(lights_)), materials (std::move(materials_)), primitives (std::move(primitives_)) {}

    void printInfo() const;

    void addMesh(const std::shared_ptr<Mesh>& m);
    void addMeshInstance(const std::shared_ptr<Mesh>& mptr, const BBox& wrapper);

    void addPrimitives(const std::vector <std::shared_ptr<Primitive>>& primitives) {
        this->primitives.insert(
            this->primitives.end(),
            std::make_move_iterator(primitives.begin()),
            std::make_move_iterator(primitives.end()));
    }
    void addMaterials(const std::vector <std::shared_ptr<Material>>& materials) {
        this->materials.insert(
            this->materials.end(),
            std::make_move_iterator(materials.begin()),
            std::make_move_iterator(materials.end()));

    }

    const std::vector <std::shared_ptr<Primitive>> &getPrimitives() const {return primitives;}

    static std::shared_ptr <Mesh> parseObj(std::string fileName);

    static std::unique_ptr <Scene> emptyScene() {
        // this is stupid
        std::vector <std::shared_ptr <Light>> lights_;
        std::vector <std::shared_ptr<Material>> materials_;
        std::vector <std::shared_ptr <Primitive>> primitives_;
        return std::make_unique <Scene> (std::move(lights_), std::move(materials_), std::move(primitives_));
    
    };
};

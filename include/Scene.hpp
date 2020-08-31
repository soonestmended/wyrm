#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "common.hpp"
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
    BBox bbox;

public:
    Scene() =delete;
    Scene(std::vector <std::shared_ptr <Light>> &&lights_, std::vector <std::shared_ptr<Material>> &&materials_, std::vector <std::shared_ptr <Primitive>> &&primitives_) :
    lights (std::move(lights_)), materials (std::move(materials_)), primitives (std::move(primitives_)) {
        std::cout << "Start scene constructor" << std::endl;
        std::cout << "Primitives size: " << primitives.size() << std::endl;
        for (auto& prim : primitives) {
            //std::cout << "." << std::flush;
            bbox.enclose(prim->getBBox());
            //std::cout << " post bbox" << std::flush;
            if (!prim->getMaterial()->getEmission().isBlack()) {
                lights.push_back(std::make_shared <GeometricLight> (prim));
            }
            //std::cout << " post emission check" << std::flush;
        }
        //std::cout << std::endl;
    }

    void printInfo() const;

    void addMesh(const std::shared_ptr<Mesh>& m);
    void addMeshInstance(const std::shared_ptr<Mesh>& mptr, const BBox& dest, const Vec3 &axis = Vec3(0.0), const Real angle = 0.0);
    void addMeshInstance(const std::shared_ptr<Mesh>& mptr, const Vec3 center, const Real scale, const Vec3 &axis, const Real angle);

    void addPrimitives(const std::vector <std::shared_ptr<Primitive>>& primitives) {
        this->primitives.insert(
            this->primitives.end(),
            std::make_move_iterator(primitives.begin()),
            std::make_move_iterator(primitives.end()));
            
        for (auto& prim : primitives) {
            bbox.enclose(prim->getBBox());
            // check for emissive prims, then make geometric lights for these
            if (!prim->getMaterial()->getEmission().isBlack()) {
                lights.push_back(std::make_shared <GeometricLight> (prim));
            }
        }
    }

    void addMaterials(const std::vector <std::shared_ptr<Material>>& materials) {
        this->materials.insert(
            this->materials.end(),
            std::make_move_iterator(materials.begin()),
            std::make_move_iterator(materials.end()));

    }

    void addLights(const std::vector <std::shared_ptr<Light>>& lights) {
        this->lights.insert(
            this->lights.end(),
            std::make_move_iterator(lights.begin()),
            std::make_move_iterator(lights.end()));

    }

    const std::vector <std::shared_ptr<Primitive>> &getPrimitives() const {return primitives;}
    const std::vector <std::shared_ptr<Light>> &getLights() const {return lights;}

    const BBox& getBBox() const {return bbox;}
    //static std::shared_ptr <Mesh> parseObj(std::string fileName);

    static std::unique_ptr <Scene> emptyScene() {
        // this is stupid
        std::vector <std::shared_ptr <Light>> lights_;
        std::vector <std::shared_ptr<Material>> materials_;
        std::vector <std::shared_ptr <Primitive>> primitives_;
        return std::make_unique <Scene> (std::move(lights_), std::move(materials_), std::move(primitives_));
    
    };
};

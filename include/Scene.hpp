#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "Light.hpp"
#include "Material.hpp"
#include "Primitive.hpp"

class Scene {
private:
    std::vector <std::shared_ptr<Light>> lights;
    std::vector <std::shared_ptr<Material>> materials;
    std::vector <std::shared_ptr<Primitive>> primitives;

public:
    Scene(std::vector <std::shared_ptr <Light>> &&_l, std::vector <std::shared_ptr<Material>> &&_m, std::vector <std::shared_ptr <Primitive>> &&_p) :
    lights (std::move(_l)), materials (std::move(_m)), primitives (std::move(_p)) {}

    void print();

    static std::unique_ptr <Scene> parseObj(std::string fileName);

};
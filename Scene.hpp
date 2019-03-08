#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include "Light.hpp"
#include "Material.hpp"
#include "Primitive.hpp"

using namespace std;

class Scene {
private:
    vector <shared_ptr<Light>> lights;
    vector <shared_ptr<Material>> materials;
    vector <shared_ptr<Primitive>> primitives;

public:
    Scene(vector <shared_ptr <Light>> &_l, vector <shared_ptr<Material>> &_m, vector <shared_ptr <Primitive>> &_p) :
    lights (move(_l)), materials (move(_m)), primitives (move(_p)) {}

    void print() {
        cout << "Lights: " << lights.size() << endl;
        cout << "Materials: " << materials.size() << endl;
        cout << "Primitives: " << primitives.size() << endl;
    }

};
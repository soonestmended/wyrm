#include <iostream>
#include <memory>
#include <vector>

#include "Material.hpp"
#include "Scene.hpp"

using namespace std;

int main (int argc, char ** argv) {
    vector <shared_ptr <Light>> parsed_lights;
    for (int i = 0; i < 10; ++i) {
        parsed_lights.push_back(make_shared <Light> ((float) i));
    }

    vector <shared_ptr <Material>> parsed_materials;
    for (int i = 0; i < 10; ++i) {
        parsed_materials.push_back(make_shared <Material> (i));
    }

    vector <shared_ptr <Primitive>> parsed_primitives;
    for (int i = 0; i < 10; ++i) {
        parsed_primitives.push_back(make_shared <Primitive> ((float) i));
    }
    Scene s = Scene(parsed_lights, parsed_materials, parsed_primitives);
    cout << "parsed_materials: " << parsed_materials.size() << endl;
}
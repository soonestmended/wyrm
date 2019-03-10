#include <iostream>
#include <memory>
#include <vector>

#include "Material.hpp"
#include "Scene.hpp"

using namespace std;

int main (int argc, char ** argv) {
    unique_ptr <Scene> s;
    s = Scene::parseObj(argv[1]);
    if (s == nullptr) {
        cout << "Parse failed. " << endl;
        exit(0);
    }
}
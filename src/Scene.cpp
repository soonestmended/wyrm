#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include <boost/algorithm/string.hpp>

#include "Scene.hpp"

using namespace std;

void Scene::print() {
    cout << "Lights: " << lights.size() << endl;
    cout << "Materials: " << materials.size() << endl;
    cout << "Primitives: " << primitives.size() << endl;
}

unique_ptr <Scene> Scene::parseObj(string fileName) {
    unique_ptr <Scene> s;
    ifstream file (fileName);
    string line;
    if (!file.is_open()) {
        cout << "Unable to open file " << fileName << endl;
        return nullptr;
    }
    while (getline(file, line)) {
        vector <string> tokens;
        boost::split(tokens, line, [](char c) {return c == ' ';});
        cout << tokens[0] << endl;
    }
    return s;
}
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>

#include <boost/algorithm/string.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "pugixml.hpp"

#include "Color.hpp"
#include "Mesh.hpp"
#include "Parser.hpp"
#include "Scene.hpp"

using namespace std;
using namespace glm;

void parseError(const string& line) {
    cout << "Parse error: " << line << endl;
}

/* Construct vec3 from tokens. If there aren't enough tokens, defaultValue is substituted. */
const vec3 Parser::readVec3(const vector <string> &tokens, float defaultValue = 1.0, int numTokens = -1) {
    if (numTokens == -1) {
        numTokens = tokens.size();
    }
    float x, y, z;
    x = y = z = defaultValue;
    if (numTokens > 1) {
        x = stof(tokens[1]);
    }
    if (numTokens > 2) {
        y = stof(tokens[2]);
    }
    if (numTokens > 3) {
        z = stof(tokens[3]);
    }
    return vec3(x, y, z);
}

/* Construct vec4 from tokens. If there aren't enough tokens, defaultValue is substituted. */
const vec4 Parser::readVec4(const vector <string> &tokens, const float defaultValue = 1.0, int numTokens = -1) {
    if (numTokens == -1) {
        numTokens = tokens.size();
    }
    const vec3 xyz = readVec3(tokens, defaultValue, numTokens);
    float w = defaultValue;
    if (numTokens > 4) {
        w = stof(tokens[4]);
    }
    return vec4(xyz, w);
}

vector <MTLMat> Parser::parseMtl(string fileName) {
    vector <MTLMat> ans;

    ifstream file (fileName);
    string line;
    if (!file.is_open()) {
        cout << "Unable to open file " << fileName << endl;
        return ans;
    }
    MTLMat m;
    bool firstPass = true;
    while (getline(file, line)) {
        
        vector <string> tokens;
        boost::trim_if(line, boost::is_any_of(" \r"));
        boost::split(tokens, line, [](char c) {return c == ' ';}, boost::token_compress_on);
        int numTokens = tokens.size();
        if (tokens[0].compare("#") == 0 || tokens[0].compare("") == 0) {
            continue;
        }
        else if (tokens[0].compare("newmtl") == 0) {
            if (!firstPass) {
                ans.push_back(m);
            }
            else {
                firstPass = false;
            }
            m = MTLMat{}; // start a new MTLmaterial for the following statements
            m.name = tokens[1];

        }
        else if (tokens[0].compare("Ka") == 0) {
            m.Ka = readVec3(tokens, 0.0, numTokens);
        }
        else if (tokens[0].compare("Kd") == 0) {
            m.Kd = readVec3(tokens, 0.0, numTokens);
        }
        else if (tokens[0].compare("Ks") == 0) {
            m.Ks = readVec3(tokens, 0.0, numTokens);
        }
        else if (tokens[0].compare("Ke") == 0) {
            m.Ke = readVec3(tokens, 0.0, numTokens);
        }
        else if (tokens[0].compare("Ns") == 0) {
            m.Ns = stof(tokens[1]);
        }
        else if (tokens[0].compare("Ni") == 0) {
            m.Ni = stof(tokens[1]);
        }
        else if (tokens[0].compare("Pr") == 0) {
            m.Pr = stof(tokens[1]);
        }
        else if (tokens[0].compare("d") == 0) {
            m.d = stof(tokens[1]);
        }
        else if (tokens[0].compare("map_Ka") == 0) {
            m.map_Ka = tokens[1];
        }
        else if (tokens[0].compare("map_Kd") == 0) {
            m.map_Kd = tokens[1];
        }
        else if (tokens[0].compare("map_Ks") == 0) {
            m.map_Ks = tokens[1];
        }
        else if (tokens[0].compare("map_Ke") == 0) {
            m.map_Ke = tokens[1];
        }
        else if (tokens[0].compare("map_Ns") == 0) {
            m.map_Ns = tokens[1];
        }
        else if (tokens[0].compare("map_d") == 0) {
            m.map_d = tokens[1];
        }
        else if (tokens[0].compare("map_bump") == 0 || tokens[0].compare("bump") == 0) {
            m.map_bump = tokens[1];
        }
    }

    if (!firstPass) {
        ans.push_back(m); // add the last MTLmaterial when we get to the end of the file
    }

    return ans;
}

shared_ptr <Mesh> Parser::parseObj(string fileName) {
    vector <vec4> vertices;
    vector <vec3> normals;
    vector <vec3> texCoords;
    vector <Tri> tris;

    vector <shared_ptr<Material>> materials;

    map <string, int> materialsMap;

    int currentMTLMaterialIndex = 0;
    int currentSmoothingGroupIndex = -1;
    ifstream file (fileName);
    string line;
    if (!file.is_open()) {
        cout << "Unable to open file " << fileName << endl;
        return nullptr;
    }
    while (getline(file, line)) {
        vector <string> tokens;
        boost::trim_if(line, boost::is_any_of(" \r"));
        boost::split(tokens, line, [](char c) {return c == ' ';}, boost::token_compress_on);
        int numTokens = tokens.size();
        if (tokens[0].compare("#") == 0 || tokens[0].compare("") == 0) {
            continue;
        }

        // Vertex data
        else if (tokens[0].compare("v") == 0) {
            // add a vertex
            if (numTokens < 4 || numTokens > 5) {
                parseError(line);
                continue;
            }
            vertices.push_back(readVec4(tokens, 1.0, numTokens));
        }
        else if (tokens[0].compare("vt") == 0) {
            // add a texture coordinate
            if (numTokens < 2 || numTokens > 4) {
                parseError(line);
                continue;
            }
            texCoords.push_back(readVec3(tokens, 0.0, numTokens));
        }
        else if (tokens[0].compare("vn") == 0) {
            // add a normal
            if (numTokens != 4) {
                parseError(line);
                continue;
            }
            normals.push_back(readVec3(tokens, 1.0, numTokens));
        }
        else if (tokens[0].compare("vp") == 0) {
            // add a parameter space vertex (?)
            cout << "Parameter space vertices not yet implemented" << endl;
            continue;
        }

        // Face data
        else if (tokens[0].compare("f") == 0) {
            //add a face
            auto processTri = [&] (const vector <string> &tokens) {
                // tokens must be of size 3
                Tri tri;

                if (tokens.size() != 3) {
                    cout << "Wrong number of tokens in call to processTri." << endl;
                    parseError(line);
                    return tri;
                }
                int i = 0;
                for (auto t : tokens) {
                    vector <string> faceData;
                    boost::split(faceData, t, [](char c) {return c == '/';});
                    tri.v[i] = stoi(faceData[0]);
                    if (tri.v[i] < 0) {
                        tri.v[i] += vertices.size(); // negative indices are offsets from end of current vertex array
                    }
                    else {
                        tri.v[i] -= 1; // .obj numbering starts at 1
                    }
                    if (faceData.size() == 2) {
                        // v/vt specified
                        tri.vt[i] = stoi(faceData[1]);
                        if (tri.vt[i] < 0) {
                            tri.vt[i] += texCoords.size();
                        }
                        else {
                            tri.vt[i] -= 1;
                        }
                        tri.vn[i] = -1;
                    }
                    else if (faceData.size() == 3) {
                        // v/vt/vn specified
                        // handle vt
                        if (faceData[1].compare("") == 0) { // specified as v//vn
                            tri.vt[i] = -1;
                        }
                        else {
                            tri.vt[i] = stoi(faceData[1]);
                            if (tri.vt[i] < 0) {
                                tri.vt[i] += texCoords.size();
                            }
                            else {
                                tri.vt[i] -= 1;
                            }
                        }

                        // handle vn
                        tri.vn[i] = stoi(faceData[2]);
                        if (tri.vn[i] < 0) {
                            tri.vn[i] += normals.size();
                        }
                        else {
                            tri.vn[i] -= 1;
                        }
                        tri.explicitNormals = true;
                    }
                    i++;
                }
                tri.m = currentMTLMaterialIndex;
                tri.s = currentSmoothingGroupIndex;
                return tri;
            };
            if (numTokens == 4) {
                tris.push_back(processTri(vector <string> (tokens.begin()+1, tokens.end())));
            }
            else if (numTokens == 5) {
                tris.push_back(processTri(vector <string> (tokens.begin()+1, tokens.end()-1)));
                tris.push_back(processTri(vector <string> {tokens[1], tokens[3], tokens[4]})); 
            }
            else {
                cout << "Faces of greater than 4 vertices not yet implemented. " << endl;
                parseError(line);
            }
        }

        else if (tokens[0].compare("s") == 0) {
            // smoothing group
            if (numTokens != 2) {
                cout << "Wrong number of tokens in smoothing group" << endl;
                parseError(line);
                continue;
            }
            if (tokens[0].compare("off") == 0) {
                currentSmoothingGroupIndex = 0;
            }
            else {
                currentSmoothingGroupIndex = stoi(tokens[1]);
            }
        }

        else if (tokens[0].compare("usemtl") == 0) {
            // smoothing group
            if (numTokens != 2) {
                cout << "Wrong number of tokens in MTLmaterial specification" << endl;
                parseError(line);
                continue;
            }
            cout << "Searching for mtl: " << tokens[1] << "..." << endl;
            auto it = materialsMap.find(tokens[1]);
            
            if (it != materialsMap.end()) {
                currentMTLMaterialIndex = it->second;
            }
            else {
                cout << "MTLMaterial " << tokens[1] << " not yet defined." << endl;
                parseError(line);
                continue;
            }
        }

        else if (tokens[0].compare("mtllib") == 0) {
            cout << "Parsing materials... " << endl;
            // load MTLmaterial file
            for (auto it = tokens.begin() + 1; it != tokens.end(); it++) {
                // read MTLmaterial and add it to our vector of materials
                vector <MTLMat> tv = parseMtl(*it); 
                cout << "Parsed " << tv.size() << " MTLMat(s)" << endl;
                // need to convert from MTLMats to Materials
                vector <ADMaterial> tv2 (tv.begin(), tv.end()); // convert MTLMats to ADMaterials
                for (auto m : tv2) {
                    materials.push_back(make_shared<ADMaterial>(m));
                }
            }

            for (auto m : materials) {
                materialsMap[m->name] = m->id;
            }

        }

        else {
            cout << "Not yet implemented: " << line << endl;
        }
    }

    cout << "Finished parsing file: " << fileName << endl;
    cout << "Parsed " << vertices.size() << " vertices" << endl;
    cout << "Parsed " << texCoords.size() << " texture coordinates" << endl;
    cout << "Parsed " << normals.size() << " normals" << endl;
    cout << "Parsed " << tris.size() << " tris" << endl;
    cout << "Parsed " << materials.size() << " materials" << endl;

    if (materials.size() == 0) {
        materials.push_back(ADMaterial::makeDiffuse("default", Color::Green(), 0));
    }

    // postprocess tris -- normal smoothing

    map <int, vector <Tri *>> smoothingGroupsToProcess;
    for (auto& t : tris) {
        if (!t.explicitNormals) {
            // make a new vertex normal based on just this face and assign its index to vn[0->3]
            vec3 e1 = vertices[t.v[1]] - vertices[t.v[0]];
            vec3 e2 = vertices[t.v[2]] - vertices[t.v[1]];
            normals.push_back(glm::normalize(glm::cross(e1, e2)));
            t.vn[0] = t.vn[1] = t.vn[2] = normals.size() - 1;
        }

        // if tri is in a smoothing group, add the triangle to the appropriate smoothing group
        if (t.s > 0) {
            auto it = smoothingGroupsToProcess.find(t.s);
            if (it == smoothingGroupsToProcess.end()) {
                // t.s not yet in list of smoothing groups to process, so let's add it and add this triangle to the group
                smoothingGroupsToProcess[t.s] = vector <Tri *>();
                smoothingGroupsToProcess[t.s].push_back(&t);
            }

            else {
                // t.s already in the list of groups, so just add this triangle to its group
                it->second.push_back(&t);
            }
        }
    }

    // now we have a map of tris to process split up according to smoothing group.
    // for each smoothing group, map vertices to a list of face normals. at the end, average them.

    for (auto sg : smoothingGroupsToProcess) {
        // sg is a <int, vector <Tri *>> pair, mapping smoothing group number to list of tris in group
        auto sgTris = sg.second;
        // map vertex indices to normals
        map<int, glm::vec3> groupNormals;
        // loop over tris in group
        for (auto t : sgTris) {
            
            //vec3 e1 = vertices[t->v[1]] - vertices[t->v[0]];
            //vec3 e2 = vertices[t->v[2]] - vertices[t->v[1]];
            //glm::vec3 &n = groupNormals[t->s] += glm::cross(e1, e2); // un-normalized face normal
            //glm::vec3 n = glm::cross(e1, e2); // un-normalized face normal
            
            // if we haven't encountered this group vertex yet, set its accumulated normal to zero
            for (int i = 0; i < 3; ++i) {
                if (groupNormals.find(t->v[i]) == groupNormals.end()) {
                    groupNormals[t->v[i]] = glm::vec3(0.0);
                }
            }

            // TODO: scale by area of triangle
            groupNormals[t->v[0]] += normals[t->vn[0]];
            groupNormals[t->v[1]] += normals[t->vn[1]];
            groupNormals[t->v[2]] += normals[t->vn[2]];
            
        }
        // now normalize all the normals in the group, add 
        // go over this when sober
        for (auto v : groupNormals) {
            v.second = glm::normalize(v.second);
            
        }

        // now we have a map of vertex index to new normal. these have to be inserted at the end of the
        // normals vector. A new map associates vertex indices with normal indices.
        
        map<int, int> v2vn;

        for (auto it : groupNormals) {
            normals.push_back(it.second);
            v2vn[it.first] = normals.size() - 1;
        }
        
        for (auto t : sgTris) {
            for (int i = 0; i < 3; i++) {
                // search for vertex index in v2vn
                t->vn[i] = v2vn[t->v[i]];
            }
        }

    } 
    // the above needs to be tested / debugged

    // make a Mesh out of the data we've read 
    vector <shared_ptr<Material>> mm = materials;

    return make_shared <Mesh>(move(vertices), move(texCoords), move(normals), move(mm), move(tris));
    
}

struct simple_walker: pugi::xml_tree_walker
{
    virtual bool for_each(pugi::xml_node& node)
    {
        for (int i = 0; i < depth(); ++i) std::cout << "  "; // indentation

        std::cout << "name='" << node.name() << "', value='" << node.value() << "'\n";

        return true; // continue traversal
    }
};

unordered_map <string, const pugi::xml_node> namedNodes;
shared_ptr <Material> defaultMaterial = ADMaterial::makeDiffuse("defaultMaterial", Color::Blue(), 0);
stack <mat4> transformStack;

glm::vec3 stringToVec3(string s) {
    vector <string> tokens;
    boost::trim_if(s, boost::is_any_of(" \r"));
    boost::split(tokens, s, [](char c) {return c == ' ';}, boost::token_compress_on);
    if (tokens.size() != 3) {
        cout << "ERROR: Attempt to read vec3 from string failed: " << s << endl;
        return glm::vec3(0.0);
    }
    return glm::vec3(stof(tokens[0]), stof(tokens[1]), stof(tokens[2]));
}

glm::vec4 stringToVec4(string s) {
    vector <string> tokens;
    boost::trim_if(s, boost::is_any_of(" \r"));
    boost::split(tokens, s, [](char c) {return c == ' ';}, boost::token_compress_on);
    if (tokens.size() != 4) {
        cout << "ERROR: Attempt to read vec4 from string failed: " << s << endl;
        return glm::vec4(0.0);
    }
    return glm::vec4(stof(tokens[0]), stof(tokens[1]), stof(tokens[2]), stof(tokens[3]));
}

bool checkDEF(const pugi::xml_node &node, vector <shared_ptr<Light>> &lights, vector <shared_ptr<Material>> &materials, vector <shared_ptr<Primitive>> &primitives) {
    pugi::xml_attribute attr = node.attribute("DEF");
    if (attr) {
        cout << "\tDEF= " << attr.value() << endl;
        namedNodes.insert({attr.value(), node});
        cout << "\tInserted node: " << node.hash_value() << endl;
        return true;
    }
    return false;
}

// check whether this node refers to another one. Return input node if it doesn't, otherwise referent
const pugi::xml_node& checkUSE(const pugi::xml_node &node, vector <shared_ptr<Light>> &lights, vector <shared_ptr<Material>> &materials, vector <shared_ptr<Primitive>> &primitives) {
    pugi::xml_attribute attr = node.attribute("USE");
    if (attr) {
        cout << "\tUSE= " << attr.value() << endl;
        auto nn = namedNodes.find(attr.value());
        if (nn == namedNodes.end()) {
            cout << "Parse error: USE node " << attr.value() << " not found." << endl;
            return node;
        }
        cout << "\tFound node: " << nn->second.hash_value() << endl;
        return nn->second;
    }
    return node;
}

shared_ptr <Material> processAppearance(const pugi::xml_node &node, vector <shared_ptr<Light>> &lights, vector <shared_ptr<Material>> &materials, vector <shared_ptr<Primitive>> &primitives) {

    return defaultMaterial;
}

void processTransform(const pugi::xml_node &node) {
    // transform can have several different attributes -- these must be applied in the right order.
    // Equivalent tree if nested is:
    // translation
    // center
    // rotation
    // scaleOrientation (rotation)
    // scale
    // reverse scaleOrientation
    // reverseCenter
    bool doCenter = false, doScaleOrientation = false;
    glm::vec3 trans{0.0}, center{0.0}, scale{1.0};
    glm::vec4 rotation{0.0}, scaleOrientation{0.0}; 
    glm::mat4 currentTransform = transformStack.top();

    pugi::xml_attribute attr = node.attribute("translation");
    if (attr) {
        cout << "\ttranslation: " << attr.value() << endl;
        trans = stringToVec3(attr.as_string("0 0 0")); // argument to as_string is default, returned if attribute doesn't exist
        currentTransform = glm::translate(currentTransform, trans);
    }

    attr = node.attribute("center");
    if (attr) {
        cout << "\tcenter: " << attr.value() << endl;
        center = stringToVec3(attr.as_string("0 0 0")); // argument to as_string is default, returned if attribute doesn't exist
        doCenter = true;
        currentTransform = glm::translate(currentTransform, center);
    }

    attr = node.attribute("rotation");
    if (attr) {
        cout << "\trotation: " << attr.value() << endl;
        rotation = stringToVec4(attr.as_string("0 0 0 0")); // argument to as_string is default, returned if attribute doesn't exist
        currentTransform = glm::rotate(currentTransform, rotation.w, glm::vec3(rotation));
    }

    attr = node.attribute("scaleOrientation");
    if (attr) {
        cout << "\tscaleOrientation: " << attr.value() << endl;
        scaleOrientation = stringToVec4(attr.as_string("0 0 0 0")); // argument to as_string is default, returned if attribute doesn't exist
        doScaleOrientation = true;
        currentTransform = glm::rotate(currentTransform, scaleOrientation.w, glm::vec3(scaleOrientation));
    }

    attr = node.attribute("scale");
    if (attr) {
        cout << "\tscale: " << attr.value() << endl;
        scale = stringToVec3(attr.as_string("0 0 0")); // argument to as_string is default, returned if attribute doesn't exist
        currentTransform = glm::scale(currentTransform, scale);
    }

    // currentTransform = currentTransform * T * C * R * SR * S * -SR * -C
    if (doScaleOrientation) {
        currentTransform = glm::rotate(currentTransform, -scaleOrientation.w, glm::vec3(scaleOrientation));
    }
    if (doCenter) {
        currentTransform = glm::translate(currentTransform, -center);
    }
    transformStack.push(currentTransform);
}

void processShape(const pugi::xml_node &node, vector <shared_ptr<Light>> &lights, vector <shared_ptr<Material>> &materials, vector <shared_ptr<Primitive>> &primitives) {
    // find child Appearance node
    pugi::xml_node c = node.child("Appearance");
    shared_ptr <Material> m;
    if (c) {
        checkDEF(c, lights, materials, primitives);
        c = checkUSE(c, lights, materials, primitives);
        m = processAppearance(c, lights, materials, primitives);
    }
    else {
        cout << "No Appearance found. Applying default." << endl;
        m = defaultMaterial;
    }

    // Now that Appearance is taken care of, figure out which kind of shape this is.
    c = node.child("Box");
    pugi::xml_node nextNode;
    if (c) {
        cout << "Found a Box..." << endl;

        checkDEF(c, lights, materials, primitives);
        c = checkUSE(c, lights, materials, primitives);
        glm::vec3 boxSize = stringToVec3(c.attribute("size").as_string("1 1 1")); // argument to as_string is default, returned if attribute doesn't exist
        cout << "\tboxSize: (" << boxSize.x << ", " << boxSize.y << ", " << boxSize.z << ")" << endl;
        shared_ptr <Box> box = make_shared <Box> (boxSize, m);
        primitives.push_back(make_shared <TransformedPrimitive> (transformStack.top(), box));
        return;
    }

    c = node.child("Cone");
    if (c) {
        cout << "Found a Cone..." << endl;
        checkDEF(c, lights, materials, primitives);
        c = checkUSE(c, lights, materials, primitives);
        cout << "Post USE in Cone" << endl;
        if (c.empty()) cout << "c is empty." << endl;
        float bottomRadius = stof(c.attribute("bottomRadius").as_string("1"));
        cout << "Post bottomRadius" << endl;
        float height = stof(c.attribute("height").as_string("2"));
        cout << "Post height" << endl;

        cout << "\tbottomRadius: " << bottomRadius << ", height: " << height << endl;
        shared_ptr <Cone> cone = make_shared <Cone> (bottomRadius, height, m);
        primitives.push_back(make_shared <TransformedPrimitive> (transformStack.top(), cone));
        return;
    }

    c = node.child("Cylinder");
    if (c) {
        cout << "Found a Cylinder..." << endl;
        checkDEF(c, lights, materials, primitives);
        c = checkUSE(c, lights, materials, primitives);
        
        float radius = stof(c.attribute("radius").as_string("1"));
        float height = stof(c.attribute("height").as_string("2"));
        cout << "\tradius: " << radius << ", height: " << height << endl;
        shared_ptr <Cylinder> cyl = make_shared <Cylinder> (radius, height, m);
        primitives.push_back(make_shared <TransformedPrimitive> (transformStack.top(), cyl));
        return;
    }

    c = node.child("Sphere");
    if (c) {
        cout << "Found a Sphere..." << endl;
        checkDEF(c, lights, materials, primitives);
        c = checkUSE(c, lights, materials, primitives);
        
        float radius = stof(c.attribute("radius").as_string("1"));
        cout << "\tradius: " << radius << endl;
        shared_ptr <Sphere> sphere = make_shared <Sphere> (radius, m);
        primitives.push_back(make_shared <TransformedPrimitive> (transformStack.top(), sphere));
        return;
    }

    cout << "ERROR: Shape has no geometry child." << endl;
}

void processMaterial(const pugi::xml_node &node, vector <shared_ptr<Light>> &lights, vector <shared_ptr<Material>> &materials, vector <shared_ptr<Primitive>> &primitives) {
}



void process(const pugi::xml_node &node, vector <shared_ptr<Light>> &lights, vector <shared_ptr<Material>> &materials, vector <shared_ptr<Primitive>> &primitives) {
    // different actions for each node type
    const char *nodeName = node.name();
    cout << "Node name: " << node.name() << endl;
    // In my implementation, the only leaf nodes are of type Shape

    pugi::xml_attribute attr;

    checkDEF(node, lights, materials, primitives);
    pugi::xml_node nextNode = checkUSE(node, lights, materials, primitives);
    cout << "node: " << node.hash_value() << "\tnextNode: " << nextNode.hash_value() << endl;
    if (nextNode != node) {
        process(nextNode, lights, materials, primitives);
        return;
    }

    if (!strcmp(nodeName, "Shape")) {
        cout << "Processing Shape... \n";
        processShape(node, lights, materials, primitives);
        return;
    }

    else if (!strcmp(nodeName, "Transform")) {
        cout << "Processing Transform... \n";
        processTransform(node);
        for (auto c : node.children()) 
            process(c, lights, materials, primitives);
        transformStack.pop();
        return;
    }

    else if (!strcmp(nodeName, "Group")) {
        cout << "Processing Group... \n";
    }

    else if (!strcmp(nodeName, "Scene")) {
        cout << "Processing Scene... \n";
    }
        
    else {
        // Unknown node type, don't go down any further
        cout << "Node type " << nodeName << " unknown." << endl;
        return;
    }

    for (auto c : node.children()) 
        process(c, lights, materials, primitives);
}

unique_ptr <Scene> Parser::parseX3D(std::string fileName) {
    vector <shared_ptr<Light>> lights;
    vector <shared_ptr<Material>> materials;
    vector <shared_ptr<Primitive>> primitives;
    unique_ptr <Scene> ans;
    transformStack.push(glm::mat4(1.0)); // start with identity matrix on the stack
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(fileName.c_str());
    if (result) {
        cout << "XML [" << fileName << "] parsed without errors." << endl;
    } else {
        std::cout << "XML [" << fileName << "] parsed with errors.\n"; 
        std::cout << "Error description: " << result.description() << "\n";
        std::cout << "Error offset: " << result.offset << "\n\n";
    }

    simple_walker walker;
    doc.traverse(walker);
    auto sceneNode = doc.child("X3D").child("Scene");
    if (!sceneNode) {
        cout << "Scene not found." << endl;
        return Scene::emptyScene();
    }

    process(sceneNode, lights, materials, primitives);

    return std::make_unique <Scene> (std::move(lights), std::move(materials), std::move(primitives));
}

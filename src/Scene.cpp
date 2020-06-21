#include "common.hpp"


#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

#include "Color.hpp"
#include "Mesh.hpp"
#include "Scene.hpp"

using namespace std;
using namespace glm;

void Scene::addMesh(const std::shared_ptr<Mesh>& m) {
    this->addPrimitives(m->toPrimitives());
    this->addMaterials(m->getMaterials());
}
/*
void Scene::addMeshInstance(const shared_ptr<Mesh>& mptr, const BBox& dest) {
    // compute transform matrix from current mesh bbox to desired bbox (no rotation)
    // cout << "srcSize: " << srcSize << "\tdestSize: " << destSize << endl;
    Real scale = glm::length(dest.max - dest.min);
    addMeshInstance(mptr, dest.getCentroid(), scale);
}
*/
void Scene::addMeshInstance(const shared_ptr<Mesh>& mptr, const BBox& dest, const Vec3 &axis, const Real angle) {
    Real scale = glm::length(dest.max - dest.min);
    addMeshInstance(mptr, dest.getCentroid(), scale, axis, angle);
}

void Scene::addMeshInstance(const shared_ptr<Mesh>& mptr, const Vec3 center, const Real finalSize, const Vec3 &axis, const Real angle) {
    BBox src = mptr->getBBox();
    cout << "Starting bbox: " << src << endl;
    Real srcSize = glm::length(src.max-src.min);
    Real scale = finalSize / srcSize;
    Mat4 rot = glm::rotate(angle, axis);
    Mat4 trans = glm::translate(Mat4(1.0), center - scale * src.getCentroid());
    //Mat4 trans(1.0);
    // cout << glm::to_string(trans) << endl;
    Mat4 scaleMat = glm::scale(Mat4(1.0), Vec3(scale));
    Mat4 xform;
    if (axis.length() > EPSILON && abs(angle) > EPSILON) {
        xform = trans * rot * scaleMat;
    }
    else {
        xform = trans * scaleMat;
    }
    //Mat4 xform = trans;
    cout << glm::to_string(xform) << endl;

    shared_ptr<MeshInstance> miptr = make_shared <MeshInstance> (mptr, xform);
    
    this->addPrimitives(miptr->toPrimitives());  
    this->addMaterials(miptr->getMaterials());
}


void Scene::printInfo() const {
    cout << "Lights: " << lights.size() << endl;
    cout << "Materials: " << materials.size() << endl;
    cout << "Primitives: " << primitives.size() << endl;
}


/*
void parseError(const string& line) {
    cout << "Parse error: " << line << endl;
}

const Vec3 readVec3(const vector <string> &tokens, Real defaultValue = 1.0, int numTokens = -1) {
    if (numTokens == -1) {
        numTokens = tokens.size();
    }
    Real x, y, z;
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
    return Vec3(x, y, z);
}

const Vec4 readVec4(const vector <string> &tokens, const Real defaultValue = 1.0, int numTokens = -1) {
    if (numTokens == -1) {
        numTokens = tokens.size();
    }
    const Vec3 xyz = readVec3(tokens, defaultValue, numTokens);
    Real w = defaultValue;
    if (numTokens > 4) {
        w = stof(tokens[4]);
    }
    return Vec4(xyz, w);
}

vector <MTLMat> Scene::parseMtl(string fileName) {
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

shared_ptr <Mesh> Scene::parseObj(string fileName) {
    vector <Vec4> vertices;
    vector <Vec3> normals;
    vector <Vec3> texCoords;
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
                vector <SimpleMaterial> tv2 (tv.begin(), tv.end()); // convert MTLMats to SimpleMaterials
                for (auto m : tv2) {
                    materials.push_back(make_shared<Material>(m));
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
        materials.push_back(make_shared<Material> (SimpleMaterial(Color::Green())));
    }

    // postprocess tris -- normal smoothing

    map <int, vector <Tri *>> smoothingGroupsToProcess;
    for (auto& t : tris) {
        if (!t.explicitNormals) {
            // make a new vertex normal based on just this face and assign its index to vn[0->3]
            Vec3 e1 = vertices[t.v[1]] - vertices[t.v[0]];
            Vec3 e2 = vertices[t.v[2]] - vertices[t.v[1]];
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
        map<int, Vec3> groupNormals;
        // loop over tris in group
        for (auto t : sgTris) {
            
            //Vec3 e1 = vertices[t->v[1]] - vertices[t->v[0]];
            //Vec3 e2 = vertices[t->v[2]] - vertices[t->v[1]];
            //Vec3 &n = groupNormals[t->s] += glm::cross(e1, e2); // un-normalized face normal
            //Vec3 n = glm::cross(e1, e2); // un-normalized face normal
            
            // if we haven't encountered this group vertex yet, set its accumulated normal to zero
            for (int i = 0; i < 3; ++i) {
                if (groupNormals.find(t->v[i]) == groupNormals.end()) {
                    groupNormals[t->v[i]] = Vec3(0.0);
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
*/

#include <memory>
#include <vector>

#include <glm/vec3.hpp>

#include "Mesh.hpp"
#include "Primitive.hpp"
using namespace std;

void Mesh::scale(float s) {
    for (auto& v : this->vertices)
        v *= s;
}

// Converts the faces in the Mesh to a vector of Primitives (Triangles, in this case)
vector <shared_ptr<Primitive>> Mesh::toPrimitives() const {
    vector <shared_ptr<Primitive>> ans;
    for (auto tri : tris) {
        // convert Tri to Triangle or TriangleWarp
        const glm::vec3& v0 = this->vertices[tri.v[0]];
        const glm::vec3& v1 = this->vertices[tri.v[1]];
        const glm::vec3& v2 = this->vertices[tri.v[2]];

        const glm::vec3& n0 = this->normals[tri.vn[0]];
        const Material& m = *(this->materials[tri.m]);

        if (tri.vn[0] == tri.vn[1] && tri.vn[0] == tri.vn[2]) {
            ans.push_back(make_shared<Triangle> (v0, v1, v2, n0, m));
        }
        else {
            const glm::vec3& n1 = this->normals[tri.vn[1]];
            const glm::vec3& n2 = this->normals[tri.vn[2]];
            ans.push_back(make_shared<TriangleWarp> (v0, v1, v2, n0, n1, n2, m));
        }
    }
    return ans;
}

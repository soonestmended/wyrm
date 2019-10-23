#include <memory>
#include <vector>

#include <glm/vec3.hpp>

#include "Mesh.hpp"
#include "Primitive.hpp"
using namespace std;

// Return vector of primitives from this MeshInstance. Vertices and normals from the associated Mesh
// are transformed by the MeshInstance transformation matrix.
// If the MeshInstance has its own Material, that is applied to the primtiives. Otherwise it uses the
// Material(s) from the associated Mesh.

std::vector <std::shared_ptr<Primitive>> MeshInstance::toPrimitives() const {
    vector <shared_ptr<Primitive>> ans;
    for (auto tri : meshPtr->tris) {
        // convert Tri to Triangle or TriangleWarp
        glm::vec4 v0 = xform * meshPtr->vertices[tri.v[0]];
        glm::vec4 v1 = xform * meshPtr->vertices[tri.v[1]];
        glm::vec4 v2 = xform * meshPtr->vertices[tri.v[2]];

        glm::vec3 n0 = inv_xform_T * glm::vec4(meshPtr->normals[tri.vn[0]], 0.0);
        Material& m = *(meshPtr->materials[tri.m]);
        if (this->materials.size() == 1) 
            m = *(this->materials[0]);

        if (tri.vn[0] == tri.vn[1] && tri.vn[0] == tri.vn[2]) {
            ans.push_back(make_shared<Triangle> (v0, v1, v2, n0, m));
        }
        else {
            glm::vec3 n1 = inv_xform_T * glm::vec4(meshPtr->normals[tri.vn[1]], 0.0);
            glm::vec3 n2 = inv_xform_T * glm::vec4(meshPtr->normals[tri.vn[2]], 0.0);
            ans.push_back(make_shared<TriangleWarp> (v0, v1, v2, n0, n1, n2, m));
        }
    }
    return ans;
}

void Mesh::scale(float s) {
    for (auto& v : this->vertices)
        v *= s;
}

void Mesh::translate(glm::vec3 dx) {
    glm::vec4 dx4(dx, 1.0);
    for (auto& v : this->vertices)
        v += dx4;
}

void Mesh::recenter(glm::vec3 c) {
    translate(c - this->bbox.getCentroid());
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

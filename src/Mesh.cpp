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
        Vec4 v0 = xform * meshPtr->vertices[tri.v[0]];
        Vec4 v1 = xform * meshPtr->vertices[tri.v[1]];
        Vec4 v2 = xform * meshPtr->vertices[tri.v[2]];

        Vec3 n0 = glm::normalize(inv_xform_T_3x3 * meshPtr->normals[tri.vn[0]]);
        //Vec3 n0 = Vec4(meshPtr->normals[tri.vn[0]], 0.0);
        shared_ptr <Material> mPtr = meshPtr->materials[tri.m];
        if (this->materials.size() == 1) 
            mPtr = this->materials[0];

        if (tri.vn[0] == tri.vn[1] && tri.vn[0] == tri.vn[2]) {
            ans.push_back(make_shared<Triangle> (v0, v1, v2, n0, mPtr));
        }
        else {
            Vec3 n1 = glm::normalize(inv_xform_T_3x3 * meshPtr->normals[tri.vn[1]]);
            Vec3 n2 = glm::normalize(inv_xform_T_3x3 * meshPtr->normals[tri.vn[2]]);
            ans.push_back(make_shared<TriangleWarp> (v0, v1, v2, n0, n1, n2, mPtr));
        }
    }
    return ans;
}
/*
void Mesh::rotate(const Vec3 &axis, const Real angle) {

}

void Mesh::scale(const Real s) {
    for (auto& v : this->vertices)
        v *= s;
    
}

void Mesh::translate(const Vec3 &dx) {
    Vec4 dx4(dx, 1.0);
    for (auto& v : this->vertices)
        v += dx4;
}

void Mesh::recenter(const Vec3 &c) {
    translate(c - this->bbox.getCentroid());
}
*/

// Converts the faces in the Mesh to a vector of Primitives (Triangles, in this case)
vector <shared_ptr<Primitive>> Mesh::toPrimitives() const {
    vector <shared_ptr<Primitive>> ans;
    for (auto tri : tris) {
        // convert Tri to Triangle or TriangleWarp
        const Vec3& v0 = this->vertices[tri.v[0]];
        const Vec3& v1 = this->vertices[tri.v[1]];
        const Vec3& v2 = this->vertices[tri.v[2]];

        const Vec3& n0 = this->normals[tri.vn[0]];
        const shared_ptr <Material> mPtr = this->materials[tri.m];

        if (tri.vn[0] == tri.vn[1] && tri.vn[0] == tri.vn[2]) {
            ans.push_back(make_shared<Triangle> (v0, v1, v2, n0, mPtr));
        }
        else {
            const Vec3& n1 = this->normals[tri.vn[1]];
            const Vec3& n2 = this->normals[tri.vn[2]];
            ans.push_back(make_shared<TriangleWarp> (v0, v1, v2, n0, n1, n2, mPtr));
        }
    }
    return ans;
}

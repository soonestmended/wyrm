#include <vector>

#include "Mesh.hpp"
#include "Primitive.hpp"
using namespace std;

// Converts the faces in the Mesh to a vector of Primitives (Triangles, in this case)
const vector <Primitive> Mesh::toPrimitives() const {
    vector <Primitive> ans;
    for (auto tri : tris_) {
        // convert Tri to Triangle
        
    }
    return ans;
}

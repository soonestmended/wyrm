#pragma once

#include "BBox.hpp"
#include "Material.hpp"

struct Tri {
    int v[3];   // vertex indices
    int vt[3];  // texCoord indices
    int vn[3];  // vertex normal indices
    int m;      // MTLmaterial index
    int s;      // smoothing group
};

class Primitive {
private:
    BBox bbox_;
    const Material& material_;

public:
    bool intersect();
    bool intersectYN();
    const Material& getMaterial();
    const BBox& getBBox();

};
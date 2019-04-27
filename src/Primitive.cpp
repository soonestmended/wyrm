#include "Primitive.hpp"

const Material& Primitive::getMaterial() const {
    return material_;
}

const BBox& Primitive::getBBox() const {
    return bbox_;
}

// TODO: Implement intersection.

bool Triangle::intersect() const {
    return false;
}

bool Triangle::intersectYN() const {
    return false;
}
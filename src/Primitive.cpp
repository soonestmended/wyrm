#include "Primitive.hpp"

const Material& Primitive::getMaterial() const {
    return material_;
}

const BBox& Primitive::getBBox() const {
    return bbox_;
}


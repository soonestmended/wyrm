#pragma once

#include <memory>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "BBox.hpp"
#include "IntersectRec.hpp"
#include "Material.hpp"
#include "Utils.hpp"

class XForm {
public:
    XForm(const Mat4& xf) : xform (xf), inv_xform (glm::inverse(xf)) {}

    Vec3 transformPoint(const Vec3& p) const {
        return Vec3(xform * Vec4(p, 1.0));
    }

    Vec3 inverseTransformPoint(const Vec3& p) const {
        return Vec3(inv_xform * Vec4(p, 1.0));
    }

    Vec3 transformVector(const Vec3& v) const {
        return Vec3(xform * Vec4(v, 0.0));
    }

    Vec3 inverseTransformVector(const Vec3& v) const {
        return Vec3(inv_xform * Vec4(v, 0.0));
    }

    Vec3 transformNormal(const Vec3& n) const {
        return Vec3(glm::transpose(inv_xform) * Vec4(n, 0.0));
    }

    Vec3 inverseTransformNormal(const Vec3& n) const {
        return Vec3(glm::transpose(xform) * Vec4(n, 0.0));
    }

    Ray transformRay(const Ray& ray) const {
        return Ray{transformPoint(ray.o), transformVector(ray.d)};
    }

    Ray inverseTransformRay(const Ray& ray) const {
        return Ray{inverseTransformPoint(ray.o), inverseTransformVector(ray.d)};
    }

    BBox transformBBox(const BBox& bbox) const {
        std::cout << bbox.toString() << " becomes ";

        // transform all 8 vertices, then re-enclose
        BBox ans;
        Vec3 verts[8] = {
            Vec3(bbox.min[0], bbox.min[1], bbox.min[2]),
            Vec3(bbox.min[0], bbox.min[1], bbox.max[2]),
            Vec3(bbox.min[0], bbox.max[1], bbox.min[2]),
            Vec3(bbox.min[0], bbox.max[1], bbox.max[2]),

            Vec3(bbox.max[0], bbox.min[1], bbox.min[2]),
            Vec3(bbox.max[0], bbox.min[1], bbox.max[2]),
            Vec3(bbox.max[0], bbox.max[1], bbox.min[2]),
            Vec3(bbox.max[0], bbox.max[1], bbox.max[2]),
        };
        for (auto v : verts) 
            ans.enclose(transformPoint(v));
        std::cout << ans.toString() << std::endl;
        return ans;
    }

    BBox inverseTransformBBox(const BBox& bbox) const {
        // transform all 8 vertices, then re-enclose
        BBox ans;
        Vec3 verts[8] = {
            Vec3(bbox.min[0], bbox.min[1], bbox.min[2]),
            Vec3(bbox.min[0], bbox.min[1], bbox.max[2]),
            Vec3(bbox.min[0], bbox.max[1], bbox.min[2]),
            Vec3(bbox.min[0], bbox.max[1], bbox.max[2]),

            Vec3(bbox.max[0], bbox.min[1], bbox.min[2]),
            Vec3(bbox.max[0], bbox.min[1], bbox.max[2]),
            Vec3(bbox.max[0], bbox.max[1], bbox.min[2]),
            Vec3(bbox.max[0], bbox.max[1], bbox.max[2]),
        };
        for (auto v : verts) 
            ans.enclose(inverseTransformPoint(v));
        return ans;
    }

    const Mat4 xform, inv_xform;
};
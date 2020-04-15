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
    XForm(const glm::mat4& xf) : xform (xf), inv_xform (glm::inverse(xf)) {}

    glm::vec3 transformPoint(const glm::vec3& p) const {
        return glm::vec3(xform * glm::vec4(p, 1.0));
    }

    glm::vec3 inverseTransformPoint(const glm::vec3& p) const {
        return glm::vec3(inv_xform * glm::vec4(p, 1.0));
    }

    glm::vec3 transformVector(const glm::vec3& v) const {
        return glm::vec3(xform * glm::vec4(v, 0.0));
    }

    glm::vec3 inverseTransformVector(const glm::vec3& v) const {
        return glm::vec3(inv_xform * glm::vec4(v, 0.0));
    }

    glm::vec3 transformNormal(const glm::vec3& n) const {
        return glm::vec3(glm::transpose(inv_xform) * glm::vec4(n, 0.0));
    }

    glm::vec3 inverseTransformNormal(const glm::vec3& n) const {
        return glm::vec3(glm::transpose(xform) * glm::vec4(n, 0.0));
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
        glm::vec3 verts[8] = {
            glm::vec3(bbox.min[0], bbox.min[1], bbox.min[2]),
            glm::vec3(bbox.min[0], bbox.min[1], bbox.max[2]),
            glm::vec3(bbox.min[0], bbox.max[1], bbox.min[2]),
            glm::vec3(bbox.min[0], bbox.max[1], bbox.max[2]),

            glm::vec3(bbox.max[0], bbox.min[1], bbox.min[2]),
            glm::vec3(bbox.max[0], bbox.min[1], bbox.max[2]),
            glm::vec3(bbox.max[0], bbox.max[1], bbox.min[2]),
            glm::vec3(bbox.max[0], bbox.max[1], bbox.max[2]),
        };
        for (auto v : verts) 
            ans.enclose(transformPoint(v));
        std::cout << ans.toString() << std::endl;
        return ans;
    }

    BBox inverseTransformBBox(const BBox& bbox) const {
        // transform all 8 vertices, then re-enclose
        BBox ans;
        glm::vec3 verts[8] = {
            glm::vec3(bbox.min[0], bbox.min[1], bbox.min[2]),
            glm::vec3(bbox.min[0], bbox.min[1], bbox.max[2]),
            glm::vec3(bbox.min[0], bbox.max[1], bbox.min[2]),
            glm::vec3(bbox.min[0], bbox.max[1], bbox.max[2]),

            glm::vec3(bbox.max[0], bbox.min[1], bbox.min[2]),
            glm::vec3(bbox.max[0], bbox.min[1], bbox.max[2]),
            glm::vec3(bbox.max[0], bbox.max[1], bbox.min[2]),
            glm::vec3(bbox.max[0], bbox.max[1], bbox.max[2]),
        };
        for (auto v : verts) 
            ans.enclose(inverseTransformPoint(v));
        return ans;
    }

    const glm::mat4 xform, inv_xform;
};
#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

#include "common.hpp"
#include "Ray.hpp"

class Camera {
public:
	Camera() =delete;

	Camera(const Vec3& _eye, const Vec3& _look, const Vec3& _up, const Vec3& _right, Real _s = 1.0) :
		eye (_eye), look (_look), up (_up), right (_right), s (_s) {
      ar = glm::length(right);
    }

  // Generate ray through the image plane, which goes from -screenWindow --> +screenWindow along
  // the short axis and is set proportionally along the long axis.
  // whole image plane is covered by sampling [-1, -1] x [1, 1]
  
  
  virtual Ray getRay(const Vec2& center) const {
    
    //return Ray(eye, glm::normalize(2.*center[0] * right + 2.*center[1] * up + s * look));
    Vec3 target_world = Vec3(camera2world * Vec4(2. * center[0] * ar, 2. * center[1], s, 1));
    Vec3 origin_world = Vec3(camera2world * Vec4(0, 0, 0, 1));
    return Ray(origin_world, glm::normalize(target_world - origin_world));
  }

  Mat4 camera2world;
  
protected:
  Vec3 eye, look, up, right;
  Real s, ar;	// distance to image plane, aspect ratio
  
  
};

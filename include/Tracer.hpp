#pragma once

#include "Accelerator.hpp"
#include "Color.hpp"
#include "Ray.hpp"
#include "Scene.hpp"

class SampleGenerator;

class Tracer {
public:
  Tracer(const Scene* s, const Accelerator* a, SampleGenerator* _sg) : scene (s), accel (a), sg(_sg) {}
  virtual const Color lightAlongRay(const Ray& r, const bool debug = false) const = 0;
  const Color estimateDirectLighting(const Vec3& wo_world, IntersectRec& ir, bool debug = false) const;
  const Color EDLOneLight(const Vec3& wo_world, IntersectRec& ir, const Light& l, bool debug = false) const;

  
  const Scene* scene;
  const Accelerator* accel;
  SampleGenerator* sg;
};

class PathTracer : public Tracer {
public:
  PathTracer(const Scene* s, const Accelerator* a, SampleGenerator* sg, int _maxDepth) : Tracer (s, a, sg), maxDepth (_maxDepth) {}
    const Color lightAlongRay(const Ray& r, const bool debug = false) const;
  int maxDepth;
};

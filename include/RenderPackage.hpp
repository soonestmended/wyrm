#pragma once

#include "PBRTParser.hpp"

class RenderPackage {
public:
  RenderPackage();
  static void go(const Options& o, const Scene& s);
  /*
  static std::shared_ptr <Accelerator> accel;
  static std::shared_ptr <Image> image;
  static std::shared_ptr <Renderer> renderer;
  static std::shared_ptr <Tracer> tracer;
  */
};


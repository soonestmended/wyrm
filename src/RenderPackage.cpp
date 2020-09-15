#include "BVH.hpp"
#include "Renderer.hpp"
#include "RenderPackage.hpp"

#include <chrono>

using namespace std;


void RenderPackage::go(const Options& o, const Scene& s) {

  // Accelerator
  //  shared_ptr <const Accelerator> accel = make_shared <BVH> (s);
  const BVH bvh{s};
  // Accelerator a{s};
  // Tracer
  if (o.tracerType != "path") {
    cerr << "Error: only path tracer supported at this time." << endl;
  }
 
  //PathTracer pt(&s, accel.get(), o.tracerMaxDepth);
  //SampleGenerator sg(Vec2(0), Vec2(1), o.tracerMaxDepth*20);
  //sg.generate();
  PathTracer pt(&s, &bvh, o.tracerMaxDepth);
  // Sample Generator
  //  if (o.samplerType == "stratified") {
    
  

  // Image
  Image foo(o.imageWidth, o.imageHeight);
  cout << "Image constructed." << endl;


  // Camera
  Vec3 right = Vec3(o.camera2world * Vec4(1, 0, 0, 0));
  Vec3 up = Vec3(o.camera2world * Vec4(0, 1, 0, 0));
  Vec3 look = Vec3(o.camera2world * Vec4(0, 0, 1, 0));
  Vec3 eye = Vec3(o.camera2world * Vec4(0, 0, 0, 1));;
  Real cam_s;

  Real far = o.frameAspectRatio;
  if (far <= 0.) {
    // Calculate from image dimensions (default)
    far = (Real) o.imageWidth / (Real) o.imageHeight;
  }
  cout << "far: " << far << endl;
  right = glm::normalize(right) * far;
  cout << "right: " << right << endl;
  cam_s = 1./tan(0.5 * o.fov * M_PI / 180.);
  
  Camera c(eye, look, up, right, cam_s);
  c.camera2world = o.camera2world;
  cout << "Starting render. " << StratifiedSampleGenerator::calculateN(o.spp) << " samples per pixel, " << o.nt << " threads." << endl;
  
#ifdef DEBUG
  DebugRenderer dbr(c, s, pt);
  dbr.render();
#else
  MultiThreadRenderer mtr(c, s, pt, foo, o.spp, o.nt);
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();  
  mtr.render();
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();  
  cout << endl << "Render complete." << endl;
  std::chrono::milliseconds rt_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  cout << utils::format_duration(rt_ms) << endl;
#endif    
  //  if (vm.count("output_file")) {
    foo.writePNG(o.imageFilename.c_str());
    //}
  
}

#include "BVH.hpp"
#include "Renderer.hpp"
#include "RenderPackage.hpp"

using namespace std;

void RenderPackage::go(const Options& o, const Scene& s) {

  // Accelerator
  shared_ptr <Accelerator> accel = make_shared <BVH> (s);
  if (!accel->build()) {
    cout << "Error building BVH." << endl;
    exit(1);
  }
  else {
    cout << "BVH build complete." << endl;
  }

  // Tracer
  if (o.tracerType != "path") {
    cerr << "Error: only path tracer supported at this time." << endl;
  }
 
  PathTracer pt(&s, accel.get(), o.tracerMaxDepth);
  cout << "Path tracer constructed." << endl;
  Image foo(o.imageWidth, o.imageHeight);
  cout << "Image constructed." << endl;

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
  cout << "Starting render. " << o.spp << " samples per pixel requested, " << o.nt << " threads." << endl;
  
#ifdef DEBUG
  DebugRenderer dbr(c, s, pt);
  dbr.render();
#else
  MultiThreadRenderer mtr(c, s, pt, foo, o.spp, o.nt);
  mtr.render();
#endif    
  //  if (vm.count("output_file")) {
    foo.writePNG(o.imageFilename.c_str());
    //}
  
}

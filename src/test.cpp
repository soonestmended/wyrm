#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

// #include <boost/program_options.hpp>

#include "Accelerator.hpp"
#include "BVH.hpp"
#include "Camera.hpp"
#include "Distribution.hpp"
#include "Image.hpp"
#include "Material.hpp"
#include "Parser.hpp"
#include "PBRTParser.hpp"
#include "Renderer.hpp"
#include "RenderPackage.hpp"
#include "SampleGenerator.hpp"
#include "Scene.hpp"
#include "Tracer.hpp"

using namespace std;
// namespace po = boost::program_options;

map <string, string> parseCommandLine(int argc, char** argv) {
  map <string, string> ans;
  for (int i = 1; i < argc; ++i) {
    string opt{argv[i]};
    if (opt[0] == '-') { // this is an option
      if (i+1 < argc && argv[i+1][0] != '-') { // this is an option with an argument
        ans.insert({opt, string{argv[i+1]}});
        i++;
      }
      else {
        ans.insert({opt, ""});
      }
    }
    else {
      ans.insert({"-i", opt});
    }
  }
  return ans;
}

int main (int argc, char ** argv) {
/*
    // named options
    po::options_description desc("Options"); 
    desc.add_options() 
      ("help", "Print help message") 
      ("input_file,i", po::value<string>(), "input file -- or specify at end")
      ("output_file,o", po::value<string>(), "output file")
      ("quick_render,q", "quick render")
      ("spp,s", po::value<int>(), "samples per pixel")
      ("width,w", po::value<int>(), "width of output image")
      ("height,h", po::value<int>(), "height of output image")
      ("threads,j", po::value<int>(), "number of threads to use; 0 for all available");

    // positional options
    po::positional_options_description positionalOptions; 
    positionalOptions.add("input_file", 1);

    // parse command line into vm
    po::variables_map vm; 
    po::store(po::command_line_parser(argc, argv).options(desc) 
            .positional(positionalOptions).run(), 
          vm); 
    po::notify(vm); 
*/
  
  string desc = "Usage: " + string{argv[0]} + " [-w width] [-h height] [-s samples_per_pixel] [-j num_threads] [-o output_filename] input_file";
  
  Options o;

  map <string, string> clo = parseCommandLine(argc, argv);

  if (clo.find("-help") != clo.end() || clo.find("--help") != clo.end()) {
    cout << desc << "\n";
    return 1;
  }

  auto it = clo.find("-i");
  if (it == clo.end()) {
    cout << "Error: No input file provided." << endl;
    return 0;
  }

  PBRTParser pbrtp;
  cout << "pre 91" << endl;
  unique_ptr <Scene> s = pbrtp.parse(it->second, o);
  cout << "post 91" << endl;
  if (s != nullptr) {
    cout << "Parse successful." << endl;
  }
  else {
    cout << "Parse failed." << endl;
    exit(0);
  }
  s->printInfo();

  it = clo.find("-o");
  if (it != clo.end()) {
    o.imageFilename = it->second;
  }

  it = clo.find("-w");
  if (it != clo.end()) {
    o.imageWidth = stoi(it->second);
  }

  it = clo.find("-h");
  if (it != clo.end()) {
    o.imageHeight = stoi(it->second);
  }

  it = clo.find("-s");
  if (it != clo.end()) {
    o.spp = stoi(it->second);
  }

  it = clo.find("-j");
  if (it != clo.end()) {
    o.nt = stoi(it->second);
  } else {
    o.nt = 0;
  }

  if (o.nt == 0) {
    o.nt = std::thread::hardware_concurrency();
  } 

  it = clo.find("-rt");
  if (it != clo.end()) {
      // interactive (Real Time) mode
      o.interactiveMode = true;
  }


  //RenderPackage::go(o, *s);
/*
  vector <Real> testInput = {.05, .7, .08, .02, 0.07, .08};
  Distribution1D d1d(testInput);
  int N = 100;
  Real pdf;
  for (int i = 0; i < N; i++) {
    Real u = (Real) rand() / (Real) RAND_MAX;
    cout << d1d.sampleContinuous(u, &pdf) << " (" << d1d.sampleDiscrete(u, &pdf) << ")" << endl;
  }
*/
//  Image ss{"imageTexture.png"};
//  Distribution2D d2d{ss};

//  int N = 100;
//  Real pdf;
//  for (int i = 0; i < N; i++) {
//    Vec2 uv = Vec2{(Real) rand() / (Real) RAND_MAX, (Real) rand() / (Real) RAND_MAX};
//    cout << "uv: " << uv << "\t\t";
//    cout << d2d.sampleContinuous(uv, &pdf) << " [" << pdf << "]\t" << d2d.sampleDiscrete(uv, &pdf) << " [" << pdf << "]" << endl;
//  }
  //Image foo{"cb.png"};
  //foo.boxBlur(5, 100);
  //foo.writePNG("cb_blur.png");

  RenderPackage::go(o, *s);
/*
  Image foo{10, 10};
  for (int j = 0; j < 10; j++) {
    for (int i = 0; i < 10; i++) {
      if (i > 3 && i < 6 && j > 3 && j < 6) foo(i, j) = Color{10};
      else foo(i, j) = Color{.01};
    }
  }
  Distribution2D d2d{foo};
  Real pdf;
  for (int i = 0; i < 200; i++) {
    Vec2 uv{(Real) rand() / (Real) RAND_MAX, (Real) rand() / (Real) RAND_MAX};
    Vec2 xy = d2d.sampleContinuous(uv, &pdf);
    int ix = utils::clamp(xy[0] * foo.width(), 0, foo.width() - 1);
    int iy = utils::clamp(xy[1] * foo.height(), 0, foo.height() - 1);
    //int ix = xy[0] * (foo.width() - 1);
    //int iy = xy[1] * (foo.height() - 1);
    cout << xy << "\tpdf: " << pdf << "\t(ix, iy): (" << ix << ", " << iy << ")\tcolor: " << foo(ix, iy) << endl;
  }
*/
}

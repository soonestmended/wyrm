#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <boost/program_options.hpp>

#include "Accelerator.hpp"
#include "BVH.hpp"
#include "Camera.hpp"
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
namespace po = boost::program_options;

int main (int argc, char ** argv) {
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

    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }
    if (!vm.count("input_file")) {
        cout << "Error: No input file provided." << endl;
        return 0;
    }

    Options o;
    //    unique_ptr <Scene> s = Parser::parseX3D(vm["input_file"].as<string>().c_str(), c);
    PBRTParser pbrtp;
    unique_ptr <Scene> s = pbrtp.parse(vm["input_file"].as<string>().c_str(), o);
//    unique_ptr <Scene> s = Scene::emptyScene();

    //s->addMesh(Scene::parseObj(vm["input_file"].as<string>()));

    if (s != nullptr) {
      cout << "Parse successful." << endl;
    }
    else {
      cout << "Parse failed." << endl;
      exit(0);
    }
    s->printInfo();

    if (vm.count("output_file")) {
      o.imageFilename = vm["output_file"].as<string>();
    }


    if (vm.count("width")) {
      o.imageWidth = vm["width"].as<int>();
    }
    
    if (vm.count("height")) {
      o.imageHeight = vm["height"].as<int>();
    }
    
    if (vm.count("spp")) {
      o.spp = vm["spp"].as<int>();
    }
    
    if (vm.count("threads")) {
      o.nt = vm["threads"].as<int>();
    }
    else {
      o.nt = 0;
    }
    
    if (o.nt == 0) {
      o.nt = std::thread::hardware_concurrency();
    }

    //BBox wrapper(Vec3(2.0, -2.0, -2.0), Vec3(6.0, 2.0, 2.0));
    //s->addMeshInstance(Parser::parseObj("bunny.obj"), wrapper, Vec3(0, 1, 0), 180);
 
/*
    shared_ptr <Light> l = make_shared <PointLight> (Vec3(0.0, 0.0, -10.0), Color::White(), 200.0);
    vector <shared_ptr<Light>> bar;
    bar.push_back(l);
    bar.push_back(make_shared <PointLight> (Vec3(0.0, 10.0, -2.0), Color::White(), 200.0));
    s->addLights(bar);
*/
    // for now just implement quick render
    //    shared_ptr <Camera> c = make_shared <Camera> (Vec3(0, 0, -10), Vec3(0, 0, 1), Vec3(0, 1, 0), Vec3(1, 0, 0), 1);

    RenderPackage::go(o, *s);
    
}

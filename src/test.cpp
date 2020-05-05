#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "Accelerator.hpp"
#include "BVH.hpp"
#include "Camera.hpp"
#include "Image.hpp"
#include "Material.hpp"
#include "Parser.hpp"
#include "Renderer.hpp"
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
      ("samples_per_pixel,s", po::value<int>(), "samples per pixel")
      ("width,w", po::value<int>(), "width of output image")
      ("height,h", po::value<int>(), "height of output image");

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
    shared_ptr <Camera> c = nullptr;
    unique_ptr <Scene> s = Parser::parseX3D(vm["input_file"].as<string>().c_str(), c);
    
//    unique_ptr <Scene> s = Scene::emptyScene();

    //s->addMesh(Scene::parseObj(vm["input_file"].as<string>()));

    cout << "Parse successful." << endl;
    s->printInfo();

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
    if (!c) {
        c = make_shared <Camera> (Vec3(0, 0, -10), Vec3(0, 0, 1), Vec3(0, 1, 0), Vec3(1, 0, 0), 1);
    }

    BVH bvh(*s);
    if (!bvh.build()) {
        cout << "Error building BVH." << endl;
        exit(1);
    }
    else {
        cout << "BVH build complete." << endl;
    }
    Accelerator da(*s);
    PathTracer pt(s.get(), &bvh);
    cout << "Path tracer constructed." << endl;
    Image foo(512, 512);
    cout << "Image constructed." << endl;

//    QuickRenderer qr(*c, *s, pt, foo);

#ifdef DEBUG
    DebugRenderer dbr(*c, *s, pt);
    dbr.render();
#else
    MultisampleRenderer mr(*c, *s, pt, foo, 81);
    mr.render();
#endif
    if (vm.count("output_file")) {
        foo.writePNG(vm["output_file"].as<string>().c_str());
    }
}
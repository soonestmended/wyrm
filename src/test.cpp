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

    unique_ptr <Scene> x3ds = Parser::parseX3D("test.x3d");
    /*
    unique_ptr <Scene> s = Scene::emptyScene();

    BBox wrapper(glm::vec3(-2.0, -2.0, -2.0), glm::vec3(2.0, 2.0, 2.0));
    s->addMeshInstance(Parser::parseObj(vm["input_file"].as<string>()), wrapper, glm::vec3(0, 1, 0), 180);
    //s->addMesh(Scene::parseObj(vm["input_file"].as<string>()));
    if (s == nullptr) {
        cout << "Parse of " << vm["input_file"].as<string>() << " failed. " << endl;
        exit(0);
    }
    cout << "Parse successful." << endl;
    s->printInfo();

    shared_ptr <Light> l = make_shared <PointLight> (glm::vec3(0.0, 0.0, -10.0), Color::White(), 100.0);
    vector <shared_ptr<Light>> bar;
    bar.push_back(l);
    s->addLights(bar);

    // for now just implement quick render
    Camera c(glm::vec3(0, 0, -10), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), 1);

    BVH bvh(*s);
    if (!bvh.build()) {
        cout << "Error building BVH." << endl;
        exit(1);
    }

    DirectLightingShader dlsh(*s, bvh);

    AcceleratedTracer at(*s, dlsh, bvh);

    Image foo(512, 512);

    QuickRenderer qr(c, *s, at, foo);
    qr.render();
    if (vm.count("output_file")) {
        foo.writePNG(vm["output_file"].as<string>().c_str());
    }
*/
}
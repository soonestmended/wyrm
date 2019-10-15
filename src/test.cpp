#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "Camera.hpp"
#include "Image.hpp"
#include "Material.hpp"
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
    
    unique_ptr <Scene> s;
    s = Scene::parseObj(vm["input_file"].as<string>());
    if (s == nullptr) {
        cout << "Parse of " << vm["input_file"].as<string>() << " failed. " << endl;
        exit(0);
    }
    cout << "Parse successful." << endl;
    s->printInfo();

    // for now just implement quick render
    unique_ptr <Camera> c;

    unique_ptr<Tracer> t;

    int foo;

    QuickRenderer qr(*c, *s, *t, foo);
    qr.render();
    Image bar(256, 256);
    if (vm.count("output_file")) {
        bar.writePNG(vm["output_file"].as<string>().c_str());
    }

}
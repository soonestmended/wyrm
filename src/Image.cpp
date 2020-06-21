#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

#include "Image.hpp"

#include <filesystem>

namespace fs = std::filesystem;
using namespace std;

inline Real linearTosRGB(Real value) {
    if (value <= 0.0031308)
        return 12.92 * value;
    return 1.055 * std::pow(value, (Real)(1.f / 2.4f)) - 0.055;
}

inline Real sRGBToLinear(Real value) {
  if (value <= 0.04045)
    return value / 12.92;
  else
    return std::pow((value + 0.055) / 1.055, 2.4);
}

bool Image::writePNG(const std::string& filename) {
//    png::image <png::rgb_pixel> png_image(width_, height_);
	unsigned char uchar_data[width_*height_*3];
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            Color& c = (*this)(x, y);
            int index = 3*((height_-1-y)*width_+x);
            uchar_data[index+0] = linearTosRGB(c.r) * 255;
            uchar_data[index+1] = linearTosRGB(c.g) * 255;
            uchar_data[index+2] = linearTosRGB(c.b) * 255;
        }
    }
	stbi_write_png(filename.c_str(), width_, height_, 3, uchar_data, 3*width_);
    return true;
}

Image::Image(const string& filename) {
//    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
	int n;
    cout << "Attempting to load: " << filename.c_str() << endl;

    // if .exr, use openEXR
    std::filesystem::path ext = fs::path(filename).extension();
    cout << "File extension: " << ext.c_str() << endl;
    if (ext.string() == ".exr" || ext.string() == ".EXR") {
      float* float_data; // width * height * RGBA
      const char* err = NULL; // or nullptr in C++11
      
      int ret = LoadEXR(&float_data, &width_, &height_, filename.c_str(), &err);
      
      if (ret != TINYEXR_SUCCESS) {
        if (err) {
          fprintf(stderr, "ERR : %s\n", err);
          FreeEXRErrorMessage(err); // release memory of error message.
          width_ = height_ = 1;
          data.emplace_back(0.5, 0.5, 0.5);
          return;
        }
      } else {
        data.reserve(width_ * height_);
        for (int row = height_ - 1; row >= 0; row--) {
          for (int col = 0; col < width_; col++) {
            int ind = 4 * (row * width_ + col);
            Real r = float_data[ind];
            Real g = float_data[ind+1];
            Real b = float_data[ind+2];
            data.emplace_back(r, g, b);
          }
        }
        
        free(float_data); // release memory of image data
      }
    }
    else {
      unsigned char* uchar_data = stbi_load(filename.c_str(), &width_, &height_, &n, 3);
      if (uchar_data == NULL) {
		cout << "Error: unable to load image " << filename << endl;
        cout << stbi_failure_reason() << endl;
        width_ = height_ = 1;
        data.emplace_back(0.5, 0.5, 0.5);
		return;
      }
      data.reserve(width_ * height_);
      for (int row = height_ - 1; row >= 0; row--) {
		for (int col = 0; col < width_; col++) {
          int ind = 3 * (row * width_ + col);
          Real r = sRGBToLinear((Real) uchar_data[ind] / 255.);
          Real g = sRGBToLinear((Real) uchar_data[ind+1] / 255.);
          Real b = sRGBToLinear((Real) uchar_data[ind+2] / 255.);
          data.emplace_back(r, g, b);
		}
      }
      stbi_image_free(uchar_data);
    }
}

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

#include "Image.hpp"
#include "Utils.hpp"

#include <filesystem>

namespace fs = std::filesystem;
using namespace std;

inline Real tonemapHDR(Real value) {
  if (value <= 0.0031308)
    return 12.92 * value;
  return 1.055 * std::pow(value, (Real)(1.f / 1.35f)) - 0.055;
}

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
  unsigned char *uchar_data = new unsigned char[width_*height_*3];
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            Color& c = (*this)(x, y);
            int index = 3*(y*width_+x);
            uchar_data[index+0] = utils::clamp(linearTosRGB(c.r), 0, 1) * 255;
            uchar_data[index+1] = utils::clamp(linearTosRGB(c.g), 0, 1) * 255;
            uchar_data[index+2] = utils::clamp(linearTosRGB(c.b), 0, 1) * 255;
        }
    }
    stbi_write_png(filename.c_str(), width_, height_, 3, uchar_data, 3*width_);
    delete[] uchar_data;
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
        for (int row = 0; row < height_; row++) {
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
      for (int row = 0; row < height_; row++) {
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

vector <Real> createGaussian(int kernelWidth, Real sigma) {
  vector <Real> ans;
  ans.reserve(kernelWidth*kernelWidth);
  Real tSSinv = 1. / (2. * sigma * sigma); // one over two sigma squared
  Real tSSPinv = tSSinv / M_PI; // one over (two sigma squared times pi)
  Real sum = 0;
  for (int row = 0; row < kernelWidth; row++) {
    Real y = row - (kernelWidth / 2);
    for (int col = 0; col < kernelWidth; col++) {
      Real x = col - (kernelWidth / 2);
      Real g = tSSPinv * exp(-tSSinv * (x*x+y*y));
      ans.push_back(g);
      sum += g;
    }
  }
  Real scale = 1. / sum;
  for (auto& r : ans)
    r *= scale;
  return ans;
}

void Image::convolve(vector <Real> &kernel) {
  Image image{width(), height()};
  int kw = sqrt(kernel.size());
  for (int row = 0; row < height(); row++) {
    for (int col = 0; col < width(); col++) {
      Color result{0};
      for (int j = 0; j < kw; j++) {
        for (int i = 0; i < kw; i++) {
          int sourceX = col + i - kw/2;
          int sourceY = row + j - kw/2;
          if (sourceX < 0) sourceX = 0;
          else if (sourceX >= width()) sourceX = width() - 1;
          if (sourceY < 0) sourceY = 0;
          else if (sourceY >= height()) sourceY = height() - 1;
          result += (*this)(sourceX, sourceY) * kernel[j*kw+i];
        }
      }
      image(col, row) = result;
    }
  }
  *this = image;
}

void Image::gaussianBlur(Real sigma = 1.0) {
  int radius = sigma * 4;
  int kernelWidth = 2 * radius - 1;
  vector <Real> kernel = createGaussian(kernelWidth, sigma);
//  for (int row = 0; row < kernelWidth; row++) {
//    for (int col = 0; col < kernelWidth; col++) {
//      cout << kernel[row*kernelWidth+col] << " ";
//    }
//    cout << endl;
//  }
  // convolve kernel
  convolve(kernel);
}

void Image::hBoxBlur(int w) {
  if (w % 2 == 0) w++; // make w odd
  vector <Color> rowSums;
  rowSums.reserve(width_ + w*2); // row plus padding
  for (int row = 0; row < height_; row++) {
    // build 1D prefix sums array -- rowSums[i] = sum of entries 0, 1, 2 ... i
    // sum of range [a -> b], is rowSums[b] - rowSums[a-1]
    rowSums[0] = (*this)(0, row);
    for (int i = 1; i < w; i++) {
      rowSums[i] = (*this)(0, row) + rowSums[i-1];
    }
    for (int i = w; i < w+width_; i++) {
      rowSums[i] = (*this)(i-w, row) + rowSums[i-1];
    }
    for (int i = w+width_; i < 2*w+width_; i++) {
      rowSums[i] = (*this)(width_-1, row) + rowSums[i-1];
    }

    for (int col = 0; col < width_; col++) {
      int a = col - w/2 - 1;
      int b = col + w/2;
      (*this)(col, row) = (rowSums[b+w] - rowSums[a+w]) / (Real) w;
    }
  }
}

void Image::vBoxBlur(int w) {
  if (w % 2 == 0) w++; // make w odd
  vector <Color> colSums;
  colSums.reserve(height_ + w*2); // row plus padding
  for (int col = 0; col < width_; col++) {
    // build 1D prefix sums array -- rowSums[i] = sum of entries 0, 1, 2 ... i
    // sum of range [a -> b], is rowSums[b] - rowSums[a-1]
    colSums[0] = (*this)(col, 0);
    for (int i = 1; i < w; i++) {
      colSums[i] = (*this)(col, 0) + colSums[i-1];
    }
    for (int i = w; i < w+height_; i++) {
      colSums[i] = (*this)(col, i-w) + colSums[i-1];
    }
    for (int i = w+height_; i < 2*w+height_; i++) {
      colSums[i] = (*this)(col, height_-1) + colSums[i-1];
    }

    for (int row = 0; row < height_; row++) {
      int a = row - w/2 - 1;
      int b = row + w/2;
      (*this)(col, row) = (colSums[b+w] - colSums[a+w]) / (Real) w;
    }
  }
}


void Image::boxBlur(int w, int n) {
  for (int i = 0; i < n; i++) {
    hBoxBlur(w);
    vBoxBlur(w);
  }
}

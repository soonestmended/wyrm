#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include "Image.hpp"

using namespace std;

inline Real GammaCorrect(Real value) {
    if (value <= 0.0031308)
        return 12.92 * value;
    return 1.055 * std::pow(value, (Real)(1.f / 2.4f)) - 0.055;
}

bool Image::writePNG(const std::string& filename) {
//    png::image <png::rgb_pixel> png_image(width_, height_);
	unsigned char uchar_data[width_*height_*3];
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            Color& c = (*this)(x, y);
            int index = 3*((height_-1-y)*width_+x);
            uchar_data[index+0] = GammaCorrect(c.r) * 255;
            uchar_data[index+1] = GammaCorrect(c.g) * 255;
            uchar_data[index+2] = GammaCorrect(c.b) * 255;
        }
    }
	stbi_write_png(filename.c_str(), width_, height_, 3, uchar_data, 3*width_);
    return true;
}

Image::Image(const string& filename) {
//    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
	int n;
	unsigned char* uchar_data = stbi_load(filename.c_str(), &width_, &height_, &n, 3);
	if (uchar_data == NULL) {
		cout << "Error: unable to load image " << filename << endl;
        cout << stbi_failure_reason() << endl;
		return;
	}
	data.reserve(width_ * height_);
	for (int row = 0; row < height_; row++) {
		for (int col = 0; col < width_; col++) {
			int ind = 3 * (row * width_ + col);
			Real r = (Real) uchar_data[ind] / 255;
			Real g = (Real) uchar_data[ind+1] / 255;
			Real b = (Real) uchar_data[ind+2] / 255;
			data.emplace_back(r, g, b);
		}
	}
	stbi_image_free(uchar_data);
}

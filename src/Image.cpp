#include <png++/png.hpp>

#include "Image.hpp"

inline Real GammaCorrect(Real value) {
    if (value <= 0.0031308)
        return 12.92 * value;
    return 1.055 * std::pow(value, (Real)(1.f / 2.4f)) - 0.055;
}

bool Image::writePNG(const std::string& filename) {
    png::image <png::rgb_pixel> png_image(width_, height_);
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            Color& c = (*this)(x, y);
            unsigned int r = GammaCorrect(c.r) * 255;
            unsigned int g = GammaCorrect(c.g) * 255;
            unsigned int b = GammaCorrect(c.b) * 255;
            png_image[height_-1-y][x] = png::rgb_pixel(r, g, b);
        }
    }
    png_image.write(filename);
    return true;
}

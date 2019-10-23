#include <png++/png.hpp>

#include "Image.hpp"

bool Image::writePNG(const std::string& filename) {
    png::image <png::rgb_pixel> png_image(width_, height_);
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            Color& c = (*this)(x, y);
            unsigned int r = c.r * 255;
            unsigned int g = c.g * 255;
            unsigned int b = c.b * 255;
            png_image[height_-1-y][x] = png::rgb_pixel(r, g, b);
        }
    }
    png_image.write(filename);
    return true;
}
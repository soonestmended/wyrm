#include <png++/png.hpp>

#include "Image.hpp"

bool Image::writePNG(const std::string& filename) {
    png::image <png::rgb_pixel> png_image(width_, height_);
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            png_image[y][x] = png::rgb_pixel(x, y, x + y);
        }
    }
    png_image.write(filename);
    return true;
}
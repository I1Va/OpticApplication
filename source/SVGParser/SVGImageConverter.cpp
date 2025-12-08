#include <vector>
#include <iostream>
#include <cassert>
#include <stdexcept>

#include "SVGParser/SVGImageConverter.hpp"

#define NANOSVG_IMPLEMENTATION
#include "SVGParser/nanosvg.h"

#define NANOSVGRAST_IMPLEMENTATION
#include "SVGParser/nanosvgrast.h"

void ExtractSVG(const std::string &path, dr4::Vec2f pixelSize, std::function<void(int,int,dr4::Color)> putPixel) {
    assert(putPixel);

    NSVGimage* img = nsvgParseFromFile(path.c_str(), "px", 96);
    if (img == nullptr) {
        throw std::invalid_argument("nsvgParseFromFile `" + path + "` failed");
    }

    if (pixelSize.x < 0 || pixelSize.y < 0) {
        throw std::invalid_argument("PixelSize " + std::to_string(pixelSize.x) + " " + std::to_string(pixelSize.y) + " is incorrect");
    }

    int w = static_cast<int>(pixelSize.x);
    int h = static_cast<int>(pixelSize.y);

    std::vector<unsigned char> pixels(w * h * 4);

    NSVGrasterizer* rast = nsvgCreateRasterizer();
    if (rast == nullptr) {
        nsvgDelete(img);
        throw std::runtime_error("nsvgCreateRasterizer failed\n");
    }

    float scale = (float)w / img->width;

    nsvgRasterize
    (
        rast,       
        img,          
        0, 0,         
        scale,        
        pixels.data(),
        w, h,        
        w * 4
    );

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
        int i = (y * w + x) * 4;

        dr4::Color color;
        color.r = pixels[i + 0];
        color.g = pixels[i + 1];
        color.b = pixels[i + 2];
        color.a = pixels[i + 3];
        
        putPixel(x, y, color);
        }
    }

    nsvgDeleteRasterizer(rast);
    nsvgDelete(img);
};
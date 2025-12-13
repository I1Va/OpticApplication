#pragma once
#include <string>
#include <functional>
#include "dr4/math/color.hpp"
#include "dr4/math/vec2.hpp"

void ExtractSVG(const std::string &path, dr4::Vec2f pixelSize, std::function<void(int,int,dr4::Color)> putPixel);
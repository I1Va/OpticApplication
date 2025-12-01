#pragma once
#include <iostream>

#include "dr4/math/color.hpp"
#include "dr4/math/rect.hpp"
#include "dr4/math/vec2.hpp"

namespace roa
{

const dr4::Color BLACK = dr4::Color{0, 0, 0, 255};
const dr4::Color WHITE = dr4::Color{255, 255, 255, 255};


dr4::Vec2f getClampedDotInRect(const dr4::Vec2f dot, const dr4::Rect2f rect);

std::ostream &operator<< (std::ostream &stream, const dr4::Vec2f vec);
std::ostream &operator<< (std::ostream &stream, const dr4::Rect2f rect);

} // namespace roa

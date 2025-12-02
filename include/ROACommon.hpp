#pragma once
#include <iostream>

#include "dr4/math/color.hpp"
#include "dr4/math/rect.hpp"
#include "dr4/math/vec2.hpp"
#include "hui/event.hpp"

namespace roa
{

template <typename T>
concept EventDerived = std::is_base_of_v<hui::Event, T>;

const dr4::Color BLACK = dr4::Color{0, 0, 0, 255};
const dr4::Color GRAY = dr4::Color{60, 60, 60, 255};
const dr4::Color WHITE = dr4::Color{255, 255, 255, 255};

dr4::Vec2f getClampedDotInRect(const dr4::Vec2f dot, const dr4::Rect2f rect);


template <EventDerived T>
inline bool checkEventType(const hui::Event &event) {
    bool result = false;
    try {
        dynamic_cast<const T&>(event);
        result = true;
    } catch(...) {}

    return result;
}

std::ostream &operator<< (std::ostream &stream, const dr4::Vec2f vec);
std::ostream &operator<< (std::ostream &stream, const dr4::Rect2f rect);

} // namespace roa

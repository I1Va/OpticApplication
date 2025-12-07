#include "ROACommon.hpp"

namespace roa
{

dr4::Vec2f getClampedDotInRect(const dr4::Vec2f dot, const dr4::Rect2f rect) {
    dr4::Vec2f result = dot;
    result.x = std::clamp(result.x, rect.pos.x, rect.pos.x + rect.size.x);
    result.y = std::clamp(result.y, rect.pos.y, rect.pos.y + rect.size.y);
    return result;
}

// std::ostream &operator<< (std::ostream &stream, const dr4::Vec2f vec) {
//     stream << "dr4::Vec2f{" << vec.x << ", " << vec.y << "}";
//     return stream;
// }

// std::ostream &operator<< (std::ostream &stream, const dr4::Rect2f rect) {
//     stream << "dr4::Rect2f{pos: " << rect.pos.x << ", " << rect.pos.y << " | size : " << rect.size.x << ", " << rect.size.y << "}";
//     return stream;
// }

} // namespace roa

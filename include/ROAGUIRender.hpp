#pragma once
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <functional>

#include "dr4/math/color.hpp"


namespace roa
{

inline bool insideRounded(int x, int y, int w, int h, int R) {
    if (w <= 0 || h <= 0) return false;
    if (R <= 0) return (x >= 0 && x < w && y >= 0 && y < h);

    // clamp nearest point to the central "non-corner" area
    int nx = std::clamp(x, R, w - R - 1);
    int ny = std::clamp(y, R, h - R - 1);

    int dx = x - nx;
    int dy = y - ny;
    // integer compare avoids sqrt
    return dx*dx + dy*dy <= R * R;
}

inline void drawBlenderRoundedFrame(
        int W, int H,
        int radius,
        int border,
        dr4::Color bd,
        std::function<void(int,int,dr4::Color)> putPixel)
{
    if (W <= 0 || H <= 0) return;
    int outerR = std::max(0, radius);
    int innerW  = W - 2*border;
    int innerH  = H - 2*border;
    int innerR  = std::max(0, radius - border);
    bool hasInner = (innerW > 0 && innerH > 0 && innerR >= 0);

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            if (!insideRounded(x, y, W, H, outerR)) {
                // outside rounded outer -> leave transparent
                putPixel(x, y, FULL_TRANSPARENT);
                continue;
            }

            bool inInner = false;
            if (hasInner) {
                // transform coordinates to inner rect space and test
                inInner = insideRounded(x - border, y - border, innerW, innerH, innerR);
            }

            if (inInner) {
                continue;
            } else {
                putPixel(x, y, bd);
            }
        }
    }
}


}

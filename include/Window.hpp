#pragma once

#include "ROACommon.hpp"
#include "Containers.hpp"

namespace roa
{


inline bool insideRounded(int x, int y, int w, int h, int R) {
    int rx = (x < R) ? R :
             (x >= w - R) ? w - R - 1 : -1;
    int ry = (y < R) ? R :
             (y >= h - R) ? h - R - 1 : -1;

    if (rx == -1 || ry == -1) return true; // straight edge

    int dx = x - rx;
    int dy = y - ry;
    return dx*dx + dy*dy <= R*R;
}

inline void drawBlenderRoundedRect(
        int W, int H,
        int radius,
        int border,
        dr4::Color bg,
        dr4::Color bd,
        std::function<void(int,int,dr4::Color)> putPixel)
{
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {

            bool insideOuter = insideRounded(x, y, W, H, radius);

            bool insideInner = insideRounded(
                x - border,
                y - border,
                W - 2*border,
                H - 2*border,
                radius - border
            );

            if (!insideOuter) {
                continue; // fully transparent outside
            }

            if (!insideInner) {
                putPixel(x, y, bd); // border region
            } else {
                putPixel(x, y, bg); // interior
            }
        }
    }
}


class Window : public LinContainer<hui::Widget> {
    dr4::Image *surface;
public:
    Window(hui::UI *ui): LinContainer(ui), surface(ui->GetWindow()->CreateImage()) {
        assert(ui);
        assert(surface);
    }

    using LinContainer<hui::Widget>::LinContainer;
    virtual ~Window() = default;

protected:
    void Redraw() const override {
        GetTexture().Clear(FULL_TRANSPARENT);
        surface->DrawOn(GetTexture());
    }

    void OnSizeChanged() override {
        relayoutTexture();
    }

private:
    void relayoutTexture() {
        assert(surface);
    
        surface->SetSize(GetSize());

        drawBlenderRoundedRect(
            surface->GetWidth(),
            surface->GetHeight(),
            10,               // radius
            5,                // border thickness
            {60,60,60,255},   // blender bg
            {42,42,42,255},   // blender border
            [&](int x, int y, dr4::Color c) { surface->SetPixel(x,y,c); }
        );
    }
};



} // namespace roa
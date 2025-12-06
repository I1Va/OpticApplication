#pragma once

#include "ROAGUIRender.hpp"
#include "ROACommon.hpp"

#include "Containers.hpp"

namespace roa
{


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

        // drawBlenderRoundedRect(
        //     surface->GetWidth(),
        //     surface->GetHeight(),
        //     10,               // radius
        //     4,                // border thickness
        //     {60,60,60,255},   // blender bgv
        //     {42,42,42,255},   // blender border
        //     [&](int x, int y, dr4::Color c) { surface->SetPixel(x,y,c); }
        // );
    }
};



} // namespace roa
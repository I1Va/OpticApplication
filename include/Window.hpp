#pragma once

#include "ROAGUIRender.hpp"
#include "ROACommon.hpp"

#include "Containers.hpp"

namespace roa
{


class Window : public LinContainer<hui::Widget> {
public:
    using LinContainer::LinContainer;
    virtual ~Window() = default;

protected:
    void Redraw() const override final {
        GetTexture().Clear(FULL_TRANSPARENT);
        
        RedrawSelfAction();
        for (auto &child : children) child->DrawOn(GetTexture());
        
        dr4::Image *backSurface = GetTexture().GetImage();
      

        dr4::Color borderColor = ((GetUI()->GetHovered() == this) ? dr4::Color(55,55,55,255) : dr4::Color(88, 88, 88, 255)); 
        drawBlenderRoundedFrame(
            backSurface->GetWidth(),
            backSurface->GetHeight(),
            10,               // radius
            2,                // border thickness
            borderColor,      // blender border
            [&](int x, int y, dr4::Color c) { backSurface->SetPixel(x,y,c); }
        );

        GetTexture().Clear(FULL_TRANSPARENT);
        backSurface->DrawOn(GetTexture());
    }

    virtual void RedrawSelfAction() const {} 
    void OnSizeChanged() override {}

private:

};



} // namespace roa
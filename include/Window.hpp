#pragma once

#include "ROAGUIRender.hpp"
#include "ROACommon.hpp"

#include "Containers.hpp"

namespace roa
{
 
class Window : public LinContainer<hui::Widget> {
    bool implicitHovered = false;
public:
    using LinContainer::LinContainer;
    virtual ~Window() = default;

protected:
    hui::EventResult OnIdle(hui::IdleEvent &evt) override final {
        PropagateToChildren(evt);

        bool newImplicitHovered = checkImplicitHover();
        if (newImplicitHovered != implicitHovered) ForceRedraw();
        implicitHovered = newImplicitHovered;

        
        return hui::EventResult::UNHANDLED;
    }

    void Redraw() const override final {
        GetTexture().Clear(FULL_TRANSPARENT);
        
        for (auto &child : children) child->DrawOn(GetTexture());
        
        dr4::Image *backSurface = GetTexture().GetImage();
        dr4::Color borderColor = (implicitHovered ? dr4::Color(88, 88, 88, 255) : dr4::Color(55,55,55,255));
        
        DrawBlenderRoundedFrame(
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

private:
    bool checkImplicitHover() const {
        hui::Widget *hover = GetUI()->GetHovered();
        while (hover && (hover != this)) {
            if (hover == hover->GetParent()) break;
            hover = hover->GetParent();
        }

        return hover == this;
    }
};



} // namespace roa
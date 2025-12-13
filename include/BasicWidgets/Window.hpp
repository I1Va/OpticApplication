#pragma once

#include "Utilities/ROAGUIRender.hpp"
#include "Utilities/ROACommon.hpp"

#include "BasicWidgets/Containers.hpp"
#include "DropDownMenu.hpp"

namespace roa
{
 
class Window : public Container  {
    bool implicitHovered = false;
    std::vector<DropDownMenu *> tools;
public:
    static constexpr float TOOL_BAR_HEIGHT = 20;
    static constexpr float TOOL_WIDTH = 60;

    using Container::Container;
    Window(const Window &) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = default;
    Window& operator=(Window&&) = default;
    virtual ~Window() = default;   

    void AddTool(std::unique_ptr<DropDownMenu> menu) {
        menu->SetSize(TOOL_WIDTH, TOOL_BAR_HEIGHT);
        menu->SetPos(dr4::Vec2f(TOOL_WIDTH, 0) * tools.size());

        tools.push_back(menu.get());
        AddWidget(std::move(menu));
    }

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
        for (auto &tool : tools) tool->DrawOn(GetTexture());
        
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

        GetTexture().Clear({61, 61, 61});
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
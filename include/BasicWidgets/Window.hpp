#pragma once

#include "Utilities/ROAGUIRender.hpp"
#include "Utilities/ROACommon.hpp"

#include "BasicWidgets/Containers.hpp"
#include "CompositeWidgets/DropDownMenu.hpp"

namespace roa
{
 
class Window : public Container  {
    bool implicitHovered = false;

    std::unique_ptr<dr4::Rectangle> toolsBG;
    std::vector<DropDownMenu *> tools;
public:
    float TOOL_BAR_HEIGHT = 20;
    static constexpr float TOOL_WIDTH = 60;

    Window(hui::UI *ui) : Container(ui), toolsBG(ui->GetWindow()->CreateRectangle()) {
        assert(ui);
        toolsBG->SetFillColor(FULL_TRANSPARENT);
    }

    Window(const Window &) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = default;
    Window& operator=(Window&&) = default;
    virtual ~Window() = default;   

    void AddTool(std::unique_ptr<DropDownMenu> menu) {
        menu->SetSize(TOOL_WIDTH, TOOL_BAR_HEIGHT);
        menu->SetPos(dr4::Vec2f(TOOL_WIDTH, 0) * tools.size());

        tools.push_back(menu.get());
        
        auto *menuRaw = menu.get();
        AddWidget(std::move(menu));
        BringToFront(menuRaw);
    }

    void SetToolsBG(const dr4::Color color) {
        toolsBG->SetFillColor(color);
        ForceRedraw();
    }

protected:
    hui::EventResult OnIdle(hui::IdleEvent &evt) override {
        PropagateToChildren(evt);
        bool newImplicitHovered = CheckImplicitHover();
        if (newImplicitHovered != implicitHovered) ForceRedraw();
        implicitHovered = newImplicitHovered;
        
        return hui::EventResult::UNHANDLED;
    }
    
    virtual void WindowDrawSelfAction() const {}

    void Redraw() const override {
        GetTexture().Clear(FULL_TRANSPARENT);
        toolsBG->SetSize({GetSize().x, TOOL_BAR_HEIGHT});
        toolsBG->DrawOn(GetTexture());

        WindowDrawSelfAction();
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

        GetTexture().Clear(FULL_TRANSPARENT);
        backSurface->DrawOn(GetTexture());
    }

private:
};



} // namespace roa
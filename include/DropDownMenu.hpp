#pragma once
#include "BasicWidgets/Buttons.hpp"
#include "BasicWidgets/Containers.hpp"

namespace roa
{

class DropDownMenu : public LinContainer<hui::Widget> {
    ObjectButton *topButton=nullptr;
    hui::Widget *dropDown=nullptr;

public:
    DropDownMenu(hui::UI *ui): LinContainer(ui), topButton(new ObjectButton(ui))
    {
        assert(ui);
    
        topButton->SetOnPressAction([this](){ SwitchDropDownState(); });

        UI *extUI = static_cast<UI *>(ui); assert(ui);
        topButton->LoadSVGDropDownIcon(extUI->GetIconsTexturePack().triaRightSvgPath, extUI->GetIconsTexturePack().triaDownSvgPath);
        AddWidget(topButton);
    }

    void SetLabelText(const std::string &label) { topButton->SetLabelText(label); }
    void SetDropDownWidget(hui::Widget *wgt) { dropDown = wgt; ForceRedraw(); }
    void SwitchDropDownState() {
        topButton->SwitchDropDownState();
    }

protected:
    hui::EventResult PropagateToChildren(hui::Event &event) override {
        if (event.Apply(*topButton) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        if (topButton->IsDropDownActive() && dropDown && event.Apply(*dropDown) == hui::EventResult::HANDLED) 
            return hui::EventResult::HANDLED; 

        return hui::EventResult::UNHANDLED;
    }

    void OnSizeChanged() override { layout(); }

    void Redraw() const override {
        topButton->DrawOn(GetTexture());
        if (topButton->IsDropDownActive()) {
            if (dropDown) dropDown->DrawOn(GetTexture());
        }
    }

private:
    void layout() {
        topButton->SetSize(GetSize());
        if (dropDown) dropDown->SetPos(dr4::Vec2f(0, topButton->GetSize().y));
        ForceRedraw();
    }
};


} // namespace roa
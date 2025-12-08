#pragma once
#include "BasicWidgets/Buttons.hpp"
#include "BasicWidgets/Containers.hpp"

namespace roa
{


class DropDownButton final : public Button {
    const dr4::Color buttonColor = dr4::Color(61, 61, 61);
    int borderRadius = 2;

    dr4::Text *label;

    dr4::Image *dropDownActiveIcon;
    dr4::Image *dropDownNonActiveIcon;

public:
    DropDownButton(hui::UI *ui): 
        Button(ui), 
        label(ui->GetWindow()->CreateText()),
        dropDownActiveIcon(ui->GetWindow()->CreateImage()),
        dropDownNonActiveIcon(ui->GetWindow()->CreateImage())
    {
        assert(ui);
        assert(label);    
        
        // FIXME: ADD LAYOUT
        label->SetPos({40, 3}); 
        label->SetColor(static_cast<UI *>(GetUI())->GetTexturePack().whiteTextColor);
        label->SetFontSize(static_cast<UI *>(GetUI())->GetTexturePack().fontSize);
        label->SetText("Figure.000");
        label->DrawOn(GetTexture());

        dropDownNonActiveIcon->SetPos({4, 2});
        dropDownNonActiveIcon->SetSize({16, 16});

        SetMode(Button::Mode::STICK_MODE);

        dropDownActiveIcon->SetPos(dropDownNonActiveIcon->GetPos());
        dropDownActiveIcon->SetSize(dropDownNonActiveIcon->GetSize());

        LoadSVGDropDownIcons();
    }

    ~DropDownButton() = default;

    void SetLabelText(const std::string &text) { label->SetText(text); }
    void SetLabelFontSize(const int fontSize) { label->SetFontSize(fontSize); }

    bool IsDropDownActive() const { return pressed; }

protected:
    void Redraw() const override final {
        GetTexture().Clear(buttonColor);

        if (pressed) dropDownActiveIcon->DrawOn(GetTexture());
        else dropDownNonActiveIcon->DrawOn(GetTexture());
        label->DrawOn(GetTexture());

        dr4::Image *backSurface = GetTexture().GetImage(); assert(backSurface);

        DrawBlenderRoundedRectangle(
            backSurface->GetWidth(),
            backSurface->GetHeight(),
            borderRadius,               
            0,                
            FULL_TRANSPARENT,  
            buttonColor,
            [&](int x, int y, dr4::Color c) { backSurface->SetPixel(x,y,c); }
        );

        backSurface->DrawOn(GetTexture());
    }

private:
    void LoadSVGDropDownIcons() {
        assert(dropDownNonActiveIcon);
        assert(dropDownActiveIcon);

        UI *ui = static_cast<UI *>(GetUI()); assert(ui);       

        ExtractSVG(ui->GetTexturePack().triaRightSvgPath, dropDownNonActiveIcon->GetSize(), [this](int x, int y, dr4::Color color){ dropDownNonActiveIcon->SetPixel(x, y, color); });
        ExtractSVG(ui->GetTexturePack().triaDownSvgPath, dropDownActiveIcon->GetSize(), [this](int x, int y, dr4::Color color){ dropDownActiveIcon->SetPixel(x, y, color); });
    }
};

class DropDownMenu : public LinContainer<hui::Widget> {
    DropDownButton *topButton=nullptr;
    hui::Widget *dropDown=nullptr;


public:
    DropDownMenu(hui::UI *ui): LinContainer(ui), topButton(new DropDownButton(ui))
    {
        assert(ui);
    
        AddWidget(topButton);
    }

    void SetLabelText(const std::string &label) { topButton->SetLabelText(label); }
    void SetDropDownWidget(hui::Widget *wgt) { dropDown = wgt; ForceRedraw(); }

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
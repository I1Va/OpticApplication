#pragma once
#include "BasicWidgets/Buttons.hpp"
#include "BasicWidgets/Containers.hpp"

namespace roa
{


class DropDownButton final : public Button {
    const dr4::Color buttonColor = dr4::Color(61, 61, 61);
    int borderRadius = 2;

    std::unique_ptr<dr4::Text > label;
    std::unique_ptr<dr4::Image> dropDownActiveIcon;
    std::unique_ptr<dr4::Image> dropDownNonActiveIcon;

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
        label->SetPos({20, 3}); 
        label->SetColor(static_cast<UI *>(GetUI())->GetTexturePack().whiteTextColor);
        label->SetFontSize(static_cast<UI *>(GetUI())->GetTexturePack().fontSize);
        label->SetText("Figure.000");
        label->DrawOn(GetTexture());
        dropDownNonActiveIcon->SetPos({4, 2});

        float dropDownNonActiveIconWHCoef = 700.0 / 1100;
        dropDownNonActiveIcon->SetSize({16 * dropDownNonActiveIconWHCoef, 16});

        SetMode(Button::Mode::STICK_MODE);

        dropDownActiveIcon->SetPos(4, 6);
        float dropDownActiveIconWHCoef = 1100.0 / 700.0;
        dropDownActiveIcon->SetSize({10 * dropDownActiveIconWHCoef, 10});

        LoadSVGDropDownIcons();
    }

    ~DropDownButton() = default;

    void SetLabel(const std::string &text) { label->SetText(text); }
    void SetLabelFontSize(const int fontSize) { label->SetFontSize(fontSize); }

    bool IsDropDownActive() const { return pressed; }

protected:
    void Redraw() const override final {
        GetTexture().Clear(FULL_TRANSPARENT);

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

        if (pressed) dropDownActiveIcon->DrawOn(GetTexture());
        else dropDownNonActiveIcon->DrawOn(GetTexture());
        label->DrawOn(GetTexture());        
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
protected:
    DropDownButton *topButton;
    hui::Widget *dropDown=nullptr;

    bool detailResize = false;
    dr4::Vec2f originSize = dr4::Vec2f(0, 0);

    std::function<void()> onSizeChangedAction = nullptr;

public:
    DropDownMenu(hui::UI *ui): LinContainer(ui), topButton(new DropDownButton(ui))
    {
        assert(ui);
        
        topButton->SetOnPressAction([this](){ layout(); });
        topButton->SetOnUnpressAction([this](){ layout(); });
       
        AddWidget(topButton);
    }

    void SetLabel(const std::string &label) { topButton->SetLabel(label); }
    void SetDropDownWidget(hui::Widget *wgt) { dropDown = wgt; ForceRedraw(); }
    void SetOnSizeChangedAction(std::function<void()> action) { onSizeChangedAction = action; }

protected:
    hui::EventResult PropagateToChildren(hui::Event &event) override {
        if (event.Apply(*topButton) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        if (topButton->IsDropDownActive() && dropDown && event.Apply(*dropDown) == hui::EventResult::HANDLED) 
            return hui::EventResult::HANDLED; 

        return hui::EventResult::UNHANDLED;
    }

    void OnSizeChanged() override { 
        if (!detailResize) {
            originSize = GetSize(); 
            layout();
        }
        if (onSizeChangedAction) onSizeChangedAction();
    }

    void Redraw() const override {
        GetTexture().Clear(FULL_TRANSPARENT);
        topButton->DrawOn(GetTexture());

        if (dropDown) dropDown->DrawOn(GetTexture());
    }

private:
    void layout() {
        detailResize = true;
        topButton->SetSize(originSize);
        dropDown->SetPos(dr4::Vec2f(0, topButton->GetSize().y));
        if (topButton->IsDropDownActive()) {
            dr4::Vec2f extendedSize = dr4::Vec2f(originSize.x, std::fmax(originSize.y, dropDown->GetPos().y + dropDown->GetSize().y));
            SetSize(extendedSize);
        } else {
            SetSize(originSize);
        }       
      
        ForceRedraw();
        detailResize = false;
    }
};



} // namespace roa
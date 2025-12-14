#pragma once
#include <memory>
#include <cassert>
#include <functional>
#include <cmath>
#include "BasicWidgets/Buttons.hpp"
#include "BasicWidgets/Containers.hpp"

namespace roa
{

class DropDownButton final : public Button {
    const dr4::Color buttonColor = dr4::Color(61, 61, 61);
    int borderRadius = 2;
    int borderThickness = 0;
    dr4::Color borderColor = FULL_TRANSPARENT;

    std::unique_ptr<dr4::Text> label;
    std::unique_ptr<dr4::Image> dropDownActiveIcon;
    std::unique_ptr<dr4::Image> dropDownNonActiveIcon;

public:
    DropDownButton(hui::UI *ui) :
        Button(ui),
        label(ui->GetWindow()->CreateText()),
        dropDownActiveIcon(ui->GetWindow()->CreateImage()),
        dropDownNonActiveIcon(ui->GetWindow()->CreateImage())
    {
        assert(ui);
        assert(label);
        assert(dropDownActiveIcon);
        assert(dropDownNonActiveIcon);

        // FIXME: ADD LAYOUT
        label->SetPos({20, 3});
        label->SetColor(static_cast<UI*>(GetUI())->GetTexturePack().whiteTextColor);
        label->SetFontSize(static_cast<UI*>(GetUI())->GetTexturePack().fontSize);
        label->SetText("Figure.000");
        label->DrawOn(GetTexture());

        dropDownNonActiveIcon->SetPos({4, 2});
        float dropDownNonActiveIconWHCoef = 700.0f / 1100.0f;
        dropDownNonActiveIcon->SetSize({16.0f * dropDownNonActiveIconWHCoef, 16.0f});

        SetMode(Button::Mode::STICK_MODE);

        dropDownActiveIcon->SetPos({4, 6});
        float dropDownActiveIconWHCoef = 1100.0f / 700.0f;
        dropDownActiveIcon->SetSize({10.0f * dropDownActiveIconWHCoef, 10.0f});

        LoadSVGDropDownIcons();
    }

    virtual ~DropDownButton() = default;

    void SetBorderThinkess(const int thikness) {
        borderThickness = thikness;
        ForceRedraw();
    }

    void SetBorderColor(const dr4::Color color) {
        borderColor = color;
        ForceRedraw();
    }

    void Hide() { 
        pressed = false;
        if (onUnpressAction) onUnpressAction();
        ForceRedraw();
    }

    void SetLabel(const std::string &text) { label->SetText(text); }
    void SetLabelFontSize(int fontSize) { label->SetFontSize(fontSize); }

    bool IsDropDownActive() const { return pressed; }

protected:
    void Redraw() const override final {
        GetTexture().Clear(FULL_TRANSPARENT);

        dr4::Image* backSurface = GetTexture().GetImage();
        assert(backSurface);

        DrawBlenderRoundedRectangle(
            backSurface->GetWidth(),
            backSurface->GetHeight(),
            borderRadius,
            borderThickness,
            borderColor,
            buttonColor,
            [&](int x, int y, dr4::Color c) { backSurface->SetPixel(x, y, c); }
        );

        backSurface->DrawOn(GetTexture());

        if (pressed) dropDownActiveIcon->DrawOn(GetTexture());
        else dropDownNonActiveIcon->DrawOn(GetTexture());

        label->DrawOn(GetTexture());
    }

private:
    void LoadSVGDropDownIcons() {
        UI* ui = static_cast<UI*>(GetUI());
        assert(ui);

        ExtractSVG(ui->GetTexturePack().triaRightSvgPath, dropDownNonActiveIcon->GetSize(),
            [this](int x, int y, dr4::Color color) { dropDownNonActiveIcon->SetPixel(x, y, color); });

        ExtractSVG(ui->GetTexturePack().triaDownSvgPath, dropDownActiveIcon->GetSize(),
            [this](int x, int y, dr4::Color color) { dropDownActiveIcon->SetPixel(x, y, color); });
    }
};

class DropDownMenu : public Container {
protected:
    DropDownButton* topButton = nullptr;
    hui::Widget*    dropDown = nullptr;

    bool detailResize = false;
    dr4::Vec2f originSize = {0, 0};

    std::function<void()> onSizeChangedAction = nullptr;

public:
    DropDownMenu(hui::UI* ui) : Container(ui) {
        assert(ui);

        auto topButtonUnique = std::make_unique<DropDownButton>(ui);
        topButtonUnique->SetOnPressAction([this]() { layout(); });
        topButtonUnique->SetOnUnpressAction([this]() { layout(); });
        topButton = topButtonUnique.get();
        AddWidget(std::move(topButtonUnique));
    }

    void SetBorderThinkess(const int thikness) {
        topButton->SetBorderThinkess(thikness);
    }

    void SetBorderColor(const dr4::Color color) {
        topButton->SetBorderColor(color);
    }

    void SetLabel(const std::string& label) { 
        topButton->SetLabel(label); 
        ForceRedraw();
    }
    void SetDropDownWidget(std::unique_ptr<hui::Widget> wgt) { 
        dropDown = wgt.get(); 
        AddWidget(std::move(wgt));
        ForceRedraw(); 
    }

    void SetOnSizeChangedAction(std::function<void()> action) { onSizeChangedAction = action; }

protected:
    hui::EventResult PropagateToChildren(hui::Event& event) override {
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

        if (dropDown && topButton->IsDropDownActive()) dropDown->DrawOn(GetTexture());
    }

    hui::EventResult OnIdle(hui::IdleEvent &evt) override {
        if (!CheckImplicitHover()) topButton->Hide();
        return Container::OnIdle(evt);
    }

private:
    void layout() {
        detailResize = true;
        topButton->SetSize(originSize);
        if (dropDown) dropDown->SetPos({0, topButton->GetSize().y});

        if (topButton->IsDropDownActive() && dropDown) {
            dr4::Vec2f extendedSize = {std::fmax(originSize.x, dropDown->GetPos().x + dropDown->GetSize().x), std::fmax(originSize.y, dropDown->GetPos().y + dropDown->GetSize().y)};
            SetSize(extendedSize);
        } else {
            SetSize(originSize);
        }

        ForceRedraw();
        detailResize = false;
    }
};

} // namespace roa

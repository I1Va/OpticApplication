#pragma once

#include "hui/widget.hpp"
#include "Utilities/ROACommon.hpp"
#include "ROAUI.hpp"
#include "Utilities/ROAGUIRender.hpp"
#include "SVGParser/SVGImageConverter.hpp"

namespace roa
{

class Button : public hui::Widget {
public:
    enum class Mode{
        HOVER_MODE,
        FOCUS_MODE,
        CAPTURE_MODE,
        STICK_MODE
    };

protected:
    enum class StateProperty : uint8_t {
        NON_ACTIVE  = 0b00000000,
        HOVERED     = 0b10000000,
        FOCUSED     = 0b01000000,
        CLICKED     = 0b00100000,
    };

// Logic
    Mode mode = Mode::CAPTURE_MODE;
    bool pressed = false;
    std::function<void()> onPressAction = nullptr;
    std::function<void()> onUnpressAction = nullptr;

// Drawing
    uint8_t state = static_cast<uint8_t>(StateProperty::NON_ACTIVE); 

public:
    using hui::Widget::Widget;
    virtual ~Button() = default;

    void SetOnPressAction(std::function<void()> action) { onPressAction = action; }
    void SetOnUnpressAction(std::function<void()> action) { onUnpressAction = action; }

    bool IsPressed() const { return pressed; }

    void SetMode(const Mode m) { mode = m; }

protected:
    hui::EventResult OnMouseDown(hui::MouseButtonEvent &event) override final { 
        if (!GetRect().Contains(event.pos)) return hui::EventResult::UNHANDLED;
        if (!(event.button == dr4::MouseButtonType::LEFT)) return hui::EventResult::UNHANDLED;
        
        GetUI()->ReportFocus(this);
        GetUI()->SetCaptured(this);

        addStateProperty(StateProperty::CLICKED);

        switch (mode) {
            case Mode::HOVER_MODE: break;
            case Mode::FOCUS_MODE: break;
            case Mode::CAPTURE_MODE:
                ForceRedraw(); 
                pressed = true; 
                if (onPressAction) onPressAction();
                break;
            case Mode::STICK_MODE: 
                pressed = !pressed; 
                ForceRedraw();
                if (pressed && onPressAction) onPressAction();
                if (!pressed && onUnpressAction) onUnpressAction();
                break;
            default: assert(0); break;
        }
        return hui::EventResult::HANDLED;
    }

    hui::EventResult OnMouseUp(hui::MouseButtonEvent &event) override final { 
        if (!GetRect().Contains(event.pos) && (GetUI()->GetCaptured() != this)) return hui::EventResult::UNHANDLED;
        if (!(event.button == dr4::MouseButtonType::LEFT)) return hui::EventResult::UNHANDLED;

        GetUI()->SetCaptured(nullptr);
        removeStateProperty(StateProperty::CLICKED);
    
        switch (mode) {
            case Mode::HOVER_MODE: break;
            case Mode::FOCUS_MODE: break;
            case Mode::CAPTURE_MODE: 
                ForceRedraw();
                pressed = false; 
                if (onUnpressAction) onUnpressAction();
                break;
            case Mode::STICK_MODE: 
                break;
            default: assert(0); break;
        }
    
        return hui::EventResult::HANDLED;
    }

    hui::EventResult OnIdle(hui::IdleEvent &evt) override final {
        OnIdleSelfAction(evt);
        
        switch (mode) {
            case Mode::HOVER_MODE:
                {
                    bool newPressed = (GetUI()->GetHovered() == this);
                    if (pressed != newPressed) {
                        if (newPressed && onPressAction) onPressAction();
                        if (!newPressed && onUnpressAction) onUnpressAction();
                        ForceRedraw();
                    } 
                    pressed = newPressed;
                    break;
                }
            case Mode::FOCUS_MODE: 
                {
                    bool newPressed = (GetUI()->GetFocused() == this);
                    if (pressed != newPressed) {
                        if (newPressed && onPressAction) onPressAction();
                        if (!newPressed && onUnpressAction) onUnpressAction();
                        ForceRedraw();
                    } 
                    pressed = newPressed;
                    break;
                }
            case Mode::CAPTURE_MODE: 
                break;
            case Mode::STICK_MODE: 
                break;
            default: assert(0); break;
        }
        
        return hui::EventResult::UNHANDLED;
    }
    virtual void OnIdleSelfAction(hui::IdleEvent &) {}

    bool checkStateProperty(const StateProperty property) const {
        return (state & static_cast<uint8_t>(property));
    }

    void OnHoverGained() override { addStateProperty   (StateProperty::HOVERED); }
    void OnHoverLost()   override { removeStateProperty(StateProperty::HOVERED); }

    void OnFocusGained() override { addStateProperty   (StateProperty::FOCUSED); }
    void OnFocusLost()   override { removeStateProperty(StateProperty::FOCUSED); }

private:
    void addStateProperty(const StateProperty property) {
        uint8_t newState = state | static_cast<uint8_t>(property);
        if (!(newState == state)) ForceRedraw();
        state = newState;
    }

    void removeStateProperty(const StateProperty property) {
        uint8_t newState = state & ~static_cast<uint8_t>(property);
            
        if (!(newState == state)) ForceRedraw();
        state = newState;
    }
};

class SimpButton : public Button {
protected:
    dr4::Color pressedColor     = GRAY;
    dr4::Color unpressedColor   = WHITE;
public:
    using Button::Button;
    virtual ~SimpButton() = default;

    void SetPressedColor(const dr4::Color color) { pressedColor = color; } 
    void SetUnpressedColor(const dr4::Color color) { unpressedColor = color; }

protected:
    void Redraw() const override {

        if (pressed) GetTexture().Clear(pressedColor);
        else GetTexture().Clear(unpressedColor);
    }
};

class TextureButton : public Button { 
protected:
    const dr4::Texture *pressedTexture = nullptr;
    const dr4::Texture *unpressedTexture = nullptr;
public:
    using Button::Button;
    virtual ~TextureButton() = default;

    void SetpressedTexture(const dr4::Texture *texture) { 
        assert(texture);
        pressedTexture = texture; 
    } 

    void SetUnpressedTexture(const dr4::Texture *texture) { 
        assert(texture);
        unpressedTexture = texture; 
    }
protected:
    void Redraw() const override {
        GetTexture().Clear(FULL_TRANSPARENT);
    
        if (pressed) {
            if (pressedTexture) pressedTexture->DrawOn(GetTexture());
            else GetTexture().Clear(GRAY);
            return;
        }

        if (unpressedTexture) unpressedTexture->DrawOn(GetTexture());
        else GetTexture().Clear(WHITE);
    }

};

class TextButton : public SimpButton {
protected:
    std::unique_ptr<dr4::Text> text;
public:
    TextButton(hui::UI *ui) : SimpButton(ui), text(GetUI()->GetWindow()->CreateText()) {
        assert(ui);

        text->SetFont(static_cast<UI *>(GetUI())->GetDefaultFont());
    }

    virtual ~TextButton() = default;

    void SetText(const std::string &content) {
        text->SetText(content);
    }

    void SetFont(dr4::Font *font) {
        assert(font);

        text->SetFont(font);
        ForceRedraw();
    }

protected:

    void Redraw() const override {
        GetTexture().Clear(FULL_TRANSPARENT);

        if (pressed) GetTexture().Clear(pressedColor);
        else GetTexture().Clear(unpressedColor);
        text->DrawOn(GetTexture());
    }   

    void OnSizeChanged() override { relayout(); }

private: 
    void relayout() { text->SetFontSize(GetSize().y); }    
};

class RoundedBlenderButton : public Button {
    dr4::Color nonActiveColor = dr4::Color(44, 44, 44);
    dr4::Color hoverColor     = dr4::Color(54, 54, 54);
    dr4::Color clickedColor   = dr4::Color(77, 77, 77);
    int borderRadius = 2;

public:
    using Button::Button;
    virtual ~RoundedBlenderButton() = default;
    void SetBorderRadius(const int radius) { borderRadius = radius; }
    int  GetBorderRadius() const { return borderRadius; }

protected:
    void Redraw() const override final {
        GetTexture().Clear(FULL_TRANSPARENT);

        dr4::Image *backSurface = GetTexture().GetImage();
        assert(backSurface);

        dr4::Color bgColor = nonActiveColor;
        if (checkStateProperty(Button::StateProperty::CLICKED)) {
            bgColor = clickedColor;
        } else if (checkStateProperty(Button::StateProperty::HOVERED)) {
            bgColor = hoverColor;
        }

        DrawBlenderRoundedRectangle(
            backSurface->GetWidth(),
            backSurface->GetHeight(),
            borderRadius,               
            0,                
            FULL_TRANSPARENT,  
            bgColor,
            [&](int x, int y, dr4::Color c) { backSurface->SetPixel(x,y,c); }
        );

        backSurface->DrawOn(GetTexture());
    }
};
        
struct ObjectButtonColorPack {
    dr4::Color focusedHovered;
    dr4::Color focused;
    dr4::Color hovered;
    dr4::Color nonActive;
};

static const inline ObjectButtonColorPack BLACK_OBJECT_PACK = 
{
    .focusedHovered = dr4::Color(78, 100, 145),
    .focused = dr4::Color(51, 77, 128),
    .hovered = dr4::Color(68, 68, 68),
    .nonActive = dr4::Color(40, 40, 40)
};

static const inline ObjectButtonColorPack GRAY_OBJECT_PACK = 
{
    .focusedHovered = dr4::Color(78, 100, 145),
    .focused = dr4::Color(51, 77, 128),
    .hovered = dr4::Color(71, 71, 71),
    .nonActive = dr4::Color(43, 43, 43)
};

class ObjectButton final : public Button {
    const dr4::Color labelNonFocusedColor = dr4::Color(174, 174, 174, 255); 
    const dr4::Color labelFocusedColor    = dr4::Color(232, 165, 55, 255); 

    ObjectButtonColorPack colorPack = BLACK_OBJECT_PACK;
    // icon texture
    // label
    dr4::Text *label;

    bool dropDownActive = false;
    dr4::Image *dropDownActiveIcon;
    dr4::Image *dropDownNonActiveIcon;
    dr4::Image *mainIcon;

public:
    ObjectButton(hui::UI *ui): 
        Button(ui), 
        label(ui->GetWindow()->CreateText()),
        dropDownActiveIcon(ui->GetWindow()->CreateImage()),
        dropDownNonActiveIcon(ui->GetWindow()->CreateImage()),
        mainIcon(ui->GetWindow()->CreateImage()) 
    {
        assert(ui);
        assert(label);    
        
        // FIXME: ADD LAYOUT
        label->SetPos({40, 3}); 
        label->SetFontSize(11);
        label->SetText("Figure.000");
        label->DrawOn(GetTexture());


        dropDownNonActiveIcon->SetPos({4, 2});
        dropDownNonActiveIcon->SetSize({16, 16});

        dropDownActiveIcon->SetPos(dropDownNonActiveIcon->GetPos());
        dropDownActiveIcon->SetSize(dropDownNonActiveIcon->GetSize());

        mainIcon->SetPos({20, 2});
        mainIcon->SetSize({16, 16});
    }

    ~ObjectButton() = default;
    
    void SetColorPack(const ObjectButtonColorPack pack) {
        colorPack = pack;
    }

    void SetLabelText(const std::string &text) { label->SetText(text); }
    void SetLabelFontSize(const int fontSize) { label->SetFontSize(fontSize); }
    void LoadSVGMainIcon(const std::string &path) {
        assert(mainIcon);
        ExtractSVG(path, mainIcon->GetSize(), [this](int x, int y, dr4::Color color){ mainIcon->SetPixel(x, y, color); });
    }
    void LoadSVGDropDownIcon(const std::string &nonActiveIconPath, const std::string &activeIconPath) {
        assert(dropDownNonActiveIcon);
        assert(dropDownActiveIcon);
        
        ExtractSVG(nonActiveIconPath, dropDownNonActiveIcon->GetSize(), [this](int x, int y, dr4::Color color){ dropDownNonActiveIcon->SetPixel(x, y, color); });
        ExtractSVG(activeIconPath, dropDownActiveIcon->GetSize(), [this](int x, int y, dr4::Color color){ dropDownActiveIcon->SetPixel(x, y, color); });
    }

    void SwitchDropDownState() { dropDownActive = !dropDownActive; ForceRedraw(); }

protected:
    void Redraw() const override final {
        GetTexture().Clear(FULL_TRANSPARENT);

        label->SetColor(labelNonFocusedColor);
        if (checkStateProperty(Button::StateProperty::FOCUSED) && checkStateProperty(Button::StateProperty::HOVERED)) {
            GetTexture().Clear(colorPack.focusedHovered);
            label->SetColor(labelFocusedColor);
        } else if (checkStateProperty(Button::StateProperty::FOCUSED)) {
            label->SetColor(labelFocusedColor);
            GetTexture().Clear(colorPack.focused);
        } else if (checkStateProperty(Button::StateProperty::HOVERED)) {
            GetTexture().Clear(colorPack.hovered);
        } else {
            GetTexture().Clear(colorPack.nonActive);
        }
        
        mainIcon->DrawOn(GetTexture());
        if (dropDownActive) dropDownActiveIcon->DrawOn(GetTexture());
        else dropDownNonActiveIcon->DrawOn(GetTexture());
        label->DrawOn(GetTexture());
    }
};

}
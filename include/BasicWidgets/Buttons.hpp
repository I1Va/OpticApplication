#pragma once
#include <string>

#include "hui/widget.hpp"
#include "Utilities/ROACommon.hpp"
#include "ROAUI.hpp"
#include "Utilities/ROAGUIRender.hpp"
#include "Utilities/SVGImageConverter.hpp"

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
    using hui::Widget::Widget;

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
    virtual ~Button() = default;
    Button(const Button&) = delete;
    Button& operator=(const Button&) = delete;
    Button(Button&&) = default;
    Button& operator=(Button&&) = default;

    void SetOnPressAction(std::function<void()> action) { onPressAction = action; }
    void SetOnUnpressAction(std::function<void()> action) { onUnpressAction = action; }

    bool IsPressed() const { return pressed; }

    void SetMode(const Mode m) { mode = m; }

protected:
    hui::EventResult OnMouseDown(hui::MouseButtonEvent &event) override { 
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

    hui::EventResult OnMouseUp(hui::MouseButtonEvent &event) override { 
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

    hui::EventResult OnIdle(hui::IdleEvent &evt) override {
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



class TextButton : public Button {
    dr4::Color nonActiveColor = dr4::Color(44, 44, 44);
    dr4::Color hoverColor     = dr4::Color(54, 54, 54);
    dr4::Color clickedColor   = dr4::Color(77, 77, 77);
    int borderRadius = 2;
    std::unique_ptr<dr4::Text> label;

public:
    using Button::Button;
    virtual ~TextButton() = default;
    void SetBorderRadius(const int radius) { borderRadius = radius; }
    int  GetBorderRadius() const { return borderRadius; }
    // void SetLabel(const std::string &content) {
    //     label->SetText()
    // }

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


}
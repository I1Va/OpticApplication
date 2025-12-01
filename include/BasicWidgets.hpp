#pragma once

#include "hui/widget.hpp"
#include "ROACommon.hpp"

namespace roa
{


class Button : public hui::Widget {
    enum class Mode {
        HOVER,
        FOCUSED,
        CAPTURED,
    };

protected:
    std::function<void()> onClickAction = nullptr;

    bool pressed        = false;
    bool actived        = false;
    Mode mode           = Mode::FOCUSED;

public:
    using hui::Widget::Widget;
    ~Button() = default;

    void SetOnClickAction(std::function<void()> action) { onClickAction = action; }
    bool IsPressed() const { return pressed; }

    void SetHoverMode()         { mode = Mode::HOVER; }
    void SetFocusedMode()       { mode = Mode::FOCUSED; }
    void SetCapturedMode()      { mode = Mode::CAPTURED; }

protected:
    hui::EventResult OnMouseDown(hui::MouseButtonEvent &event) override { 
        if (!GetRect().Contains(event.pos)) return hui::EventResult::UNHANDLED;
        if (!(event.button == dr4::MouseButtonType::LEFT)) return hui::EventResult::UNHANDLED;
        if (!event.pressed) return hui::EventResult::UNHANDLED; 

        pressed = true;
        GetUI()->ReportFocus(this);
        if (onClickAction) onClickAction();
        return hui::EventResult::HANDLED;
    }

    hui::EventResult OnMouseUp (hui::MouseButtonEvent &) override { 
        pressed = false;
        return hui::EventResult::HANDLED;
    }

    hui::EventResult OnIdle(hui::IdleEvent &) override {
        bool newActiveState = actived;
        
        switch (mode) {
        case Mode::HOVER:
            newActiveState = (GetUI()->GetFocused() == this);
            break;
        case Mode::FOCUSED:
            newActiveState = (GetUI()->GetHovered() == this);
            break;
        case Mode::CAPTURED:
            newActiveState = pressed;
            break;
        default:
            break;
        }

        // std::cout << "pressed : " << pressed << "\n";
        if (actived != newActiveState) ForceRedraw();
        actived = newActiveState;
        // pressed = false;

        return hui::EventResult::UNHANDLED;
    }
};

class SimpButton : public Button {
    dr4::Color pressedColor     = dr4::Color(0, 0, 0, 255);
    dr4::Color unpressedColor   = dr4::Color(255, 255, 255, 255);;
public:
    using Button::Button;
    ~SimpButton() = default;

    void SetPressedColor(const dr4::Color color) { pressedColor = color; } 
    void SetUnpressedColor(const dr4::Color color) { pressedColor = color; }

protected:
    void Redraw() const override {
        if (pressed) GetTexture().Clear(pressedColor);
        else GetTexture().Clear(unpressedColor);
    }
};

class TextureButton : public Button { 
    const dr4::Texture *pressedTexture = nullptr;
    const dr4::Texture *unpressedTexture = nullptr;
public:
    using Button::Button;
    ~TextureButton() = default;

    void SetPressedTexture(const dr4::Texture *texture) { 
        assert(texture);
        pressedTexture = texture; 
    } 

    void SetUnpressedTexture(const dr4::Texture *texture) { 
        assert(texture);
        unpressedTexture = texture; 
    }
protected:
    void Redraw() const override {
        if (pressed) {
            if (pressedTexture) pressedTexture->DrawOn(GetTexture());
            else GetTexture().Clear(BLACK);
            return;
        }

        if (unpressedTexture) unpressedTexture->DrawOn(GetTexture());
        else GetTexture().Clear(WHITE);
    }

};

}
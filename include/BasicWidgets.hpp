#pragma once

#include "hui/widget.hpp"
#include "ROACommon.hpp"

namespace roa
{

class Button : public hui::Widget {
public:
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

        pressed = true;
        GetUI()->ReportFocus(this);
        GetUI()->SetCaptured(this);
        if (onClickAction) onClickAction();
        return hui::EventResult::HANDLED;
    }

    hui::EventResult OnMouseUp (hui::MouseButtonEvent &event) override { 
        if (!GetRect().Contains(event.pos) && (GetUI()->GetCaptured() != this)) return hui::EventResult::UNHANDLED;
        if (!(event.button == dr4::MouseButtonType::LEFT)) return hui::EventResult::UNHANDLED;

        GetUI()->SetCaptured(nullptr);
        pressed = false;
        return hui::EventResult::HANDLED;
    }

    hui::EventResult OnIdle(hui::IdleEvent &) override {
        bool newActiveState = actived;
        
        switch (mode) {
        case Mode::HOVER:
            newActiveState = (GetUI()->GetHovered() == this);
            break;
        case Mode::FOCUSED:
            newActiveState = (GetUI()->GetFocused() == this);
            break;
        case Mode::CAPTURED:
            newActiveState = pressed;
            break;
        default:
            break;
        }

        if (actived != newActiveState) ForceRedraw();
        actived = newActiveState;

        return hui::EventResult::UNHANDLED;
    }
};

class SimpButton : public Button {
    dr4::Color pressedColor     = BLACK;
    dr4::Color unpressedColor   = WHITE;
public:
    using Button::Button;
    ~SimpButton() = default;

    void SetPressedColor(const dr4::Color color) { pressedColor = color; } 
    void SetUnpressedColor(const dr4::Color color) { unpressedColor = color; }

protected:
    void Redraw() const override {
        if (actived) GetTexture().Clear(pressedColor);
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
        if (actived) {
            if (pressedTexture) pressedTexture->DrawOn(GetTexture());
            else GetTexture().Clear(BLACK);
            return;
        }

        if (unpressedTexture) unpressedTexture->DrawOn(GetTexture());
        else GetTexture().Clear(WHITE);
    }

};

class ThumbButton : public SimpButton  {
    dr4::Rect2f movingArea = {};

    dr4::Vec2f accumulatedRel = {};
    bool replaced = false;

    std::function<void()> onReplaceAction = nullptr;

public:
    ThumbButton(hui::UI *ui): SimpButton(ui) {
        assert(ui);
    }

    ~ThumbButton() = default;

    void SetMovingArea(const dr4::Rect2f rect) { 
        movingArea = rect;
    }

    void SetOnReplaceAction(std::function<void()> action) {
        onReplaceAction = action;
    }

protected:
    hui::EventResult OnIdle(hui::IdleEvent &) override {
        if (replaced) {
            SetPos(GetPos() + accumulatedRel);
            clampPos();
            accumulatedRel = {0, 0};
            replaced = false;
            if (onReplaceAction) onReplaceAction();
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }

        bool newActiveState = actived;
        
        switch (mode) {
        case Mode::HOVER:
            newActiveState = (GetUI()->GetHovered() == this);
            break;
        case Mode::FOCUSED:
            newActiveState = (GetUI()->GetFocused() == this);
            break;
        case Mode::CAPTURED:
            newActiveState = pressed;
            break;
        default:
            break;
        }

        if (actived != newActiveState) ForceRedraw();
        actived = newActiveState;

        return hui::EventResult::UNHANDLED;

    }

    hui::EventResult OnMouseMove(hui::MouseMoveEvent &event) override {
        if (!GetRect().Contains(event.pos) && (GetUI()->GetCaptured() != this)) return hui::EventResult::UNHANDLED;

        if (GetUI()->GetFocused() == this && actived) {
            accumulatedRel += event.rel;
            replaced = true;
            return hui::EventResult::HANDLED;
        }

        return hui::EventResult::UNHANDLED;
    }

    void OnPosChanged() {
        replaced = true;
    }

private:
    void clampPos() {
        dr4::Rect2f realMovingArea = movingArea;
        realMovingArea.size.x = std::fmax(0, realMovingArea.size.x - GetSize().x);
        realMovingArea.size.y = std::fmax(0, realMovingArea.size.y - GetSize().y);
        
        dr4::Vec2f pos = getClampedDotInRect(GetPos(), realMovingArea);
        SetPos(pos);
    }
};


}
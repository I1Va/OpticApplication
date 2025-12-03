#pragma once

#include "hui/widget.hpp"
#include "ROACommon.hpp"
#include "ROAUI.hpp"

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
    std::function<void()> onUnpressAction = nullptr;

    bool pressed        = false;
    bool actived        = false;
    Mode mode           = Mode::CAPTURED;

public:
    using hui::Widget::Widget;
    ~Button() = default;

    void SetOnClickAction(std::function<void()> action) { onClickAction = action; }
    void SetOnUnpressAction(std::function<void()> action) { onUnpressAction = action; }
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
        if (!actived && onUnpressAction) onUnpressAction();
        

        return hui::EventResult::UNHANDLED;
    }
};

class SimpButton : public Button {
protected:
    dr4::Color pressedColor     = GRAY;
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
protected:
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

    ~TextButton() = default;

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
        if (actived) GetTexture().Clear(pressedColor);
        else GetTexture().Clear(unpressedColor);
        text->DrawOn(GetTexture());
    }   

    void OnSizeChanged() override { relayout(); }

private: 
    void relayout() { text->SetFontSize(GetSize().y); }    
};

}
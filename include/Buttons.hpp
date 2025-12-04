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
        STICKING
    };

protected:
    std::function<void()> onPressAction = nullptr;
    std::function<void()> onUnpressAction = nullptr;

    bool pressed = false;
    Mode mode = Mode::CAPTURED;

public:
    using hui::Widget::Widget;
    virtual ~Button() = default;

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

        switch (mode) {
            case Mode::HOVER: break;
            case Mode::FOCUSED: break;
            case Mode::CAPTURED:
                ForceRedraw(); 
                pressed = true; 
                if (onPressAction) onPressAction();
                break;
            case Mode::STICKING: 
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
    
        switch (mode) {
            case Mode::HOVER: break;
            case Mode::FOCUSED: break;
            case Mode::CAPTURED: 
                ForceRedraw();
                pressed = false; 
                if (onUnpressAction) onUnpressAction();
                break;
            case Mode::STICKING: 
                break;
            default: assert(0); break;
        }
    
        return hui::EventResult::HANDLED;
    }

    hui::EventResult OnIdle(hui::IdleEvent &) override {
        switch (mode) {
            case Mode::HOVER:
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
            case Mode::FOCUSED: 
                {
                    bool newPressed = (GetUI()->GetCaptured() == this);
                    if (pressed != newPressed) {
                        if (newPressed && onPressAction) onPressAction();
                        if (!newPressed && onUnpressAction) onUnpressAction();
                        ForceRedraw();
                    } 
                    pressed = newPressed;
                    break;
                }
            case Mode::CAPTURED: 
                break;
            case Mode::STICKING: 
                break;
            default: assert(0); break;
        }
        
        return hui::EventResult::UNHANDLED;
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

}
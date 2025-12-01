#pragma once
#include <functional>

#include "hui/widget.hpp"

#include "ROAUI.hpp"
#include "ROACommon.hpp"

namespace roa
{

class TextWidget : public hui::Widget {
protected:
    std::unique_ptr<dr4::Text> text;

    std::unique_ptr<dr4::Line> caret;
    bool drawCaret = false;
    
    dr4::Color backColor = WHITE;

public:
    TextWidget(hui::UI *ui): hui::Widget(ui), text(GetUI()->GetWindow()->CreateText()), caret(GetUI()->GetWindow()->CreateLine()) { 
        assert(ui); 
        text->SetFont(static_cast<UI *>(GetUI())->GetDefaultFont());
        caret->SetColor(BLACK);
        caret->SetThickness(2);
    }

    ~TextWidget() = default;

    void SetFont(dr4::Font *font) {
        assert(font);

        text->SetFont(font);
        ForceRedraw();
    }

    void SetText(const std::string &content) {
        text->SetText(content);
        relayoutCaret();
        ForceRedraw();
    }

    void ShowCaret() { 
        drawCaret = true; 
        ForceRedraw();
    }

    void HideCaret() { 
        drawCaret = false; 
        ForceRedraw();
    }
    
    void SwitchCaret() {
        if (drawCaret) HideCaret();
        else ShowCaret();
    }

protected:
    void Redraw() const override {
        GetTexture().Clear(backColor);
        text->DrawOn(GetTexture());
        if (drawCaret) {
            caret->DrawOn(GetTexture());
        }
    }
    
    void OnSizeChanged() override { relayoutCaret(); }

    void relayoutCaret() {
        dr4::Vec2f start = text->GetPos() + dr4::Vec2f(text->GetBounds().x, 0);
        dr4::Vec2f end   = text->GetPos() + text->GetBounds();

        caret->SetStart(start);
        caret->SetEnd(end);
    }

};

class TextInputWidget : public TextWidget {
    static constexpr double CARET_BLINK_DELTA_SECS = 0.5; 
    
    double curCaretBlinkDeltaSecs = CARET_BLINK_DELTA_SECS; 
    bool caretBlinkState = false;

    std::function<void(const std::string&)> onEnterAction = nullptr;
    
    bool needOnEnterCall_ = false;
    std::string textBufer;

    

public:
    TextInputWidget(hui::UI *ui) : TextWidget(ui) {
        assert(ui);
        SetText(" ");
    }

    ~TextInputWidget() = default;

    void setOnEnterAction(std::function<void(const std::string&)> action) {
        onEnterAction = action;
    }

protected:
    void setOnEnter( std::function<void(const std::string &)> onEnter) { onEnterAction = onEnter; }

    hui::EventResult OnIdle(hui::IdleEvent &event) override {
        caretBlinkState = (GetUI()->GetFocused() == this);
    
        curCaretBlinkDeltaSecs -= event.deltaTime;
        if (curCaretBlinkDeltaSecs <= 0) {
            curCaretBlinkDeltaSecs = CARET_BLINK_DELTA_SECS;
            if (!caretBlinkState) HideCaret();
            else SwitchCaret();
        }

        if (needOnEnterCall_) {
            if (onEnterAction) onEnterAction(text->GetText());
            needOnEnterCall_ = false;
            return hui::EventResult::HANDLED;
        }

        return hui::EventResult::UNHANDLED;
    }

    hui::EventResult OnText(hui::TextEvent &event) { 
        textBufer += std::string(event.text);
        SetText(textBufer);
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }

    hui::EventResult OnKeyDown(hui::KeyEvent &event) override {
        if (event.key == dr4::KeyCode::KEYCODE_BACKSPACE && !textBufer.empty()) {
            textBufer.pop_back();
            SetText(textBufer);
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }

        if (event.key == dr4::KeyCode::KEYCODE_ENTER) {
            needOnEnterCall_ = true;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }

        return hui::EventResult::UNHANDLED;
    }

};


}

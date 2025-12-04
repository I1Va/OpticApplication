#pragma once
#include <functional>
#include <cstring>

#include "hui/widget.hpp"

#include "ROAUI.hpp"
#include "ROACommon.hpp"

namespace roa
{

class TextWidget : public hui::Widget {
protected:
    std::unique_ptr<dr4::Text> text;

    std::unique_ptr<dr4::Line> caret;
    int caretPos = 0;
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

    void SetCaretPos(const int pos) { 
        caretPos = pos; 
        relayoutCaret();
    }

    void ShowCaret() { 
        drawCaret = true; 
        ForceRedraw();
    }
    void HideCaret() { 
        drawCaret = false; 
        ForceRedraw();
    }  
    void BlinkCaret() {
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
    
    void OnSizeChanged() override { 
        relayoutCaret(); 
        relayoutText();
    }

    void relayoutText() { text->SetFontSize(GetSize().y); }

    void relayoutCaret() {
        caretPos = std::clamp(caretPos, 0, static_cast<int>(text->GetText().size()));
        std::string prevText = text->GetText();
        std::string caretSubstr = prevText.substr(0, caretPos);

        text->SetText(caretSubstr);
        dr4::Vec2f caretTextBounds = text->GetBounds();
        text->SetText(prevText);

        dr4::Vec2f start = text->GetPos() + dr4::Vec2f(caretTextBounds.x, 0);
        dr4::Vec2f end   = text->GetPos() + caretTextBounds;

        caret->SetStart(start);
        caret->SetEnd(end);
    }
};

class TextInputWidget : public TextWidget {
    static constexpr double CARET_BLINK_DELTA_SECS = 0.5; 
    
    double curCaretBlinkDeltaSecs = CARET_BLINK_DELTA_SECS; 
    bool caretBlinkState = false;
    int caretPos = 0;

    std::function<void(const std::string&)> onEnterAction = nullptr;
    
    bool needOnEnterCall_ = false;
    std::string textBufer;

    

public:
    TextInputWidget(hui::UI *ui) : TextWidget(ui) {
        assert(ui);
        SetText(" ");
    }

    ~TextInputWidget() = default;

    void SetOnEnterAction(std::function<void(const std::string&)> action) {
        onEnterAction = action;
    }

    void SetText(const std::string &content) {
        textBufer = content;
        TextWidget::SetText(content);
    }

protected:
    hui::EventResult OnIdle(hui::IdleEvent &event) override {
        caretBlinkState = (GetUI()->GetFocused() == this);
        SetCaretPos(caretPos);
        curCaretBlinkDeltaSecs -= event.deltaTime;
        if (curCaretBlinkDeltaSecs <= 0) {
            curCaretBlinkDeltaSecs = CARET_BLINK_DELTA_SECS;
            if (!caretBlinkState) HideCaret();
            else BlinkCaret();
        }

        if (needOnEnterCall_) {
            if (onEnterAction) onEnterAction(text->GetText());
            needOnEnterCall_ = false;
            return hui::EventResult::HANDLED;
        }

        return hui::EventResult::UNHANDLED;
    }

    hui::EventResult OnText(hui::TextEvent &event) { 
        if (GetUI()->GetFocused() != this) return hui::EventResult::UNHANDLED;
    
        int prevSize = static_cast<int>(textBufer.size());
        textBufer.insert(caretPos, event.text);
        caretPos += static_cast<int>(textBufer.size()) -  prevSize;
        SetText(textBufer);
        ForceRedraw();
        return hui::EventResult::HANDLED;
    }

    hui::EventResult OnKeyDown(hui::KeyEvent &event) override {
        if (GetUI()->GetFocused() != this) return hui::EventResult::UNHANDLED;
    
        if (event.key == dr4::KeyCode::KEYCODE_BACKSPACE && !textBufer.empty()) {
            if (caretPos > 0) {
                textBufer.erase(caretPos - 1, 1);
                caretPos--;
                SetText(textBufer);
                ForceRedraw();
            }
            return hui::EventResult::HANDLED;
        }

        if (event.key == dr4::KeyCode::KEYCODE_ENTER) {
            needOnEnterCall_ = true;
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }

        if (event.key == dr4::KeyCode::KEYCODE_LEFT) {
            caretPos = std::max(0, caretPos - 1);
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }
        if (event.key == dr4::KeyCode::KEYCODE_RIGHT) {
            caretPos = std::min(static_cast<int>(textBufer.size()), caretPos + 1);
            ForceRedraw();
            return hui::EventResult::HANDLED;
        }

        return hui::EventResult::UNHANDLED;
    }
};

class TextInputField : public ZContainer<hui::Widget> {
    std::unique_ptr<TextWidget> label;
    std::unique_ptr<TextInputWidget> inputField;

public:
    TextInputField(hui::UI *ui) : ZContainer(ui), label(new TextWidget(ui)), inputField(new TextInputWidget(ui)) { 
        assert(ui); 
        BecomeParentOf(label.get());
        BecomeParentOf(inputField.get());
    }
    ~TextInputField() = default;

    hui::EventResult PropagateToChildren(hui::Event &event) override {
        if (event.Apply(*inputField) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;

        return hui::EventResult::UNHANDLED;
    }

    void SetLabel(const std::string &content) {
        label->SetText(content);
    }
    void SetText(const std::string &content) {
        inputField->SetText(content);
    }

    void SetOnEnterAction(std::function<void(const std::string &)> action) { inputField->SetOnEnterAction(action); }

    void BringToFront(hui::Widget *) override {}

protected:

    void OnSizeChanged() override {
        relayout();
    }

    void relayout() {
        float labelWIdth = GetSize().x / 2;
        float labelHeight = GetSize().y;
    
        float inputFieldWidth = labelWIdth;
        float inputFieldWidthHeight = GetSize().y;

        label->SetSize({labelWIdth, labelHeight});
        inputField->SetSize({inputFieldWidth, inputFieldWidthHeight});    
        inputField->SetPos({labelWIdth, 0});

        inputField->ForceRedraw();
        label->ForceRedraw();
    }
    
    void Redraw() const override {
        GetTexture().Clear(FULL_TRANSPARENT);
        
        label->DrawOn(GetTexture());
        inputField->DrawOn(GetTexture());
    }
};

}

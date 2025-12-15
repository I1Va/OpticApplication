#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include "BasicWidgets/Containers.hpp"
#include "BasicWidgets/Buttons.hpp"
#include "BasicWidgets/TextWidgets.hpp"

namespace roa {

class TextWindow final : public Window {
    RoundedBlenderButton *deleteBtn = nullptr;
    TextInputWidget *inputField = nullptr;
    TextWidget *messageField = nullptr;

    bool requestBringToFront = true;

public:
    TextWindow(hui::UI *ui): Window(ui) {
        assert(ui);
        InitLayout();
    }

    ~TextWindow() = default;

    TextWindow(const TextWindow&) = delete;
    TextWindow& operator=(const TextWindow&) = delete;
    TextWindow(TextWindow&&) = default;
    TextWindow& operator=(TextWindow&&) = default;

    void DisplayMessage(const std::string &message, const dr4::Color color=WHITE) {
        messageField->SetText(message);
        messageField->SetColor(color);
        ForceRedraw();
    }
    
    void SetInputFieldOnEnterAction(std::function<void(const std::string &text)> action) {
        inputField->SetOnEnterAction(action);
    }

    void InitLayout() {
        SetSize({200, 120});

        auto delBtn = std::make_unique<RoundedBlenderButton>(GetUI());
        deleteBtn = delBtn.get();
        deleteBtn->SetSize({24, 18});
        deleteBtn->SetPos({GetSize().x - deleteBtn->GetSize().x - 6, 6});
        deleteBtn->SetOnPressAction([this] {
            if (auto parent = GetParent()) {
                if (auto container = dynamic_cast<roa::Container*>(parent)) {
                    container->EraseWidget(this);
                }
            }
        });
        AddWidget(std::move(delBtn));

        auto in = std::make_unique<TextInputWidget>(GetUI());
        inputField = in.get();
        inputField->SetSize({GetSize().x - 12, 24});
        inputField->SetPos({6, 36});
        inputField->SetText("");
        inputField->SetBGColor({61, 61, 61, 255});

        AddWidget(std::move(in));

        auto msg = std::make_unique<TextWidget>(GetUI());
        messageField = msg.get();
        messageField->SetSize({GetSize().x - 12, 24});
        messageField->SetPos({6, 70});
        messageField->SetText("");
        messageField->SetBGColor({61, 61, 61, 255});
        messageField->SetFont(GetUI()->GetWindow()->GetDefaultFont());
        messageField->SetFontSize(static_cast<UI*>(GetUI())->GetTexturePack().fontSize);
        AddWidget(std::move(msg));

        ForceRedraw();
    }

protected:
    hui::EventResult OnIdle(hui::IdleEvent &evt) override {
        if (requestBringToFront) {
            requestBringToFront = false;
            if (auto parent = GetParent()) {
                if (auto container = dynamic_cast<roa::Container*>(parent)) {
                    container->BringToFront(this);
                }
            }
        }

        return Window::OnIdle(evt);
    }

    void OnSizeChanged() override {
        if (deleteBtn) deleteBtn->SetPos({GetSize().x - deleteBtn->GetSize().x - 6, 6});
        if (inputField) {
            inputField->SetSize({GetSize().x - 12, 24});
        }
        if (messageField) {
            messageField->SetSize({GetSize().x - 12, 24});
        }
        Window::OnSizeChanged();
    }

    void WindowDrawSelfAction() const override {
        GetTexture().Clear({61, 61, 61});
    }
};

} // namespace roa

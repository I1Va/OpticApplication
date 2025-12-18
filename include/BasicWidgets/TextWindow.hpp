#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include "BasicWidgets/Containers.hpp"
#include "BasicWidgets/Buttons.hpp"
#include "BasicWidgets/Window.hpp"
#include "BasicWidgets/TextWidgets.hpp"

namespace roa {

class TextWindow final : public Window {
    static constexpr float PADDING = 3; 

    RoundedBlenderButton *closeButton = nullptr;
    TextInputWidget *inputField = nullptr;
    TextWidget *messageField = nullptr;
    TextWidget *titleWidget = nullptr;

    TextButton *okButton = nullptr;
    TextButton *cancelButton = nullptr;

    bool requestBringToFront = true;
    std::string pendingTitle;

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

    void SetTitle(const std::string &t) {
        pendingTitle = t;
        if (titleWidget) titleWidget->SetText(t);
        ForceRedraw();
    }

    void SetOkButtonLabel(const std::string &label) {
        assert(okButton);
        okButton->SetLabel(label);
        ForceRedraw();
    }
    void SetCancelButtonLabel(const std::string &label) {
        assert(cancelButton);
        cancelButton->SetLabel(label);
        ForceRedraw();
    }

    void DisplayMessage(const std::string &message, const dr4::Color color=WHITE) {
        messageField->SetText(message);
        messageField->SetColor(color);
        ForceRedraw();
    }
    
    void SetInputFieldOnEnterAction(std::function<void(const std::string &text)> action) {
        inputField->SetOnEnterAction(action);
        okButton->SetOnPressAction([this, action](){ action(inputField->GetText()); });
    }

    void InitLayout() {
        SetSize({200, 150});

        auto title = std::make_unique<TextWidget>(GetUI());
        titleWidget = title.get();
        titleWidget->SetPos({6, 6});
        titleWidget->SetSize({GetSize().x - 12, 20});
        titleWidget->SetBGColor({45, 45, 45, 255});
        titleWidget->SetColor(static_cast<UI*>(GetUI())->GetTexturePack().whiteTextColor);
        titleWidget->SetFont(GetUI()->GetWindow()->GetDefaultFont());
        titleWidget->SetFontSize(static_cast<UI*>(GetUI())->GetTexturePack().fontSize);
        if (!pendingTitle.empty()) titleWidget->SetText(pendingTitle);
        AddWidget(std::move(title));

        auto closeButtonUnique = std::make_unique<RoundedBlenderButton>(GetUI());
        closeButton = closeButtonUnique.get();
        closeButton->SetHoverColor({251, 149, 159, 255});
        closeButton->SetClickedColor({109, 32, 41, 255});
        closeButton->SetSize({24, 18});
        closeButton->SetPos({GetSize().x - closeButton->GetSize().x - 6, 6});
        closeButton->SetOnPressAction([this] {
            if (auto parent = GetParent()) {
                if (auto container = dynamic_cast<roa::Container*>(parent)) {
                    container->EraseWidget(this);
                }
            }
        });
        AddWidget(std::move(closeButtonUnique));

        auto in = std::make_unique<TextInputWidget>(GetUI());
        inputField = in.get();
        inputField->SetSize({GetSize().x - 12, 24});
        inputField->SetPos({6, 36});
        inputField->SetText("");
        inputField->SetBGColor({45, 45, 45, 255});

        AddWidget(std::move(in));

        // Message field
        auto msg = std::make_unique<TextWidget>(GetUI());
        messageField = msg.get();
        messageField->SetSize({GetSize().x - 12, 24});
        messageField->SetPos({6, 70});
        messageField->SetText("");
        messageField->SetBGColor({45, 45, 45, 255});
        messageField->SetFont(GetUI()->GetWindow()->GetDefaultFont());
        messageField->SetFontSize(static_cast<UI*>(GetUI())->GetTexturePack().fontSize);
        AddWidget(std::move(msg));

      

        auto okButtonUnique = std::make_unique<TextButton>(GetUI());
        okButton = okButtonUnique.get();
        okButton->SetNonActiveColor({74, 114, 179, 255});
        okButton->SetHoverColor({98, 139, 202, 255});
        okButton->SetClickedColor({71, 114, 179, 255});
        okButton->SetLabelFontSize(static_cast<UI*>(GetUI())->GetTexturePack().fontSize);
        okButton->SetSize({(GetSize().x - 12 - PADDING) / 2, 24});
        okButton->SetPos(messageField->GetPos() + dr4::Vec2f(0, messageField->GetSize().y + 24));
        AddWidget(std::move(okButtonUnique));

        auto cancelButtonUnique = std::make_unique<TextButton>(GetUI());
        cancelButton = cancelButtonUnique.get();
        cancelButton->SetNonActiveColor({84, 84, 84, 255});
        cancelButton->SetHoverColor({101, 101, 101, 255});
        cancelButton->SetClickedColor({71, 114, 179, 255});
        cancelButton->SetLabelFontSize(static_cast<UI*>(GetUI())->GetTexturePack().fontSize);
        cancelButton->SetSize({(GetSize().x - 12 - PADDING) / 2, 24});
        cancelButton->SetPos(okButton->GetPos() + dr4::Vec2f(cancelButton->GetSize().x + PADDING, 0));
        cancelButton->SetOnPressAction([this] {  
            if (auto parent = GetParent()) {
                if (auto container = dynamic_cast<roa::Container*>(parent)) {
                    container->EraseWidget(this);
                }
            }
        });

        AddWidget(std::move(cancelButtonUnique));

        SetSize(GetSize());
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
        if (closeButton) closeButton->SetPos({GetSize().x - closeButton->GetSize().x - 6, 6});
        if (titleWidget) titleWidget->SetSize({GetSize().x - closeButton->GetSize().x - 18, 20});
        if (inputField) {
            inputField->SetSize({GetSize().x - 12, 24});
        }
        if (messageField) {
            messageField->SetSize({GetSize().x - 12, 24});
        }
        if (cancelButton) {
            cancelButton->SetSize({(GetSize().x - 12 - PADDING) / 2, 24});
        }
        if (okButton) {
            okButton->SetSize({(GetSize().x - 12 - PADDING) / 2, 24});
        }
        
        Window::OnSizeChanged();
    }

    void WindowDrawSelfAction() const override {
        // main background
        GetTexture().Clear({61, 61, 61});

        // draw title background area (kept inside titleWidget which has its own BG)
    }
};

} // namespace roa

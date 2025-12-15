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

    void InitLayout() {
        SetSize({400, 120});

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
        inputField->SetOnEnterAction([this](const std::string &text){
            try {
                namespace fs = std::filesystem;
                if (text.empty()) {
                    messageField->SetText("Provide filename");
                    messageField->SetColor(dr4::Color(200, 120, 0));
                } else if (fs::exists(fs::path(text))) {
                    messageField->SetText("Exists");
                    messageField->SetColor(dr4::Color(0, 200, 0));
                } else {
                    messageField->SetText("Not found");
                    messageField->SetColor(dr4::Color(200, 0, 0));
                }
            } catch (...) {
                messageField->SetText("Error checking file");
                messageField->SetColor(dr4::Color(200, 0, 0));
            }
        });
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

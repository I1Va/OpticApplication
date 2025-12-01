#include <iostream>
#include <functional>
#include <cassert>
#include <list>
#include <cmath>
#include <algorithm>

#include "hui/ui.hpp"
#include "cum/manager.hpp"
#include "cum/plugin.hpp"
#include "cum/ifc/dr4.hpp"

#include "BasicWidgets.hpp"
#include "MainWindow.hpp"
#include "hui/event.hpp"

const char FONT_PATH[] = "assets/RobotoFont.ttf";


namespace roa
{

class UI : public hui::UI {
    dr4::Font *defaultFont = nullptr;

public:
    UI(dr4::Window *window, const std::string &defaultFontPath): hui::UI(window) {
        assert(window);

        try {
            defaultFont = window->CreateFont();        
            if (defaultFont)
                defaultFont->LoadFromFile(defaultFontPath);
        } catch (std::runtime_error &e) {
            delete defaultFont;
            std::cerr << "roa::UI create defaultFont failed : " << e.what() << "\n";
            throw;
        }
    }
    ~UI() { if (defaultFont) delete defaultFont; }

    dr4::Font *GetDefaultFont() { return defaultFont; }
};








class VerticalScrollBar : public ZContainer<hui::Widget *> {
    static constexpr double BUTTON_LAYOUT_SHARE_ = 0.1; 
    static constexpr double THUMB_MOVING_DELTA = 0.05;

    std::function<void(double)> onScrollAction=nullptr;

    std::unique_ptr<TextureButton>  bottomButton = nullptr;
    std::unique_ptr<TextureButton>  topButton    = nullptr;
    std::unique_ptr<ThumbButton>    thumbButton  = nullptr;

public:
    VerticalScrollBar(hui::UI *ui): ZContainer(ui) {

        bottomButton = std::make_unique<TextureButton>(ui);
        topButton    = std::make_unique<TextureButton>(ui);
        thumbButton  = std::make_unique<ThumbButton>(ui);

        bottomButton->SetCapturedMode();
        topButton->SetCapturedMode();
        thumbButton->SetCapturedMode();

        thumbButton->SetUnpressedColor({255, 0, 255, 255});
        
        BecomeParentOf(bottomButton.get());
        BecomeParentOf(topButton.get());
        BecomeParentOf(thumbButton.get());

        initLayout();

        topButton->SetOnClickAction([this] { moveThumb(THUMB_MOVING_DELTA); });
        bottomButton->SetOnClickAction([this] { moveThumb(-THUMB_MOVING_DELTA); });
        thumbButton->SetOnReplaceAction([this] { percantageChanged(); });
    }

    ~VerticalScrollBar() = default;

    void BringToFront(hui::Widget *) override {}

    void SetOnScrollAction(std::function<void(double)> action) {
        onScrollAction = action;
    }

protected:
    hui::EventResult PropagateToChildren(hui::Event &event) override {
        assert(bottomButton);
        assert(topButton);
        assert(thumbButton);
    
        if (event.Apply(*thumbButton) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        if (event.Apply(*bottomButton) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        if (event.Apply(*topButton) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;

        return hui::EventResult::UNHANDLED;
    }

    void OnSizeChanged() override { reLayout(); }

protected:
    void Redraw() const override {
        assert(bottomButton);
        assert(topButton);

        GetTexture().Clear({100, 0, 0, 255});
        bottomButton->DrawOn(GetTexture());
        topButton->DrawOn(GetTexture());
        thumbButton->DrawOn(GetTexture());
    }

    hui::EventResult OnMouseDown(hui::MouseButtonEvent &event) override {
        if (GetRect().Contains(event.pos)) {
            event.pos -= GetPos();
            if (PropagateToChildren(event) == hui::EventResult::HANDLED) {
                event.pos += GetPos();
                return hui::EventResult::HANDLED;
            }

            bool handled = false;
            if (calculateThumbMovingArea().Contains(event.pos)) {
                thumbButton->SetPos(event.pos);
                handled = true;
                ForceRedraw();
            }

            event.pos += GetPos();
            return handled ? hui::EventResult::HANDLED : hui::EventResult::UNHANDLED;
        }
        return hui::EventResult::UNHANDLED;
    }

private:
    dr4::Vec2f calculateButtonSize() { 
        double buttonHeight = std::fmax(1, BUTTON_LAYOUT_SHARE_ * GetSize().y);
        double buttonWidth  = std::fmax(1, GetSize().x);
        return dr4::Vec2f(static_cast<float>(buttonWidth), static_cast<float>(buttonHeight));
    }

    dr4::Rect2f calculateThumbMovingArea() {
        dr4::Vec2f buttonSize = calculateButtonSize();
    
        dr4::Rect2f thumbMovingArea;
        thumbMovingArea.pos = bottomButton->GetPos() + dr4::Vec2f(0, buttonSize.y);
        thumbMovingArea.size = dr4::Vec2f(GetSize().x, GetSize().y - 2 * buttonSize.y);

        return thumbMovingArea;
    }

    dr4::Vec2f calculateThumbPos(double percentage) {
        percentage = std::clamp(percentage, 0.0, 1.0);

        dr4::Vec2f buttonSize = calculateButtonSize();
        double len = calculateThumbMovingArea().size.y - buttonSize.y;
        double y = buttonSize.y + len * percentage;

        return dr4::Vec2f(0, y);
    }

    double calculateThumbPercentage() {
        dr4::Vec2f buttonSize = calculateButtonSize();
        
        double y = thumbButton->GetPos().y - buttonSize.y; // - buttom button height
        double len = calculateThumbMovingArea().size.y - buttonSize.y; // - thumb button height

        return y / len;
    }

    void initLayout() {
        assert(bottomButton);
        assert(topButton);
        assert(thumbButton);

        dr4::Vec2f buttonSize = calculateButtonSize();
        dr4::Rect2f thumbMovingArea = calculateThumbMovingArea();

        bottomButton->SetPos({0, 0});
        bottomButton->SetSize(buttonSize);
        topButton->SetPos({0, GetSize().y - buttonSize.y});
        topButton->SetSize(buttonSize);
        
        thumbButton->SetSize(buttonSize);
        thumbButton->SetPos({0, buttonSize.y});
        thumbButton->SetMovingArea(thumbMovingArea);
    }
    
    void reLayout() {
        assert(bottomButton);
        assert(topButton);
        assert(thumbButton);

        initLayout();
    
        double percentage = calculateThumbPercentage();
        dr4::Rect2f thumbMovingArea = calculateThumbMovingArea();
        thumbButton->SetPos({0, static_cast<float>(thumbMovingArea.pos.y + thumbMovingArea.size.y * percentage)});
    }

    void moveThumb(double deltaPercent) {
        double percentage = calculateThumbPercentage();
        double newPercentage = std::clamp(percentage + deltaPercent, 0.0, 1.0);

        if (onScrollAction) onScrollAction(newPercentage);

        dr4::Vec2f newThumbPos = calculateThumbPos(newPercentage);
        thumbButton->SetPos(newThumbPos);
        percentage = newPercentage;
    }

    void percantageChanged() {
        if (onScrollAction) onScrollAction(calculateThumbPercentage());
    }

};







}

void runUI(roa::UI &ui) {
    double frameDelaySecs_ = 0.032;
    while (ui.GetWindow()->IsOpen()) {
        while (true) {
            auto evt = ui.GetWindow()->PollEvent();
            if (!evt.has_value()) break; 

            if (evt->type == dr4::Event::Type::QUIT ||
            (evt->type == dr4::Event::Type::KEY_DOWN && evt->key.sym == dr4::KeyCode::KEYCODE_ESCAPE)) {
                ui.GetWindow()->Close();
                break;
            }
            ui.ProcessEvent(evt.value());
        }

        double frameStartSecs = ui.GetWindow()->GetTime();
        hui::IdleEvent idleEvent;
        idleEvent.absTime = frameStartSecs;
        idleEvent.deltaTime = frameDelaySecs_;
        ui.OnIdle(idleEvent);

        ui.GetWindow()->Clear({50,50,50,255});
        if (ui.GetTexture()) ui.GetWindow()->Draw(*ui.GetTexture());
        ui.GetWindow()->Display();

        // ui.GetWindow()->Sleep(0.1);
    }
}


void printPercentage(double val) {
    std::cout << "Percentage : " << val << "%\n";
}

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        std::cerr << "Expected one argument: dr4 backend path\n";
        return 1;
    }
    const char *dr4BackendPath = argv[1];

    cum::Manager pluginManager;
    pluginManager.LoadFromFile(dr4BackendPath);
    auto *dr4Backend = pluginManager.GetAnyOfType<cum::DR4BackendPlugin>();
    assert(dr4Backend);

    dr4::Window *window = dr4Backend->CreateWindow(); assert(window);
    window->Open();
    window->StartTextInput();
    window->SetSize({800, 600});

    roa::UI ui(window, FONT_PATH);
    roa::MainWindow *mainWindow = new roa::MainWindow(&ui);
    mainWindow->SetSize({window->GetSize().x, window->GetSize().y});
    ui.SetRoot(mainWindow);



    roa::VerticalScrollBar *vScrollBar = new roa::VerticalScrollBar(&ui);  
    // vScrollBar->SetPos({200, 200});
    vScrollBar->SetSize({20, 300});
    vScrollBar->SetOnScrollAction(printPercentage);

    hui::MouseButtonEvent e;
    e.pressed = true;
    e.button = dr4::MouseButtonType::LEFT;
    e.pos = {0, 100};

    e.Apply(*vScrollBar);


    mainWindow->addWidget(vScrollBar);


    
    runUI(ui);
    
    delete window;
}
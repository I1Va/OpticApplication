#include <iostream>
#include <functional>
#include <cassert>
#include <list>
#include <cmath>

#include "hui/ui.hpp"
#include "cum/manager.hpp"
#include "cum/plugin.hpp"
#include "cum/ifc/dr4.hpp"

#include "BasicWidgets.hpp"
#include "MainWindow.hpp"

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
















// class ThumbButton : public SimpButton {
//     gm_dot<int, 2> accumulatedRel_ = {};

//     bool replaced_ = false;

//     SDL_Rect movingArea_ = {};

// public:
//     ThumbButton
//     (
//         int width, int height,
//         SDL_Rect movingArea,
//         const ButtonTexturePath texturePath,
//         std::function<void()> onClickFunction=nullptr, Widget *parent=nullptr
//     ): SimpButton(width, height, texturePath.unpressed, texturePath.pressed, onClickFunction, parent),
//        movingArea_(movingArea) {};

//     bool onMouseMoveSelfAction(const MouseMotionEvent &event) {
//         if (this == UIManager_->mouseActived() && event.button == SDL_BUTTON_LEFT) {
//             accumulatedRel_ += event.rel;
//             replaced_ = true;
//             return true;
//         }

//         return false;
//     }

    

//     void clampPos() {
//         rect_.x = std::clamp(rect_.x, movingArea_.x, movingArea_.x + movingArea_.w);
//         rect_.y = std::clamp(rect_.y, movingArea_.y, movingArea_.y + movingArea_.h);
//     }

//     bool updateSelfAction() {
//         if (replaced_) {
//             rect_.x += accumulatedRel_.x;
//             rect_.y += accumulatedRel_.y;

//             clampPos();
            
//             accumulatedRel_ = {0, 0};
//             replaced_ = false;
//             if (parent_) parent_->invalidate();
//             return true;
//         }
        
//         return false;
//     }
// };

class VerticalScrollBar : public ZContainer<hui::Widget *> {
    static constexpr double BUTTON_LAYOUT_SHARE_ = 0.1; 
    static constexpr double THUMB_MOVING_DELTA = 0.05;

    std::function<void(double)> onScroll_=nullptr;

    // gm_dot<int, 2> startThumbPos_;
    // double percentage_ = 0; 
    // bool isHorizontal_;

    // gm_dot<int, 2> buttonSize_ = {};

    // SDL_Rect thumbMovingArea_ = {};

    std::unique_ptr<SimpButton>  bottomButton = nullptr;
    std::unique_ptr<SimpButton>  topButton    = nullptr;
    // std::unique_ptr<ThumbButton> thumbButton  = nullptr;

public:
    VerticalScrollBar(hui::UI *ui): ZContainer(ui) {

        bottomButton = std::make_unique<SimpButton>(ui);
        topButton    = std::make_unique<SimpButton>(ui);

        bottomButton->SetCapturedMode();
        topButton->SetCapturedMode();
        
        BecomeParentOf(bottomButton.get());
        BecomeParentOf(topButton.get());

        layout();

        // topButton_ = new SimpButton(buttonSize_.x, buttonSize_.y, scrollBarTopBtnPath.unpressed, scrollBarTopBtnPath.pressed, 
        //     [this] { move(THUMB_MOVING_DELTA); }, this);
        // addWidget(( isHorizontal ? rect_.w - buttonSize_.x : 0), (!isHorizontal ? rect_.h - buttonSize_.y : 0), topButton_);

        // bottomButton_ = new SimpButton(buttonSize_.x, buttonSize_.y, scrollBarBottomBtnPath.unpressed, scrollBarBottomBtnPath.pressed, 
        //     [this] { move(-THUMB_MOVING_DELTA); }, this);
        // addWidget(0, 0, bottomButton_);

        // thumbMovingArea_.x = (isHorizontal ? buttonSize_.x : 0);
        // thumbMovingArea_.y = (!isHorizontal ? buttonSize_.y : 0);
        // thumbMovingArea_.w = (isHorizontal ? rect_.w - 3 * buttonSize_.x : 0);
        // thumbMovingArea_.h = (!isHorizontal ? rect_.h - 3 * buttonSize_.y : 0);

        // startThumbPos_ = getThumbPos(percentage_);
        
        // thumbButton_ = new ThumbButton(buttonSize_.x, buttonSize_.y, thumbMovingArea_, scrollThumbBtnPath, nullptr, this);
        // addWidget(startThumbPos_.x, startThumbPos_.y, thumbButton_);
    }

    ~VerticalScrollBar() = default;

    void BringToFront(hui::Widget *) override {}

protected:
    hui::EventResult PropagateToChildren(hui::Event &event) override {
        assert(bottomButton);
        assert(topButton);
        // assert(thumbButton);

        if (event.Apply(*bottomButton) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        if (event.Apply(*topButton) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        // if (event.Apply(*thumbButton) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;

        return hui::EventResult::UNHANDLED;
    }

    void OnSizeChanged() override { 
        layout();
    }

private:
    void layout() {
        assert(bottomButton);
        assert(topButton);
        // assert(thumbButton);

        double buttonHeight = std::fmax(1, BUTTON_LAYOUT_SHARE_ * GetSize().y);
        double buttonWidth  = std::fmax(1, GetSize().x);

        bottomButton->SetPos({0, 0});
        bottomButton->SetSize({static_cast<float>(buttonWidth), static_cast<float>(buttonHeight)});

        topButton->SetPos({0, static_cast<float>(GetSize().y - buttonHeight)});
        topButton->SetSize({static_cast<float>(buttonWidth), static_cast<float>(buttonHeight)});
    }



    // gm_dot<int, 2> getThumbPos(double percentage) {
    //     percentage = std::clamp(percentage, 0.0, 100.0);
    
    //     double len = (isHorizontal_? rect_.w - 3 * buttonSize_.x : rect_.h - 3 * buttonSize_.y);
    //     double x = (isHorizontal_? buttonSize_.x + percentage * len : 0);
    //     double y = (!isHorizontal_? buttonSize_.y + percentage * len : 0);
    //     return {(int) x, (int) y};
    // }

    // void move(double deltaPercent) {
    //     const double newPercentage = std::clamp(percentage_ + deltaPercent, 0.0, 1.0);
    //     const double realDelta = newPercentage - percentage_;

    //     if (onScroll_) onScroll_(newPercentage);

    //     gm_dot<int, 2> thumbPos = getThumbPos(percentage_);
        
    //     int relX = (isHorizontal_ ? realDelta : 0) * (rect_.w - 3 * buttonSize_.x);
    //     int relY = (!isHorizontal_ ? realDelta : 0) * (rect_.h - 3 * buttonSize_.y);

    //     thumbButton_->setPosition(thumbPos.x + relX, thumbPos.y + relY);
    //     thumbButton_->clampPos();

    //     percentage_ = newPercentage;
    // }

    // double getPercentFromRelativePos(int x, int y) {
    //     // std::cout << "val : " << x << " " << rect_.w - 3 * buttonSize_.x << "\n";
    //     if (isHorizontal_) return ((double) x) / (rect_.w - 3 * buttonSize_.x);
    //     return ((double) y) / (rect_.h - 3 * buttonSize_.y);
    // }





protected:
    void Redraw() const override {
        assert(bottomButton);
        assert(topButton);

        GetTexture().Clear({100, 0, 0, 255});
        bottomButton->DrawOn(GetTexture());
        topButton->DrawOn(GetTexture());
    
    }

    // bool updateSelfAction() override {
    //     bool updated = false;

    //     int dx = thumbButton_->rect().x - startThumbPos_.x;
    //     int dy = thumbButton_->rect().y - startThumbPos_.y;
    //     double newPercentage = getPercentFromRelativePos(dx, dy);

    //     if (newPercentage != percentage_) {
    //         percentage_ = newPercentage;
    //         setRerenderFlag();
    //         if (onScroll_) onScroll_(percentage_);
    //         updated = true;
    //     }
    
    //     return updated;
    // }

    // void renderSelfAction(SDL_Renderer* renderer) override {
    //     assert(renderer);

    //     SDL_Rect widgetRect = {0, 0, rect_.w, rect_.h};
    //     SDL_SetRenderDrawColor(renderer, DEFAULT_WINDOW_COLOR.r, DEFAULT_WINDOW_COLOR.g, DEFAULT_WINDOW_COLOR.b, DEFAULT_WINDOW_COLOR.a);
    //     SDL_RenderFillRect(renderer, &widgetRect);
    // }

    // bool onMouseDownSelfAction(const MouseButtonEvent &event) override {
    //     // hitting a free area
    //     return true;
    // }



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
    vScrollBar->SetPos({200, 200});
    vScrollBar->SetSize({20, 300});


    mainWindow->addWidget(vScrollBar);


    
    runUI(ui);
    
    delete window;
}
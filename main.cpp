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
#include "ScrollBar.hpp"
#include "TextWidgets.hpp"

const char FONT_PATH[] = "assets/RobotoFont.ttf";

namespace roa
{



// class ClickableTextWidget : public Button {
//     static constexpr SDL_Color PRESSED_BACK_COLOR = {128, 128, 128, 255};
//     static constexpr SDL_Color UNPRESSED_BACK_COLOR = WHITE_SDL_COLOR;
//     static constexpr SDL_Color TEXT_COLOR = BLACK_SDL_COLOR;
//     TextWidget textWidget_;

// public:
//     ClickableTextWidget
//     (   
//         int width, int height, 
//         const std::string &text,
//         TTF_Font *font,
//         std::function<void()> onDownFunction,
//         std::function<void()> onUpFunction,
//         Widget *parent=nullptr
//     ):  
//         Button(width, height, Button::STICKY, onDownFunction, onUpFunction, parent),
//         textWidget_(width, height, text, font, parent)
//     {}

// protected:

//     void setPressedTexture(SDL_Renderer* renderer) override {
//         assert(renderer);      
//         SDL_Rect buttonRect = {0, 0, rect_.w, rect_.h};
//         SDL_SetRenderDrawColor(renderer, PRESSED_BACK_COLOR.r, PRESSED_BACK_COLOR.g, PRESSED_BACK_COLOR.b, PRESSED_BACK_COLOR.a);
//         SDL_RenderFillRect(renderer, &buttonRect);

//         textWidget_.setBackColor(PRESSED_BACK_COLOR);
//         textWidget_.render(renderer);
    
//         SDL_Rect dst = textWidget_.rect();
//         SDL_RenderCopy(renderer, textWidget_.texture(), NULL, &dst);
//     }

//     void setUnPressedTexture(SDL_Renderer* renderer) override {
//         assert(renderer);      

//         SDL_Rect buttonRect = {0, 0, rect_.w, rect_.h};
//         SDL_SetRenderDrawColor(renderer, UNPRESSED_BACK_COLOR.r, UNPRESSED_BACK_COLOR.g, UNPRESSED_BACK_COLOR.b, UNPRESSED_BACK_COLOR.a);
//         SDL_RenderFillRect(renderer, &buttonRect);

//         textWidget_.setBackColor(UNPRESSED_BACK_COLOR);
//         textWidget_.render(renderer);
    
//         SDL_Rect dst = textWidget_.rect();
//         SDL_RenderCopy(renderer, textWidget_.texture(), NULL, &dst);
//     }
// };



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

        ui.GetWindow()->Sleep(frameDelaySecs_);
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



    roa::TextWidget *textField = new roa::TextWidget(&ui);
    textField->SetPos({300, 300});
    textField->SetSize({200, 30});

    roa::TextWidget *textMirror = new roa::TextWidget(&ui);
    textMirror->SetPos({300, 400});
    textMirror->SetSize({200, 30});



    roa::VerticalScrollBar *vScrollBar = new roa::VerticalScrollBar(&ui);  
    vScrollBar->SetPos({200, 200});
    vScrollBar->SetSize({20, 300});
    vScrollBar->SetOnScrollAction([textField](double percentage) {textField->SetText(std::to_string(percentage)); });
    
    roa::TextInputWidget *textInput = new roa::TextInputWidget(&ui);
    textInput->SetPos({300, 350});
    textInput->SetSize({200, 30});
    textInput->setOnEnterAction([textMirror](const std::string &content) {textMirror->SetText(content); });

    mainWindow->addWidget(vScrollBar);
    mainWindow->addWidget(textField);
    mainWindow->addWidget(textInput);
    mainWindow->addWidget(textMirror);

    
    runUI(ui);
    
    delete window;
}
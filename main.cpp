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



class TextWidget : public hui::Widget {
protected:
    std::unique_ptr<dr4::Text> text;
    dr4::Color backColor = WHITE;

public:
    TextWidget(hui::UI *ui): hui::Widget(ui), text(GetUI()->GetWindow()->CreateText()) { 
        assert(ui); 
        text->SetFont(static_cast<UI *>(GetUI())->GetDefaultFont());
    }

    void SetFont(dr4::Font *font) {
        assert(font);

        text->SetFont(font);
        ForceRedraw();
    }

    void SetText(const std::string &content) {
        text->SetText(content);
        ForceRedraw();
    }

protected:

    void Redraw() const override {
        GetTexture().Clear(backColor);
        text->DrawOn(GetTexture());
    }
};



// class TextInputWidget : public TextWidget {
//     std::function<void(const std::string&)> onEnter_;
//     bool needOnEnterCall_ = false;
// public:
//     TextInputWidget
//     (
//         const std::size_t width, const std::size_t height,
//         const std::string &text,
//         TTF_Font *font,
//         std::function<void(const std::string)> onEnter=nullptr,
//         Widget *parent=nullptr
//     ):  
//         TextWidget(width, height, text, font, parent),
//         onEnter_(onEnter) {}

//     bool updateSelfAction() {
//         if (needOnEnterCall_) {
//             if (onEnter_) onEnter_(text_);
//             needOnEnterCall_ = false;
//             return true;
//         }

//         return false;
//     }

//     bool onKeyDownSelfAction(const KeyEvent &event) override {
//         if (event.sym == SDLK_BACKSPACE && !text_.empty()) {
//             text_.pop_back();
//             setRerenderFlag();
//             return CONSUME;
//         }

//         if (event.sym >= SDLK_0 && event.sym <= SDLK_9) {
//             text_.push_back(static_cast<char>('0' + (event.sym - SDLK_0)));
//             setRerenderFlag();
//             return CONSUME;
//         }

//         if (event.sym >= SDLK_a && event.sym <= SDLK_z) {
//             bool shift = (event.keymod & KMOD_SHIFT);
//             char c = static_cast<char>(shift ? ('A' + (event.sym - SDLK_a))
//                                              : ('a' + (event.sym - SDLK_a)));
//             text_.push_back(c);
//             setRerenderFlag();
//             return CONSUME;
//         }

//         if (event.sym == SDLK_MINUS) {
//             text_.push_back('-');
//             setRerenderFlag();
//             return CONSUME;
//         }

//         if (event.sym == SDLK_SPACE) {
//             text_.push_back(' ');
//             setRerenderFlag();
//             return CONSUME;
//         }

//         if (event.sym == SDLK_PERIOD) {
//             text_.push_back('.');
//             setRerenderFlag();
//             return CONSUME;
//         }

//         if (event.sym == SDLK_COMMA) {
//             text_.push_back(',');
//             setRerenderFlag();
//             return CONSUME;
//         }

//         if (event.sym == SDLK_KP_ENTER || event.sym == SDLK_RETURN) {
//             setRerenderFlag();
//             needOnEnterCall_ = true;
//             return CONSUME;
//         }

//         return PROPAGATE;
//     }

//     void setOnEnter( std::function<void(const std::string &)> onEnter) { onEnter_ = onEnter; }
// };

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



    roa::TextWidget *textField = new roa::TextWidget(&ui);
    textField->SetPos({300, 300});
    textField->SetSize({200, 30});
    

    roa::VerticalScrollBar *vScrollBar = new roa::VerticalScrollBar(&ui);  
    vScrollBar->SetPos({200, 200});
    vScrollBar->SetSize({20, 300});
    vScrollBar->SetOnScrollAction([textField](double percantage) {textField->SetText(std::to_string(percantage)); });
    
    



    mainWindow->addWidget(vScrollBar);
    mainWindow->addWidget(textField);

    
    runUI(ui);
    
    delete window;
}
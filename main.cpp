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


    mainWindow->addWidget(vScrollBar);


    
    runUI(ui);
    
    delete window;
}
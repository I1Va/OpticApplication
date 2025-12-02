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

#include "Buttons.hpp"
#include "MainWindow.hpp"
#include "ScrollBar.hpp"
#include "TextWidgets.hpp"

const char FONT_PATH[] = "assets/RobotoFont.ttf";

namespace roa
{


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

    roa::TextButton *textButton = new roa::TextButton(&ui);
    textButton->SetText("I am button");
    textButton->SetPos({300, 450});
    textButton->SetSize({200, 30});


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
    mainWindow->addWidget(textButton);

    
    runUI(ui);
    
    delete window;
}
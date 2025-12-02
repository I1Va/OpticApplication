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
#include "SceneWidgets.hpp"

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

roa::SceneWidget *createSceneWidget(hui::UI *ui) {
    assert(ui);

    RTMaterial *groundMaterial = new RTLambertian({0.8, 0.8, 0.0}); 
    RTMaterial *midSphereMaterial = new RTLambertian({0.1, 0.2, 0.5});
    RTMaterial *rightSphereMaterial = new RTMetal({0.8, 0.8, 0.8}, 0.3);
    RTMaterial *glassMaterial = new RTDielectric({1.0, 1.0, 1.0}, 1.50);
    RTMaterial *sunMaterial = new RTEmissive(gm::IVec3f(1.0, 0.95, 0.9) * 10);

    roa::SceneWidget *scene = new roa::SceneWidget(ui);

    SphereObject *sun = new SphereObject(1, sunMaterial, &scene->GetSceneManager());

    Light *light = new Light
    (
        /* ambientIntensity  */  gm::IVec3f(0.2, 0.2, 0.2),
        /* defuseIntensity   */  gm::IVec3f(0.8, 0.7, 0.6),
        /* specularIntensity */  gm::IVec3f(0.7, 0.7, 0),
        /* viewLightPow      */  15.0
    );

    SphereObject    *midSphere = new SphereObject(1, midSphereMaterial, &scene->GetSceneManager());
    SphereObject    *rightSphere = new SphereObject(1, rightSphereMaterial, &scene->GetSceneManager());
  
    PlaneObject     *ground = new PlaneObject({0, 0, 0}, {0, 0, 1}, groundMaterial, &scene->GetSceneManager());

    SphereObject    *glassSphere = new SphereObject(1, glassMaterial, &scene->GetSceneManager());


    scene->AddObject({0, 0, -100}, ground);
    scene->AddObject({0, 0, 1}, glassSphere);
    // GetSceneManager.AddObject({-2, 0, 1}, leftBubbleSphere);
    scene->AddObject({0, 4, 3}, midSphere);
    scene->AddObject({2, 0, 1}, rightSphere);

    scene->AddLight({0, 0, 10}, light);
    scene->AddObject({-2, 0, 4}, sun);

    return scene;
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
    textField->SetSize({200, 30});

    roa::TextWidget *textMirror = new roa::TextWidget(&ui);
    textMirror->SetSize({200, 30});

    roa::TextButton *textButton = new roa::TextButton(&ui);
    textButton->SetText("I am button");
    textButton->SetSize({200, 30});


    roa::VerticalScrollBar *vScrollBar = new roa::VerticalScrollBar(&ui);  
    vScrollBar->SetSize({20, 300});
    vScrollBar->SetOnScrollAction([textField](double percentage) {textField->SetText(std::to_string(percentage)); });
    
    roa::TextInputWidget *textInput = new roa::TextInputWidget(&ui);
    textInput->SetSize({200, 30});
    textInput->setOnEnterAction([textMirror](const std::string &content) {textMirror->SetText(content); });


    roa::SceneWidget *scene = createSceneWidget(&ui);
    scene->SetSize({100, 100});

    



    vScrollBar->SetPos({200, 200}); mainWindow->addWidget(vScrollBar);
    textField->SetPos({300, 300});  mainWindow->addWidget(textField);
    textInput->SetPos({300, 350});  mainWindow->addWidget(textInput);
    textMirror->SetPos({300, 400}); mainWindow->addWidget(textMirror);
    textButton->SetPos({300, 450}); mainWindow->addWidget(textButton);
    scene->SetPos({0, 0});          mainWindow->addWidget(scene);


    runUI(ui);
    
    delete window;
}
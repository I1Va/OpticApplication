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
#include "ObjectsPanel.hpp"
#include "EditorWidget.hpp"

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

void createSceneObjects
(
    SceneManager &sceneManager,
    RTMaterialManager &materialManager,
    std::vector<Primitives *> &primitives,
    std::vector<Light *> &lights 
) {
    RTMaterial *groundMaterial      = materialManager.MakeLambertian({0.8, 0.8, 0.0}); 
    RTMaterial *midSphereMaterial   = materialManager.MakeLambertian({0.1, 0.2, 0.5});
    RTMaterial *rightSphereMaterial = materialManager.MakeMetal({0.8, 0.8, 0.8}, 0.3);
    RTMaterial *glassMaterial       = materialManager.MakeDielectric({1.0, 1.0, 1.0}, 1.50);
    RTMaterial *sunMaterial         = materialManager.MakeEmissive(gm::IVec3f(1.0, 0.95, 0.9) * 10);

    SphereObject *sun = new SphereObject(1, sunMaterial, &sceneManager);
    Light *light = new Light
    (
        /* ambientIntensity  */  gm::IVec3f(0.2, 0.2, 0.2),
        /* defuseIntensity   */  gm::IVec3f(0.8, 0.7, 0.6),
        /* specularIntensity */  gm::IVec3f(0.7, 0.7, 0),
        /* viewLightPow      */  15.0
    );

    SphereObject    *midSphere = new SphereObject(1, midSphereMaterial, &sceneManager);
    SphereObject    *rightSphere = new SphereObject(1, rightSphereMaterial, &sceneManager);
    PlaneObject     *ground = new PlaneObject({0, 0, 0}, {0, 0, 1}, groundMaterial, &sceneManager);
    SphereObject    *glassSphere = new SphereObject(1, glassMaterial, &sceneManager);


    for (int i = 0; i < 10; i++) {
        SphereObject *sphere = new SphereObject(1, midSphereMaterial, &sceneManager);
        sphere->setPosition({static_cast<float>(i), static_cast<float>(i), static_cast<float>(i)});
        primitives.push_back(sphere);
    }


    ground->setPosition({0, 0, -100});
    glassSphere->setPosition({0, 0, 1});
    midSphere->setPosition({0, 4, 3});
    rightSphere->setPosition({2, 0, 1});
    sun->setPosition({-2, 0, 4});

    light->setPosition({0, 0, 10});

    primitives.push_back(ground);
    primitives.push_back(glassSphere);
    primitives.push_back(midSphere);
    primitives.push_back(sun);
    primitives.push_back(rightSphere);

    lights.push_back(light);
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



    

    RTMaterialManager materialManager;



    roa::EditorWidget *editor = new roa::EditorWidget(&ui);
    std::vector<Primitives *> primitives;
    std::vector<Light *> lights;

    createSceneObjects(editor->GetSceneManager(), materialManager, primitives, lights);

    for (auto prim : primitives) {
        editor->AddObject(prim);
    }
    for (auto light : lights) {
        editor->AddLight(light);
    }
    
    editor->SetSize({700, 500});


    // vScrollBar->SetPos({200, 200}); mainWindow->addWidget(vScrollBar);
    // textField->SetPos({300, 300});  mainWindow->addWidget(textField);
    // textInput->SetPos({300, 350});  mainWindow->addWidget(textInput);
    // textMirror->SetPos({300, 400}); mainWindow->addWidget(textMirror);
    // textButton->SetPos({300, 450}); mainWindow->addWidget(textButton);
    // inputField->SetPos({300, 500}); mainWindow->addWidget(inputField);

    editor->SetPos({100, 100});     mainWindow->addWidget(editor);


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


    // runUI(ui);
    
    delete window;
}
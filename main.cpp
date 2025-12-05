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
#include "cum/ifc/pp.hpp"

#include "Buttons.hpp"
#include "MainWindow.hpp"
#include "ScrollBar.hpp"
#include "TextWidgets.hpp"
#include "SceneWidgets.hpp"
#include "ObjectsPanel.hpp"
#include "EditorWidget.hpp"
#include "PPWidgets.hpp"

const char FONT_PATH[] = "assets/RobotoFont.ttf";


void createSceneObjects
(
    RTMaterialManager &materialManager,
    roa::EditorWidget *editor
) {
    RTMaterial *groundMaterial      = materialManager.MakeLambertian({0.8, 0.8, 0.0}); 
    RTMaterial *midSphereMaterial   = materialManager.MakeLambertian({0.1, 0.2, 0.5});
    RTMaterial *rightSphereMaterial = materialManager.MakeMetal({0.8, 0.8, 0.8}, 0.3);
    RTMaterial *glassMaterial       = materialManager.MakeDielectric({1.0, 1.0, 1.0}, 1.50);
    RTMaterial *sunMaterial         = materialManager.MakeEmissive(gm::IVec3f(1.0, 0.95, 0.9) * 10);

    SphereObject *sun = new SphereObject(1, sunMaterial, &editor->GetSceneManager());
    Light *light = new Light
    (
        /* ambientIntensity  */  gm::IVec3f(0.2, 0.2, 0.2),
        /* defuseIntensity   */  gm::IVec3f(0.8, 0.7, 0.6),
        /* specularIntensity */  gm::IVec3f(0.7, 0.7, 0),
        /* viewLightPow      */  15.0
    );

    SphereObject    *midSphere = new SphereObject(1, midSphereMaterial, &editor->GetSceneManager());
    SphereObject    *rightSphere = new SphereObject(1, rightSphereMaterial, &editor->GetSceneManager());
    PlaneObject     *ground = new PlaneObject({0, 0, 0}, {0, 0, 1}, groundMaterial, &editor->GetSceneManager());
    SphereObject    *glassSphere = new SphereObject(1, glassMaterial, &editor->GetSceneManager());


    for (int i = 0; i < 10; i++) {
        SphereObject *sphere = new SphereObject(1, midSphereMaterial, &editor->GetSceneManager());
        sphere->setPosition({static_cast<float>(i), static_cast<float>(i), static_cast<float>(i)});
        editor->AddObject(sphere);
    }


    ground->setPosition({0, 0, -100});
    glassSphere->setPosition({0, 0, 1});
    midSphere->setPosition({0, 4, 3});
    rightSphere->setPosition({2, 0, 1});
    sun->setPosition({-2, 0, 4});

    light->setPosition({0, 0, 10});

    editor->AddObject(ground);
    editor->AddObject(glassSphere);
    editor->AddObject(midSphere);
    editor->AddObject(sun);
    editor->AddObject(rightSphere);

    editor->AddLight(light);
}

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        std::cerr << "Expected one argument: dr4 backend path\n";
        return 1;
    }
    cum::Manager pluginManager;

// SETUP DR4 PLUGIN
    const char *dr4BackendPath = argv[1];
    pluginManager.LoadFromFile(dr4BackendPath);
    auto *dr4Backend = pluginManager.GetAnyOfType<cum::DR4BackendPlugin>();
    assert(dr4Backend);

    dr4::Window *window = dr4Backend->CreateWindow(); assert(window);
    window->Open();
    window->StartTextInput();
    window->SetSize({800, 600});
// SETUP UI, MAIN WINDOW
    roa::UI ui(window, FONT_PATH);
    roa::MainWindow *mainWindow = new roa::MainWindow(&ui);
    mainWindow->SetSize({window->GetSize().x, window->GetSize().y});
    ui.SetRoot(mainWindow);


// SETUP PP PLUGIN
    std::vector<cum::PPToolPlugin*> ppPlugins = {};
    
    auto ppPlugin1 = dynamic_cast<cum::PPToolPlugin*>(pluginManager.LoadFromFile("./external/plugins/pp/libIADorisovkaPlugin.so")); assert(ppPlugin1);
    // auto ppPlugin2 = dynamic_cast<cum::PPToolPlugin*>(pluginManager.LoadFromFile("./external/plugins/pp/PPdenchick.so")); assert(ppPlugin2);
    

    ppPlugins.push_back(ppPlugin1);
    // ppPlugins.push_back(ppPlugin2);
    // push other plugins

    // roa::    PPCanvasWidget *canvas = new roa::PPCanvasWidget(&ui, ppPlugins);

    // TODO : add shortCuts
    // TODO : add hide option
    // mainWindow->addModal(canvas);


 // SETUP SCENE OBJECTS
    RTMaterialManager materialManager;
    roa::EditorWidget *editor = new roa::EditorWidget(&ui);
    createSceneObjects(materialManager, editor);
    editor->SetSize({100, 100});
    editor->SetPos({100, 100});     
    mainWindow->AddWidget(editor);

// MODALS

    roa::PPCanvasWidget *ppCanvas = new roa::PPCanvasWidget(&ui, ppPlugins);
    mainWindow->SetModal(ppCanvas);
    ui.AddHotkey({dr4::KeyCode::KEYCODE_D, dr4::KeyMode::KEYMOD_CTRL}, [mainWindow](){mainWindow->SwitchModalActiveFlag(); });

// MAIN LOOP

    ui.Run();

// CLEANUP
    delete window;
}

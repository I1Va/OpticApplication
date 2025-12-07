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
#include "Viewport3D.hpp"
#include "RecordsPanel.hpp"

#include "PPWidgets.hpp"
#include "Window.hpp"

const char FONT_PATH[] = "assets/RobotoFont.ttf";


void createSceneObjects
(
    RTMaterialManager &materialManager,
    roa::Viewport3DWindow *viewport3D
) {
    RTMaterial *groundMaterial      = materialManager.MakeLambertian({0.8, 0.8, 0.0}); 
    RTMaterial *midSphereMaterial   = materialManager.MakeLambertian({0.1, 0.2, 0.5});
    RTMaterial *rightSphereMaterial = materialManager.MakeMetal({0.8, 0.8, 0.8}, 0.3);
    RTMaterial *glassMaterial       = materialManager.MakeDielectric({1.0, 1.0, 1.0}, 1.50);
    RTMaterial *sunMaterial         = materialManager.MakeEmissive(gm::IVec3f(1.0, 0.95, 0.9) * 10);

    SphereObject *sun = new SphereObject(1, sunMaterial, &viewport3D->GetSceneManager());
    Light *light = new Light
    (
        /* ambientIntensity  */  gm::IVec3f(0.2, 0.2, 0.2),
        /* defuseIntensity   */  gm::IVec3f(0.8, 0.7, 0.6),
        /* specularIntensity */  gm::IVec3f(0.7, 0.7, 0),
        /* viewLightPow      */  15.0
    );

    SphereObject    *midSphere = new SphereObject(1, midSphereMaterial, &viewport3D->GetSceneManager());
    SphereObject    *rightSphere = new SphereObject(1, rightSphereMaterial, &viewport3D->GetSceneManager());
    PlaneObject     *ground = new PlaneObject({0, 0, 0}, {0, 0, 1}, groundMaterial, &viewport3D->GetSceneManager());
    SphereObject    *glassSphere = new SphereObject(1, glassMaterial, &viewport3D->GetSceneManager());


    for (int i = 0; i < 10; i++) {
        SphereObject *sphere = new SphereObject(1, midSphereMaterial, &viewport3D->GetSceneManager());
        sphere->setPosition({static_cast<float>(i), static_cast<float>(i), static_cast<float>(i)});
        viewport3D->AddObject(sphere);
    }


    ground->setPosition({0, 0, -100});
    glassSphere->setPosition({0, 0, 1});
    midSphere->setPosition({0, 4, 3});
    rightSphere->setPosition({2, 0, 1});
    sun->setPosition({-2, 0, 4});

    light->setPosition({0, 0, 10});

    viewport3D->AddObject(ground);
    viewport3D->AddObject(glassSphere);
    viewport3D->AddObject(midSphere);
    viewport3D->AddObject(sun);
    viewport3D->AddObject(rightSphere);

    viewport3D->AddLight(light);
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

    dr4::Font *defaultFont = window->CreateFont();
    defaultFont->LoadFromFile("/usr/share/fonts/TTF/Hack-Bold.ttf");
    window->SetDefaultFont(defaultFont);

// SETUP UI, MAIN WINDOW
    roa::UI ui(window, FONT_PATH);
    roa::MainWindow *mainWindow = new roa::MainWindow(&ui);
    mainWindow->SetSize({window->GetSize().x, window->GetSize().y});
    ui.SetRoot(mainWindow);



    // roa::Window *editorWindow = new roa::Window(&ui);
    // editorWindow->SetSize({560, 420});
    // editorWindow->SetPos({100, 100});

    // mainWindow->AddWidget(editorWindow);

    





// SETUP PP PLUGIN
    std::vector<cum::PPToolPlugin*> ppPlugins = {};
    
    std::vector<std::string> ppPluginsPathes = 
    {
        "./external/plugins/pp/libIADorisovkaPlugin.so",
        "./external/plugins/pp/libArtemLine.so",
        "external/plugins/pp/libSeva.so"
    };
    
    for (auto path : ppPluginsPathes) {
        auto plugin = dynamic_cast<cum::PPToolPlugin*>(pluginManager.LoadFromFile(path));
        assert(plugin);
        ppPlugins.push_back(plugin);
    }

    
 // SETUP SCENE OBJECTS


    roa::Viewport3DWindow *Viewport3D = new roa::Viewport3DWindow(&ui);
    RTMaterialManager materialManager;
    createSceneObjects(materialManager, Viewport3D);

    float padding = 3;
    roa::OutlinerWindow<Primitives *> *outliner = new roa::OutlinerWindow<Primitives *>(&ui);

    float Viewport3DWHCoef = 1.8;
    float Viewport3DHeight = 300;
    Viewport3D->SetSize({Viewport3DWHCoef * Viewport3DHeight, Viewport3DHeight});

    float objectsPanelHWCoef = 1;
    float objectsPanelHeight = 200;
    outliner->SetSize({objectsPanelHWCoef * objectsPanelHeight, objectsPanelHeight});

    dr4::Vec2f outlinerWindowPos = {Viewport3D->GetSize().x + padding, 0};
    outliner->SetPos(outlinerWindowPos);

    for (Primitives *primitive : Viewport3D->GetPrimitives()) {
        outliner->AddObject(primitive, "name", nullptr, nullptr);
    }

    mainWindow->AddWidget(Viewport3D);
    mainWindow->AddWidget(outliner);



// MODALS

    roa::PPCanvasWidget *ppCanvas = new roa::PPCanvasWidget(&ui, ppPlugins);
    mainWindow->SetModal(ppCanvas);
    ui.AddHotkey({dr4::KeyCode::KEYCODE_D, dr4::KeyMode::KEYMOD_CTRL}, [mainWindow](){mainWindow->SwitchModalActiveFlag(); });

// MAIN LOOP

    ui.Run(0.01);

// CLEANUP
    delete window;
}

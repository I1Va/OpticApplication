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

#include "BasicWidgets/Buttons.hpp"
#include "BasicWidgets/MainWindow.hpp"
#include "BasicWidgets/ScrollBar.hpp"
#include "BasicWidgets/TextWidgets.hpp"
#include "RecordsPanel.hpp"
#include "DropDownMenu.hpp"
#include "PropertiesPanel.hpp"

#include "PPWidgets.hpp"
#include "BasicWidgets/Window.hpp"

#include "EditorWidget.hpp"

const static char FONT_PATH[] = "assets/RobotoFont.ttf";

const static roa::TexturePack ICONS_TEXTURE_PACK = 
{
    .outlinerObMeshSvgPath = "assets/icons/OutlinerObMesh.svg",
    .collectionSvgPath     = "assets/icons/Collection.svg",
    .triaDownSvgPath       = "assets/icons/TriaDown.svg",
    .triaRightSvgPath      = "assets/icons/TriaRight.svg",

    .whiteTextColor = dr4::Color(222, 222, 222),
    .fontSize = 11,

    .propertiesPanelBGColor = dr4::Color(48, 48, 48)
};

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


    // for (int i = 0; i < 3; i++) {
    //     RTMaterial *sphereMaterial = materialManager.MakeMetal({0.1 + 0.2 * i, 0.2 + 0.2 * i, 0.3 +0.2 * i}, 0.3);
    //     SphereObject *sphere = new SphereObject(1, sphereMaterial, &editor->GetSceneManager());
    //     sphere->setPosition({static_cast<float>(i), static_cast<float>(i), static_cast<float>(i)});
    //     editor->AddRecord(sphere);
    // }

    ground->setPosition({0, 0, -2});
    glassSphere->setPosition({0, 0, 1});
    midSphere->setPosition({0, 4, 3});
    rightSphere->setPosition({2, 0, 1});
    sun->setPosition({-2, 0, 4});

    light->setPosition({0, 0, 10});

    editor->AddRecord(ground);
    editor->AddRecord(glassSphere);

    editor->AddRecord(midSphere);
    editor->AddRecord(sun);

    editor->AddRecord(rightSphere);
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

    dr4::Font *defaultFont = window->CreateFont();
    defaultFont->LoadFromFile("./assets/Inter.ttf");
    window->SetDefaultFont(defaultFont);

// SETUP UI, MAIN WINDOW
    roa::UI ui(window, FONT_PATH);
    ui.SetTexturePack(ICONS_TEXTURE_PACK);
    roa::MainWindow *mainWindow = new roa::MainWindow(&ui);
    mainWindow->SetSize({window->GetSize().x, window->GetSize().y});
    ui.SetRoot(mainWindow);

    
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

    RTMaterialManager materialManager;

    roa::EditorWidget *editor = new roa::EditorWidget(&ui);
    editor->SetSize(mainWindow->GetSize());

    createSceneObjects(materialManager, editor);
    
   
    mainWindow->AddWidget(editor);

    




// MODALS

    roa::PPCanvasWidget *ppCanvas = new roa::PPCanvasWidget(&ui, ppPlugins);
    mainWindow->SetModal(ppCanvas);
    ui.AddHotkey({dr4::KeyCode::KEYCODE_D, dr4::KeyMode::KEYMOD_CTRL}, [mainWindow](){mainWindow->SwitchModalActiveFlag(); });

// MAIN LOOP

    ui.Run(0.01);

// CLEANUP
    delete window;
}

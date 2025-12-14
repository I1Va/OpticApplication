#include <iostream>
#include <functional>
#include <cassert>
#include <list>
#include <cmath>
#include <algorithm>
#include <memory>
#include <vector>
#include <string>

#include "hui/ui.hpp"
#include "cum/manager.hpp"
#include "cum/plugin.hpp"
#include "cum/ifc/dr4.hpp"
#include "cum/ifc/pp.hpp"

// #include "PPWidgets.hpp"
#include "BasicWidgets/Desktop.hpp"
#include "BasicWidgets/Buttons.hpp"
#include "CompositeWidgets/Outliner.hpp"
#include "CompositeWidgets/EditorWidget.hpp"

const static char FONT_PATH[] = "assets/RobotoFont.ttf";

const static roa::TexturePack ICONS_TEXTURE_PACK =
{
    .outlinerObMeshSvgPath = "assets/icons/OutlinerObMesh.svg",
    .collectionSvgPath     = "assets/icons/Collection.svg",
    .triaDownSvgPath       = "assets/icons/TriaDown.svg",
    .triaRightSvgPath      = "assets/icons/TriaRight.svg",
    .fileSaveIconPath      = "assets/icons/fileFolder.svg",
    .fileLoadIconPath      = "assets/icons/file.svg",
    .addIconPath           = "assets/icons/add.svg",

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

    roa::Desktop *desktop = new roa::Desktop(&ui);
    ui.SetRoot(desktop);

// SETUP PP PLUGIN
    std::vector<cum::PPToolPlugin*> ppPlugins;
    std::vector<std::string> ppPluginsPathes =
    {
        "external/plugins/pp/libIADorisovkaPlugin.so",
        "external/plugins/pp/libArtemLine.so",
        "external/plugins/pp/libSeva.so"
    };

    for (auto &path : ppPluginsPathes) {
        auto plugin = dynamic_cast<cum::PPToolPlugin*>(pluginManager.LoadFromFile(path));
        assert(plugin);
        ppPlugins.push_back(plugin);
    }

// SETUP SCENE OBJECTS
    RTMaterialManager materialManager;
    auto editor = std::make_unique<roa::EditorWidget>(&ui);
    editor->SetSize(desktop->GetSize());
    createSceneObjects(materialManager, editor.get());
    desktop->AddWidget(std::move(editor)); 
    


// MAIN MENU
    auto fileItem = std::make_unique<roa::DropDownMenu>(&ui);
    fileItem->SetSize({50, desktop->MAIN_MENU_HEIGHT});
    fileItem->SetBorderThinkess(1);
    fileItem->SetBorderColor(roa::WHITE);
    fileItem->SetLabel("file");
    auto saveDropDown = std::make_unique<roa::Outliner<int *>>(&ui);
    saveDropDown->SetSize({70, 40});
    saveDropDown->SetRecordIconStartPos({5, 3});
    saveDropDown->SetRecordIconSize({14, 14});
    saveDropDown->SetBGColor(desktop->BGColor);
    saveDropDown->SetRecordButtonMode(roa::Button::Mode::CAPTURE_MODE);
    
    saveDropDown->AddRecord(nullptr, "Save", [](){
        std::cout << "save!\n";
    }, nullptr, ui.GetTexturePack().fileSaveIconPath);

    saveDropDown->AddRecord(nullptr, "Load", [](){
        std::cout << "load!\n";
    }, nullptr, ui.GetTexturePack().fileLoadIconPath);

    fileItem->SetDropDownWidget(std::move(saveDropDown));
    desktop->AddMaiMenuItem(std::move(fileItem));


    auto pluginItem = std::make_unique<roa::DropDownMenu>(&ui);
    pluginItem->SetBorderThinkess(1);
    pluginItem->SetBorderColor(roa::WHITE);

    pluginItem->SetSize({70, desktop->MAIN_MENU_HEIGHT});
    pluginItem->SetLabel("plugins");
    auto pluginDropDown = std::make_unique<roa::Outliner<int *>>(&ui);
    pluginDropDown->SetSize({100, 20});
    pluginDropDown->SetRecordIconStartPos({5, 3});
    pluginDropDown->SetRecordIconSize({14, 14});
    pluginDropDown->SetBGColor(desktop->BGColor);
    pluginDropDown->SetRecordButtonMode(roa::Button::Mode::CAPTURE_MODE);
    
    pluginDropDown->AddRecord(nullptr, "Add pp plugin", [](){
        std::cout << "open plugin add widget!\n";
    }, nullptr, ui.GetTexturePack().addIconPath);


    pluginItem->SetDropDownWidget(std::move(pluginDropDown));
    desktop->AddMaiMenuItem(std::move(pluginItem));

// 

// MAIN LOOP
    ui.Run(0.01);

// CLEANUP
    delete window;
}

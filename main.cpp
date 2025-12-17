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
#include "BasicWidgets/TextWindow.hpp"
#include "PP/PPWidgets.hpp"
#include "CustomWidgets.hpp"

const static char FONT_PATH[] = "assets/RobotoFont.ttf";

const static roa::TexturePack ICONS_TEXTURE_PACK =
{
    .outlinerObMeshSvgPath      = "assets/icons/meshes/mesh_data.svg",
    .outlinerSphereIconPath     = "assets/icons/meshes/mesh_uvsphere.svg",
    .outlinerPlaneIconPath      = "assets/icons/meshes/mesh_plane.svg",
    .outlinerPolygonIconPath    = "assets/icons/meshes/mesh_polygon.svg",
    .outlinerCubeIconPath       = "assets/icons/meshes/mesh_cube.svg",

    .collectionSvgPath          = "assets/icons/Collection.svg",
    .triaDownSvgPath            = "assets/icons/TriaDown.svg",
    .triaRightSvgPath           = "assets/icons/TriaRight.svg",
    .fileSaveIconPath           = "assets/icons/fileFolder.svg",
    .fileLoadIconPath           = "assets/icons/file.svg",
    .addIconPath                = "assets/icons/add.svg",

    .whiteTextColor = dr4::Color(222, 222, 222),
    .fontSize = 11,

    .propertiesPanelBGColor = dr4::Color(48, 48, 48)
};

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

    roa::OpticDesktop *desktop = new roa::OpticDesktop(&ui);
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

// PP

    auto ppCanvas = std::make_unique<roa::PPCanvasWidget>(&ui, ppPlugins);
    desktop->SetModal(std::move(ppCanvas));
    ui.AddHotkey({dr4::KeyCode::KEYCODE_D, dr4::KeyMode::KEYMOD_CTRL}, [desktop](){desktop->SwitchModalActiveFlag(); });

// MAIN LOOP
    ui.Run(0.01);

// CLEANUP
    delete window;
}

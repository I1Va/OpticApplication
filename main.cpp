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

namespace roa {

class FileItem final : public DropDownMenu {
public:
    FileItem(Desktop *desktop, roa::EditorWidget *editor): DropDownMenu(desktop->GetUI()) {
        SetSize({50, desktop->MAIN_MENU_HEIGHT});
        SetBorderThinkess(1);
        SetBorderColor(roa::WHITE);
        SetLabel("file");
    
        auto fileDropDown = std::make_unique<roa::Outliner<int *>>(desktop->GetUI());
        fileDropDown->SetSize({70, 40});
        fileDropDown->SetRecordIconStartPos({5, 3});
        fileDropDown->SetRecordIconSize({14, 14});
        fileDropDown->SetBGColor(desktop->BGColor);
        fileDropDown->SetRecordButtonMode(roa::Button::Mode::CAPTURE_MODE);

        addSaveFileDropdownRecord(desktop, editor, fileDropDown.get());
        addLoadFileDropdownRecord(desktop, editor, fileDropDown.get());

        SetDropDownWidget(std::move(fileDropDown));
    } 
private:
    void addSaveFileDropdownRecord(roa::Desktop *desktop, roa::EditorWidget *editor, roa::Outliner<int *> *fileDropDown) {
        assert(desktop);
        assert(editor);
        assert(fileDropDown);

        fileDropDown->AddRecord(nullptr, "Save", [desktop, editor](){
            auto saveWindow = std::make_unique<roa::TextWindow>(desktop->GetUI());
            saveWindow->SetPos({(desktop->GetSize().x - saveWindow->GetSize().x) / 2, (desktop->GetSize().y - saveWindow->GetSize().y) / 2});
            saveWindow->SetTitle("Saving scene to file");
            auto *saveWindowPtr = saveWindow.get();
            saveWindow->SetInputFieldOnEnterAction([saveWindowPtr, editor](const std::string &text){
                try {
                    namespace fs = std::filesystem;
                    if (text.empty()) {
                        saveWindowPtr->DisplayMessage("Provide filename", {200, 120, 0, 255});
                    } else if (editor->SerializeScene(text)) {
                        saveWindowPtr->DisplayMessage("Scene is saved to `" + text + "`", {0, 200, 0, 255});
                    } else {
                        saveWindowPtr->DisplayMessage("`" + text + "` serialization failed", {200, 0, 0, 255});
                    }
                } catch (...) {
                    saveWindowPtr->DisplayMessage("Error", {200, 0, 0, 255});
                }
            });
            desktop->AddWidget(std::move(saveWindow));
        }, nullptr, static_cast<roa::UI *>(desktop->GetUI())->GetTexturePack().fileSaveIconPath);
    }   

    void addLoadFileDropdownRecord(roa::Desktop *desktop, roa::EditorWidget *editor, roa::Outliner<int *> *fileDropDown) {
        fileDropDown->AddRecord(nullptr, "Load", [desktop, editor](){
        auto loadWindow = std::make_unique<roa::TextWindow>(desktop->GetUI());
        loadWindow->SetPos({(desktop->GetSize().x - loadWindow->GetSize().x) / 2, (desktop->GetSize().y - loadWindow->GetSize().y) / 2});
        loadWindow->SetTitle("Loading scene from file");
        auto *loadWindowPtr = loadWindow.get();
        loadWindow->SetInputFieldOnEnterAction(
            [loadWindowPtr, editor](const std::string &text) {
                try {
                    namespace fs = std::filesystem;
                    if (text.empty()) {
                        loadWindowPtr->DisplayMessage("Provide filename", {200, 120, 0, 255});
                    } else if (fs::exists(fs::path(text))) {
                        if (editor->DeserializeScene(text)) {
                            loadWindowPtr->DisplayMessage("Scene loaded successfully!", {0, 200, 0, 255});
                        } else {
                            loadWindowPtr->DisplayMessage("Scene loadeding failed!", {0, 200, 0, 255});
                        }
                    } else {
                        loadWindowPtr->DisplayMessage("Scene file is not found", {200, 0, 0, 255});
                    }
                } catch (...) {
                    loadWindowPtr->DisplayMessage("Error", {200, 0, 0, 255});
                }
            });
            desktop->AddWidget(std::move(loadWindow));
        }, nullptr, static_cast<roa::UI *>(desktop->GetUI())->GetTexturePack().fileLoadIconPath);
    }
};

}

void CreateDesktopMainMenu(roa::Desktop *desktop, roa::EditorWidget *editor) {
    assert(desktop);
    assert(editor);

    auto fileItem = std::make_unique<roa::FileItem>(desktop, editor);
    desktop->AddMaiMenuItem(std::move(fileItem));


    auto pluginItem = std::make_unique<roa::DropDownMenu>(desktop->GetUI());
    pluginItem->SetBorderThinkess(1);
    pluginItem->SetBorderColor(roa::WHITE);

    pluginItem->SetSize({70, desktop->MAIN_MENU_HEIGHT});
    pluginItem->SetLabel("plugins");
    auto pluginDropDown = std::make_unique<roa::Outliner<int *>>(desktop->GetUI());
    pluginDropDown->SetSize({100, 20});
    pluginDropDown->SetRecordIconStartPos({5, 3});
    pluginDropDown->SetRecordIconSize({14, 14});
    pluginDropDown->SetBGColor(desktop->BGColor);
    pluginDropDown->SetRecordButtonMode(roa::Button::Mode::CAPTURE_MODE);
    
    pluginDropDown->AddRecord(nullptr, "Add pp plugin", [](){
        std::cout << "open plugin add widget!\n";
    }, nullptr, static_cast<roa::UI *>(desktop->GetUI())->GetTexturePack().addIconPath);

    pluginItem->SetDropDownWidget(std::move(pluginDropDown));
    desktop->AddMaiMenuItem(std::move(pluginItem));
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
    auto editor = std::make_unique<roa::EditorWidget>(&ui);
    editor->SetSize(desktop->GetSize());
    
// MAIN MENU
    CreateDesktopMainMenu(desktop, editor.get());
    desktop->AddWidget(std::move(editor)); 

// PP

    auto ppCanvas = std::make_unique<roa::PPCanvasWidget>(&ui, ppPlugins);
    desktop->SetModal(std::move(ppCanvas));
    ui.AddHotkey({dr4::KeyCode::KEYCODE_D, dr4::KeyMode::KEYMOD_CTRL}, [desktop](){desktop->SwitchModalActiveFlag(); });

// MAIN LOOP
    ui.Run(0.01);

// CLEANUP
    delete window;
}

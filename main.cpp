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
    roa::EditorWidget *editorPtr = editor.get();
    editor->SetSize(desktop->GetSize());
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
    
    saveDropDown->AddRecord(nullptr, "Save", [desktop, editorPtr](){
        auto saveWindow = std::make_unique<roa::TextWindow>(desktop->GetUI());
        saveWindow->SetPos({(desktop->GetSize().x - saveWindow->GetSize().x) / 2, (desktop->GetSize().y - saveWindow->GetSize().y) / 2});
        saveWindow->SetTitle("Saving scene to file");
        auto *saveWindowPtr = saveWindow.get();
        saveWindow->SetInputFieldOnEnterAction([saveWindowPtr, editorPtr](const std::string &text){
            try {
                namespace fs = std::filesystem;
                if (text.empty()) {
                    saveWindowPtr->DisplayMessage("Provide filename", {200, 120, 0, 255});
                } else if (editorPtr->SerializeScene(text)) {
                    saveWindowPtr->DisplayMessage("Scene is saved to `" + text + "`", {0, 200, 0, 255});
                } else {
                    saveWindowPtr->DisplayMessage("`" + text + "` serialization failed", {200, 0, 0, 255});
                }
            } catch (...) {
                saveWindowPtr->DisplayMessage("Error", {200, 0, 0, 255});
            }
        });
        desktop->AddWidget(std::move(saveWindow));
    }, nullptr, ui.GetTexturePack().fileSaveIconPath);

    saveDropDown->AddRecord(nullptr, "Load", [desktop, editorPtr](){
        auto loadWindow = std::make_unique<roa::TextWindow>(desktop->GetUI());
        loadWindow->SetPos({(desktop->GetSize().x - loadWindow->GetSize().x) / 2, (desktop->GetSize().y - loadWindow->GetSize().y) / 2});
        loadWindow->SetTitle("Saving scene to file");
        auto *loadWindowPtr = loadWindow.get();
        loadWindow->SetInputFieldOnEnterAction([loadWindowPtr, editorPtr](const std::string &text){
            try {
                namespace fs = std::filesystem;
                if (text.empty()) {
                    loadWindowPtr->DisplayMessage("Provide filename", {200, 120, 0, 255});
                } else if (fs::exists(fs::path(text))) {
                    if (editorPtr->DeserializeScene(text)) {
                        loadWindowPtr->DisplayMessage("Scene is loaded from `" + text + "` successfully!", {0, 200, 0, 255});
                    } else {
                        loadWindowPtr->DisplayMessage("Scene loading from `" + text + "` failed!", {0, 200, 0, 255});
                    }
                } else {
                    loadWindowPtr->DisplayMessage("Scene file is not found", {200, 0, 0, 255});
                }
            } catch (...) {
                loadWindowPtr->DisplayMessage("Error", {200, 0, 0, 255});
            }
        });
        desktop->AddWidget(std::move(loadWindow));
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
// TEST

// MAIN LOOP
    ui.Run(0.01);

// CLEANUP
    delete window;
}

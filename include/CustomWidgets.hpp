#pragma once
#include <unordered_map>

#include "cum/manager.hpp"
#include "cum/plugin.hpp"

#include "BasicWidgets/Desktop.hpp"

namespace roa
{

class OpticDesktop final : public Desktop {
    cum::Manager *pluginManager;
    std::unordered_map<std::string, cum::PPToolPlugin*> PPTable;

public:
    OpticDesktop(hui::UI *ui, cum::Manager *pluginManager_) : Desktop(ui), pluginManager(pluginManager_) {
        assert(ui);
        assert(pluginManager);

        auto editor = std::make_unique<roa::EditorWidget>(ui);
        editor->SetSize(GetSize());
    
        auto fileItem = std::make_unique<FileItem>(this, editor.get());
        AddMaiMenuItem(std::move(fileItem));

        auto pluginItem = std::make_unique<PluginItem>(this, editor.get());
        AddMaiMenuItem(std::move(pluginItem));

        AddWidget(std::move(editor));
    }
    ~OpticDesktop() = default;

    bool ExistsPPPlugin(const std::string &path) {
        return (PPTable.find(path) != PPTable.end());
    }

    bool LoadPPPlugin(const std::string &path) {
        if (ExistsPPPlugin(path)) return false;
        auto plugin = dynamic_cast<cum::PPToolPlugin*>(pluginManager->LoadFromFile(path));
        assert(plugin);
        PPTable[path] = plugin;
    }
};

class FileItem final : public DropDownMenu {
public:
    FileItem(Desktop *desktop, EditorWidget *editor): DropDownMenu(desktop->GetUI()) {
        SetSize({50, desktop->MAIN_MENU_HEIGHT});
        SetBorderThinkess(1);
        SetBorderColor(WHITE);
        SetLabel("file");
    
        auto fileDropDown = std::make_unique<Outliner<int *>>(desktop->GetUI());
        fileDropDown->SetSize({70, 40});
        fileDropDown->SetRecordIconStartPos({5, 3});
        fileDropDown->SetRecordIconSize({14, 14});
        fileDropDown->SetBGColor(desktop->BGColor);
        fileDropDown->SetRecordButtonMode(Button::Mode::CAPTURE_MODE);

        addSaveFileDropdownRecord(desktop, editor, fileDropDown.get());
        addLoadFileDropdownRecord(desktop, editor, fileDropDown.get());

        SetDropDownWidget(std::move(fileDropDown));
    } 
private:
    void addSaveFileDropdownRecord(Desktop *desktop, EditorWidget *editor, Outliner<int *> *fileDropDown) {
        assert(desktop);
        assert(editor);
        assert(fileDropDown);

        fileDropDown->AddRecord(nullptr, "Save", [desktop, editor](){
            auto saveWindow = std::make_unique<TextWindow>(desktop->GetUI());
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
        }, nullptr, static_cast<UI *>(desktop->GetUI())->GetTexturePack().fileSaveIconPath);
    }   

    void addLoadFileDropdownRecord(Desktop *desktop, EditorWidget *editor, Outliner<int *> *fileDropDown) {
        fileDropDown->AddRecord(nullptr, "Load", [desktop, editor](){
        auto loadWindow = std::make_unique<TextWindow>(desktop->GetUI());
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
        }, nullptr, static_cast<UI *>(desktop->GetUI())->GetTexturePack().fileLoadIconPath);
    }
};

class PluginItem final : public DropDownMenu {
public:
    PluginItem(OpticDesktop *desktop, EditorWidget *editor): DropDownMenu(desktop->GetUI()) {
        SetBorderThinkess(1);
        SetBorderColor(WHITE);

        SetSize({70, desktop->MAIN_MENU_HEIGHT});
        SetLabel("plugins");
        auto pluginDropDown = std::make_unique<Outliner<int *>>(desktop->GetUI());
        pluginDropDown->SetSize({100, 20});
        pluginDropDown->SetRecordIconStartPos({5, 3});
        pluginDropDown->SetRecordIconSize({14, 14});
        pluginDropDown->SetBGColor(desktop->BGColor);
        pluginDropDown->SetRecordButtonMode(Button::Mode::CAPTURE_MODE);

        pluginDropDown->AddRecord(nullptr, "Add pp plugin", [desktop, editor](){
            auto addWindow = std::make_unique<TextWindow>(desktop->GetUI());
            addWindow->SetPos({(desktop->GetSize().x - addWindow->GetSize().x) / 2, (desktop->GetSize().y - addWindow->GetSize().y) / 2});
            addWindow->SetTitle("Saving scene to file");
            auto *addWindowPtr = addWindow.get();
            addWindow->SetInputFieldOnEnterAction([addWindowPtr, desktop](const std::string &text){
                try {
                    namespace fs = std::filesystem;
                    if (text.empty()) {
                        addWindowPtr->DisplayMessage("Provide pp plugin filename", {200, 120, 0, 255});
                    } else if (fs::exists(fs::path(text))) {
                        if (desktop->ExistsPPPlugin(text)) {
                            addWindowPtr->DisplayMessage("Plugin already exists", {200, 0, 0, 255});
                        } else if (desktop->LoadPPPlugin(text)){
                            addWindowPtr->DisplayMessage("Plugin loaded successfully!", {0, 200, 0, 255});
                        } else {
                            addWindowPtr->DisplayMessage("Plugin loading failed", {200, 0, 0, 255});
                        }
                    } else {
                        addWindowPtr->DisplayMessage("file `" + text + "` was not found", {200, 0, 0, 255});
                    }
                } catch (...) {
                    addWindowPtr->DisplayMessage("Error", {200, 0, 0, 255});
                }
            });
            desktop->AddWidget(std::move(addWindow));
        }, nullptr, static_cast<UI *>(desktop->GetUI())->GetTexturePack().addIconPath);

        SetDropDownWidget(std::move(pluginDropDown));
    } 
};


} // namespace roa
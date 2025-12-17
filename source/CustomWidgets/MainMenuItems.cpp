
#include <cassert>
#include <filesystem>
#include <memory>

#include "BasicWidgets/TextWindow.hpp"
#include "CompositeWidgets/EditorWidget.hpp"
#include "CustomWidgets/OpticDesktop.hpp"
#include "CustomWidgets/MainMenuItems.hpp"

namespace roa
{

FileItem::FileItem(Desktop *desktop, EditorWidget *editor)
    : DropDownMenu(desktop->GetUI())
{
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

void FileItem::addSaveFileDropdownRecord(Desktop *desktop,
                                         EditorWidget *editor,
                                         Outliner<int *> *fileDropDown)
{
    assert(desktop);
    assert(editor);
    assert(fileDropDown);

    fileDropDown->AddRecord(
        nullptr, "Save",
        [desktop, editor]() {
            auto saveWindow = std::make_unique<TextWindow>(desktop->GetUI());
            saveWindow->SetPos({
                (desktop->GetSize().x - saveWindow->GetSize().x) / 2,
                (desktop->GetSize().y - saveWindow->GetSize().y) / 2
            });
            saveWindow->SetTitle("Saving scene to file");
            auto *ptr = saveWindow.get();

            saveWindow->SetInputFieldOnEnterAction(
                [ptr, editor](const std::string &text) {
                    try {
                        if (text.empty()) {
                            ptr->DisplayMessage("Provide filename", {200,120,0,255});
                        } else if (editor->SerializeScene(text)) {
                            ptr->DisplayMessage(
                                "Scene is saved to `" + text + "`",
                                {0,200,0,255}
                            );
                        } else {
                            ptr->DisplayMessage(
                                "`" + text + "` serialization failed",
                                {200,0,0,255}
                            );
                        }
                    } catch (...) {
                        ptr->DisplayMessage("Error", {200,0,0,255});
                    }
                }
            );
            desktop->AddWidget(std::move(saveWindow));
        },
        nullptr,
        static_cast<UI *>(desktop->GetUI())
            ->GetTexturePack().fileSaveIconPath
    );
}

void FileItem::addLoadFileDropdownRecord(Desktop *desktop,
                                         EditorWidget *editor,
                                         Outliner<int *> *fileDropDown)
{
    fileDropDown->AddRecord(
        nullptr, "Load",
        [desktop, editor]() {
            auto loadWindow = std::make_unique<TextWindow>(desktop->GetUI());
            loadWindow->SetPos({
                (desktop->GetSize().x - loadWindow->GetSize().x) / 2,
                (desktop->GetSize().y - loadWindow->GetSize().y) / 2
            });
            loadWindow->SetTitle("Loading scene from file");
            auto *ptr = loadWindow.get();

            loadWindow->SetInputFieldOnEnterAction(
                [ptr, editor](const std::string &text) {
                    try {
                        namespace fs = std::filesystem;
                        if (text.empty()) {
                            ptr->DisplayMessage("Provide filename", {200,120,0,255});
                        } else if (fs::exists(fs::path(text))) {
                            if (editor->DeserializeScene(text)) {
                                ptr->DisplayMessage(
                                    "Scene loaded successfully!",
                                    {0,200,0,255}
                                );
                            } else {
                                ptr->DisplayMessage(
                                    "Scene loading failed!",
                                    {200,0,0,255}
                                );
                            }
                        } else {
                            ptr->DisplayMessage(
                                "Scene file is not found",
                                {200,0,0,255}
                            );
                        }
                    } catch (...) {
                        ptr->DisplayMessage("Error", {200,0,0,255});
                    }
                }
            );
            desktop->AddWidget(std::move(loadWindow));
        },
        nullptr,
        static_cast<UI *>(desktop->GetUI())
            ->GetTexturePack().fileLoadIconPath
    );
}

PluginItem::PluginItem(OpticDesktop *desktop, EditorWidget *editor)
    : DropDownMenu(desktop->GetUI())
{
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

    pluginDropDown->AddRecord(
        nullptr, "Add pp plugin",
        [desktop, editor]() {
            auto addWindow = std::make_unique<TextWindow>(desktop->GetUI());
            addWindow->SetPos({
                (desktop->GetSize().x - addWindow->GetSize().x) / 2,
                (desktop->GetSize().y - addWindow->GetSize().y) / 2
            });
            addWindow->SetTitle("Saving scene to file");
            auto *ptr = addWindow.get();

            addWindow->SetInputFieldOnEnterAction(
                [ptr, desktop](const std::string &text) {
                    try {
                        namespace fs = std::filesystem;
                        if (text.empty()) {
                            ptr->DisplayMessage(
                                "Provide pp plugin filename",
                                {200,120,0,255}
                            );
                        } else if (fs::exists(fs::path(text))) {
                            if (desktop->ExistsPPPlugin(text)) {
                                ptr->DisplayMessage(
                                    "Plugin already exists",
                                    {200,0,0,255}
                                );
                            } else if (desktop->LoadPPPlugin(text)) {
                                ptr->DisplayMessage(
                                    "Plugin loaded successfully!",
                                    {0,200,0,255}
                                );
                            } else {
                                ptr->DisplayMessage(
                                    "Plugin loading failed",
                                    {200,0,0,255}
                                );
                            }
                        } else {
                            ptr->DisplayMessage(
                                "file `" + text + "` was not found",
                                {200,0,0,255}
                            );
                        }
                    } catch (...) {
                        ptr->DisplayMessage("Error", {200,0,0,255});
                    }
                }
            );
            desktop->AddWidget(std::move(addWindow));
        },
        nullptr,
        static_cast<UI *>(desktop->GetUI())
            ->GetTexturePack().addIconPath
    );

    SetDropDownWidget(std::move(pluginDropDown));
}

} // namespace roa

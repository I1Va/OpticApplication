#include <cassert>
#include <filesystem>
#include <memory>

#include "BasicWidgets/TextWindow.hpp"
#include "CompositeWidgets/EditorWidget.hpp"
#include "CustomWidgets/OpticDesktop.hpp"
#include "CustomWidgets/MainMenuItems.hpp"

namespace roa
{

inline dr4::Vec2f CalculateCenteredPosition(Desktop *desktop,
                                            hui::Widget *widget)
{
    return {
        (desktop->GetSize().x - widget->GetSize().x) / 2,
        (desktop->GetSize().y - widget->GetSize().y) / 2
    };
}

template <typename OnEnterCallback>
void OpenCenteredTextWindow(Desktop *desktop,
                            const char *windowTitle,
                            OnEnterCallback &&onEnterCallback)
{
    auto textWindow = std::make_unique<TextWindow>(desktop->GetUI());
    textWindow->SetPos(
        CalculateCenteredPosition(desktop, textWindow.get())
    );
    textWindow->SetTitle(windowTitle);

    TextWindow *windowPtr = textWindow.get();
    textWindow->SetInputFieldOnEnterAction(
        [windowPtr,
         callback = std::forward<OnEnterCallback>(onEnterCallback)]
        (const std::string &inputText)
        {
            try {
                callback(windowPtr, inputText);
            } catch (...) {
                windowPtr->DisplayMessage("Error", {200,0,0,255});
            }
        }
    );

    desktop->AddWidget(std::move(textWindow));
}

template <typename RecordAction>
void AddDropdownRecord(Outliner<int *> *dropDownMenu,
                       const char *recordLabel,
                       const std::string &iconPath,
                       RecordAction &&action)
{
    dropDownMenu->AddRecord(
        nullptr,
        recordLabel,
        std::forward<RecordAction>(action),
        nullptr,
        iconPath
    );
}

FileItem::FileItem(Desktop *desktop, EditorWidget *editor)
    : DropDownMenu(desktop->GetUI())
{
    SetSize({50, desktop->MAIN_MENU_HEIGHT});
    SetBorderThinkess(1);
    SetBorderColor(WHITE);
    SetLabelFontSize(static_cast<UI *>(GetUI())->GetTexturePack().fontSize);
    SetLabel("file");
    
    auto fileDropDownMenu =
        std::make_unique<Outliner<int *>>(desktop->GetUI());

    fileDropDownMenu->SetSize({70, 40});
    fileDropDownMenu->SetRecordIconStartPos({5, 3});
    fileDropDownMenu->SetRecordIconSize({14, 14});
    fileDropDownMenu->SetBGColor(desktop->BGColor);
    fileDropDownMenu->SetRecordLabelFontSize(static_cast<UI *>(GetUI())->GetTexturePack().fontSize);
    fileDropDownMenu->SetRecordButtonMode(
        Button::Mode::CAPTURE_MODE
    );

    AddDropdownRecord(
        fileDropDownMenu.get(),
        "Save",
        static_cast<UI *>(desktop->GetUI())
            ->GetTexturePack().fileSaveIconPath,
        [desktop, editor]() {
            OpenCenteredTextWindow(
                desktop,
                "Saving scene to file",
                [editor](TextWindow *window,
                         const std::string &filename)
                {
                    if (filename.empty()) {
                        window->DisplayMessage(
                            "Provide filename",
                            {200,120,0,255}
                        );
                    } else if (editor->SerializeScene(filename)) {
                        window->DisplayMessage(
                            "Scene is saved",
                            {0,200,0,255}
                        );
                    } else {
                        window->DisplayMessage(
                            "serialization failed",
                            {200,0,0,255}
                        );
                    }
                }
            );
        }
    );

    AddDropdownRecord(
        fileDropDownMenu.get(),
        "Load",
        static_cast<UI *>(desktop->GetUI())
            ->GetTexturePack().fileLoadIconPath,
        [desktop, editor]() {
            OpenCenteredTextWindow(
                desktop,
                "Loading scene from file",
                [editor](TextWindow *window,
                         const std::string &filename)
                {
                    namespace fs = std::filesystem;

                    if (filename.empty()) {
                        window->DisplayMessage(
                            "Provide filename",
                            {200,120,0,255}
                        );
                    } else if (!fs::exists(filename)) {
                        window->DisplayMessage(
                            "Scene file is not found",
                            {200,0,0,255}
                        );
                    } else if (editor->DeserializeScene(filename)) {
                        window->DisplayMessage(
                            "Scene loaded successfully!",
                            {0,200,0,255}
                        );
                    } else {
                        window->DisplayMessage(
                            "Scene loading failed!",
                            {200,0,0,255}
                        );
                    }
                }
            );
        }
    );

    SetDropDownWidget(std::move(fileDropDownMenu));
}

PluginItem::PluginItem(OpticDesktop *desktop, EditorWidget *)
    : DropDownMenu(desktop->GetUI())
{
    SetBorderThinkess(1);
    SetBorderColor(WHITE);
    SetSize({70, desktop->MAIN_MENU_HEIGHT});
    SetLabel("plugins");
    SetLabelFontSize(static_cast<UI *>(GetUI())->GetTexturePack().fontSize);

    auto pluginDropDownMenu =
        std::make_unique<Outliner<int *>>(desktop->GetUI());

    pluginDropDownMenu->SetSize({150, 20});
    pluginDropDownMenu->SetRecordIconStartPos({5, 3});
    pluginDropDownMenu->SetRecordIconSize({14, 14});
    pluginDropDownMenu->SetBGColor(desktop->BGColor);
    pluginDropDownMenu->SetRecordButtonMode(
        Button::Mode::CAPTURE_MODE
    );

    AddDropdownRecord(
        pluginDropDownMenu.get(),
        "Add pp plugin",
        static_cast<UI *>(desktop->GetUI())
            ->GetTexturePack().addIconPath,
        [desktop]() {
            OpenCenteredTextWindow(
                desktop,
                "Add pp plugin",
                [desktop](TextWindow *window,
                           const std::string &pluginPath)
                {
                    namespace fs = std::filesystem;

                    if (pluginPath.empty()) {
                        window->DisplayMessage(
                            "Provide pp plugin filename",
                            {200,120,0,255}
                        );
                    } else if (!fs::exists(pluginPath)) {
                        window->DisplayMessage(
                            "file was not found",
                            {200,0,0,255}
                        );
                    } else if (desktop->ExistsPPPlugin(pluginPath)) {
                        window->DisplayMessage(
                            "Plugin already exists",
                            {200,0,0,255}
                        );
                    } else if (desktop->LoadPPPlugin(pluginPath)) {
                        window->DisplayMessage(
                            "Plugin loaded successfully!",
                            {0,200,0,255}
                        );
                    } else {
                        window->DisplayMessage(
                            "Plugin loading failed",
                            {200,0,0,255}
                        );
                    }
                }
            );
        }
    );

    SetDropDownWidget(std::move(pluginDropDownMenu));
}

} // namespace roa

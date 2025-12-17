#include <cassert>
#include <memory>

#include "CompositeWidgets/EditorWidget.hpp"
#include "CustomWidgets/MainMenuItems.hpp"
#include "CustomWidgets/OpticDesktop.hpp"

namespace roa
{

OpticDesktop::OpticDesktop(hui::UI *ui, cum::Manager *pluginManager_)
    : Desktop(ui), pluginManager(pluginManager_)
{
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

bool OpticDesktop::ExistsPPPlugin(const std::string &path)
{
    return PPTable.find(path) != PPTable.end();
}

bool OpticDesktop::LoadPPPlugin(const std::string &path)
{
    if (ExistsPPPlugin(path))
        return false;

    auto plugin = dynamic_cast<cum::PPToolPlugin*>(
        pluginManager->LoadFromFile(path)
    );
    assert(plugin);

    PPTable[path] = plugin;
    return true;
}

} // namespace roa

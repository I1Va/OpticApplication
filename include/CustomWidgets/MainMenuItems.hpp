#pragma once

#include "CompositeWidgets/DropDownMenu.hpp"
#include "CustomWidgets/OpticDesktop.hpp"

namespace roa
{

class FileItem final : public DropDownMenu {
public:
    FileItem(Desktop *desktop, EditorWidget *editor);

private:
    void addSaveFileDropdownRecord(Desktop *desktop,
                                   EditorWidget *editor,
                                   Outliner<int *> *fileDropDown);

    void addLoadFileDropdownRecord(Desktop *desktop,
                                   EditorWidget *editor,
                                   Outliner<int *> *fileDropDown);
};

class PluginItem final : public DropDownMenu {
public:
    PluginItem(OpticDesktop *desktop, EditorWidget *editor);
};

} // namespace roa

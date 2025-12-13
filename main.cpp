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

// core container definitions must come first so other widgets see them
#include "BasicWidgets/Buttons.hpp"
#include "BasicWidgets/ScrollBar.hpp"
#include "BasicWidgets/TextWidgets.hpp"
#include "BasicWidgets/Desktop.hpp"
#include "BasicWidgets/Window.hpp"

// #include "RecordsPanel.hpp"
// #include "DropDownMenu.hpp"
// #include "PropertiesPanel.hpp"

// #include "PPWidgets.hpp"
// #include "EditorWidget.hpp"

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
    desktop->SetSize({window->GetSize().x, window->GetSize().y});
    ui.SetRoot(desktop);


    float padding = 3; 
    auto scrollBar = std::make_unique<roa::VerticalScrollBar>(&ui);
    scrollBar->SetSize({10, 100});
    scrollBar->SetPos({100, 100});

    auto textInputWidget = std::make_unique<roa::TextInputWidget>(&ui);
    textInputWidget->SetPos(scrollBar->GetPos() + dr4::Vec2f(scrollBar->GetSize().x + padding, 0));
    textInputWidget->SetSize({200, 200});

    auto panelWindow = std::make_unique<roa::Window>(&ui);
    panelWindow->SetSize({50, 200});
    panelWindow->SetPos(textInputWidget->GetPos() + dr4::Vec2f(textInputWidget->GetSize().x + padding, 0));

    auto button = std::make_unique<roa::RoundedBlenderButton>(&ui);
    button->SetSize({200, 200});
    button->SetPos(textInputWidget->GetPos() + dr4::Vec2f(textInputWidget->GetSize().x + padding, 0));

    desktop->AddWidget(std::move(scrollBar));
    desktop->AddWidget(std::move(textInputWidget));
    desktop->AddWidget(std::move(panelWindow));
    desktop->AddWidget(std::move(button));

    ui.Run(0.01);

// CLEANUP
    delete window;
}

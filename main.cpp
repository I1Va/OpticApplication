#include <iostream>
#include <functional>
#include <cassert>
#include <list>
#include <cmath>
#include "BasicWidgets.hpp"

const char FONT_PATH[] = "assets/RobotoFont.ttf";


namespace roa
{

class UI : public hui::UI {
    dr4::Font *defaultFont = nullptr;

public:
    UI(dr4::Window *window, const std::string defaultFontPath): hui::UI(window) {
        assert(window);

        try {
            defaultFont = window->CreateFont();        
            if (defaultFont)
                defaultFont->LoadFromFile(defaultFontPath);
        } catch (std::runtime_error &e) {
            delete defaultFont;
            std::cerr << "roa::UI create defaultFont failed : " << e.what() << "\n";
            throw e;
        }
    }
    ~UI() { if (defaultFont) delete defaultFont; }

    dr4::Font *GetDefaultFont() { return defaultFont; }
};

class Button : public hui::Widget {
    constexpr static int DEFAULT_FONT_SIZE = 18;

    std::string labelContent;
    int fontSize;
    dr4::Text *label = nullptr;

    std::function<void()> onClickAction = nullptr;

    bool pressed = false;
    bool needRelayout = true;

public:
    Button(hui::UI *ui, const std::string &lbl, const int fontSize=DEFAULT_FONT_SIZE, std::function<void()> onClickAction=nullptr)
    : hui::Widget(ui), labelContent(lbl), fontSize(fontSize), onClickAction(onClickAction) 
    {
        assert(ui);
    
        label = GetUI()->GetWindow()->CreateText();
        label->SetFont(static_cast<UI *>(GetUI())->GetDefaultFont());
        label->SetText(labelContent);
        label->SetFontSize(fontSize);

        layout();
    }

    ~Button() { delete label; }

    void SetOnClickAction(std::function<void()> action) { onClickAction = action; }
    
    bool IsPressed() const { return pressed; }
    void SetPressed(bool flag) { pressed = flag; ForceRedraw(); }
    
    hui::EventResult OnMouseDown(hui::MouseButtonEvent &event) override { 
        if (event.pressed == false                     ||
            event.button != dr4::MouseButtonType::LEFT || 
              !GetRect().Contains(event.pos)) 
            return hui::EventResult::UNHANDLED;
    
        GetUI()->ReportFocus(this);
        if (onClickAction) onClickAction();
    
        ForceRedraw();

        return hui::EventResult::HANDLED;
    }

private:

    hui::EventResult OnIdle(hui::IdleEvent &) override {
        if (needRelayout) layout();

        return hui::EventResult::UNHANDLED;
    }

    void Redraw() const override { 
        dr4::Color color = (pressed ? dr4::Color{0, 0, 255, 255} : dr4::Color{255, 255, 255, 255});
        GetTexture().Clear(color); 
        label->DrawOn(GetTexture());
    }

    void layout() {
        // label->SetVAlign(dr4::Text::VAlign::MIDDLE);
        SetSize({label->GetBounds().x, label->GetBounds().y});
        needRelayout = false;
    }

};

class DropDownMenu : public ListContainer<Button *> {
    bool visible = false;
public:
    DropDownMenu(hui::UI *ui): ListContainer(ui) {}

    bool IsVisible() const { return visible; }

    void Redraw() const override { 
        if (!visible) { // full transparent
            GetTexture().Clear({0, 0, 0, 0});
            return;
        }

        GetTexture().Clear({0, 0, 0, 255});
        for (Button *child : children) {
            child->GetFreshTexture().DrawOn(GetTexture());
        }
    }

    void Hide() {
        visible = false;
        ForceRedraw();
    }

    // anchor is the widget that dropdown menu is positioned relative to.
    void Show(dr4::Vec2f showPos) {
        visible = true;
        
        SetPos(showPos);

        // layoutChildren();
        
        if (GetParent()) static_cast<ZContainer<Widget*>*>(GetParent())->BringToFront(this);

        ForceRedraw();
    }
};

class MenuBar : public LinContainer<Button *> {
    bool needRelayout = true;

    Button *currentlyActiveButton = nullptr;
    DropDownMenu *currentlyActiveMenu = nullptr;

public:
    MenuBar(UI *ui): LinContainer(ui) {}

    hui::EventResult OnIdle(hui::IdleEvent &) override {
        if (needRelayout) layout();

        return hui::EventResult::UNHANDLED;
    }

    void Redraw() const override {    
        GetTexture().Clear({255, 0, 0, 255});
        for (Button *button : children) {
            button->DrawOn(GetTexture());
        }
    }

    void addMenuItem(const std::string &label, DropDownMenu *dropDownMenu) {
        assert(dropDownMenu);
    
        Button *newButton = new Button(GetUI(), label);

        newButton->SetOnClickAction
        (
            [this, newButton, dropDownMenu]() 
            {       
                if (newButton->IsPressed()) {
                    newButton->SetPressed(false);
                    dropDownMenu->Hide();
                    currentlyActiveButton = nullptr;
                    return;
                }
            
                if (currentlyActiveButton) {
                    currentlyActiveButton->SetPressed(false);
                    currentlyActiveMenu->Hide();
                }

                newButton->SetPressed(true);
                dropDownMenu->Show(newButton->GetPos() + dr4::Vec2f(0, newButton->GetSize().y));
                currentlyActiveButton = newButton;
                currentlyActiveMenu  = dropDownMenu;
            }
        );
    
        addWidget(newButton);

        needRelayout = true;
    }
private:
    void layout() {
        float curBtnX = 0;
        float maxButtonHeight = 0;
        for (Button *button : children) {
            button->SetPos({curBtnX, 0});
            curBtnX += button->GetSize().x;
            maxButtonHeight = std::fmax(maxButtonHeight, button->GetSize().y);
        }
        if (curBtnX > 0 && maxButtonHeight > 0) {
            SetSize({curBtnX, maxButtonHeight});
            ForceRedraw();
        }

        needRelayout = false;
    }
};


}

void runUI(roa::UI &ui) {
    double frameDelaySecs_ = 0.032;
    while (ui.GetWindow()->IsOpen()) {
        while (true) {
            auto evt = ui.GetWindow()->PollEvent();
            if (!evt.has_value()) break; 

            if (evt->type == dr4::Event::Type::QUIT ||
            (evt->type == dr4::Event::Type::KEY_DOWN && evt->key.sym == dr4::KeyCode::KEYCODE_ESCAPE)) {
                ui.GetWindow()->Close();
                break;
            }
            ui.ProcessEvent(evt.value());
        }

        double frameStartSecs = ui.GetWindow()->GetTime();
        hui::IdleEvent idleEvent;
        idleEvent.absTime = frameStartSecs;
        idleEvent.deltaTime = frameDelaySecs_;
        ui.OnIdle(idleEvent);

        ui.GetWindow()->Clear({50,50,50,255});
        if (ui.GetTexture()) ui.GetWindow()->Draw(*ui.GetTexture());
        ui.GetWindow()->Display();

        // ui.GetWindow()->Sleep();
    }
}

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        std::cerr << "Expected one argument: dr4 backend path\n";
        return 1;
    }
    const char *dr4BackendPath = argv[1];

    cum::Manager pluginManager;
    pluginManager.LoadFromFile(dr4BackendPath);
    auto *dr4Backend = pluginManager.GetAnyOfType<cum::DR4BackendPlugin>();
    assert(dr4Backend);

    dr4::Window *window = dr4Backend->CreateWindow(); assert(window);
    window->Open();
    window->StartTextInput();
    window->SetSize({800, 600});

    roa::UI ui(window, FONT_PATH);
    roa::MainWindow *mainWindow = new roa::MainWindow(&ui);
    mainWindow->SetSize({window->GetSize().x, window->GetSize().y});
    ui.SetRoot(mainWindow);

    




    roa::DropDownMenu *fileDropDownMenu = new roa::DropDownMenu(&ui);
    roa::DropDownMenu *pluginDropDownMenu = new roa::DropDownMenu(&ui);
    roa::DropDownMenu *testDropDownMenu = new roa::DropDownMenu(&ui);
    mainWindow->addWidget(fileDropDownMenu);
    mainWindow->addWidget(pluginDropDownMenu);
    mainWindow->addWidget(testDropDownMenu);

    roa::MenuBar *menuBar = new roa::MenuBar(&ui);
    menuBar->SetPos({0, 0});
    menuBar->SetSize({100, 30});

    menuBar->addMenuItem("file", fileDropDownMenu);
    menuBar->addMenuItem("plugin", pluginDropDownMenu);
    menuBar->addMenuItem("test", testDropDownMenu);



    mainWindow->addWidget(menuBar);



    
    runUI(ui);
    
    delete window;
}
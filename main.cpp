#include <iostream>
#include <cassert>
#include <list>

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
    bool pressed = true;
    std::string labelContent;
    dr4::Text *label = nullptr;

public:
    Button(hui::UI *ui, const std::string &lbl) : hui::Widget(ui), labelContent(lbl) {
        assert(ui);
    
        label = GetUI()->GetWindow()->CreateText();
        label->SetFont(static_cast<UI *>(GetUI())->GetDefaultFont());
        label->SetText(labelContent);

        layoutText();
    }

    hui::EventResult OnMouseDown(hui::MouseButtonEvent &event) override {
        if (!GetRect().Contains(event.pos)) return hui::EventResult::UNHANDLED;

        GetUI()->ReportFocus(this);
        pressed = !pressed;
        ForceRedraw();

        return hui::EventResult::HANDLED;
    }

private:
    void Redraw() const override { 
        dr4::Color color = (pressed ? dr4::Color{0, 0, 255, 255} : dr4::Color{255, 255, 255, 255});

        GetTexture().Clear(color);   
    }

    void layoutText() {
        label->SetVAlign(dr4::Text::VAlign::MIDDLE);

        float fontSize = GetRect().size.y;
        label->SetFontSize(fontSize);

        float btnX = GetRect().pos.x;
        float btnY = GetRect().pos.y;
        float btnWidth = GetRect().size.x;
        float btnHeight = GetRect().size.y;

        float textWidth = label->GetBounds().x;      
        float textHeight = label->GetFont()->GetAscent(fontSize) + label->GetFont()->GetDescent(fontSize);

        float textX = btnX + (btnWidth - textWidth) / 2.0f;
        float textY = btnY + (btnHeight - textHeight) / 2.0f + label->GetFont()->GetAscent(fontSize);

        label->SetPos(textX, textY);
    }
    
    void OnSizeChanged() { layoutText(); }


};

class DropDownMenu : public ListContainer<Button *> {
    bool visible = true;
public:
    DropDownMenu(hui::UI *ui): ListContainer(ui) {}

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

    void hide() {
        visible = false;
        ForceRedraw();
    }

    // anchor is the widget that dropdown menu is positioned relative to.
    void show(dr4::Vec2f showPos) {
        visible = true;
        
        SetPos(showPos);

        // layoutChildren();
        
        if (GetParent()) static_cast<ZContainer<Widget*>*>(GetParent())->BringToFront(this);

        ForceRedraw();
    }
};

class MenuBar : public LinContainer<Button *> {
public:
    MenuBar(UI *ui): LinContainer(ui) {}

    // void addMenuItem(const std::string &label, DropDownMenu *dropDownMenu, std::function<void()> onAction) {
    //     Button *newButton = new Button(GetUI(), label);
    //     addWidget(newButton);
    //     // creates button and links it to dropDownMenu
    // }
};

class HoverButton : public hui::Widget {
public:
    HoverButton(UI *ui): hui::Widget(ui) {}
    
    hui::EventResult OnIdle(hui::IdleEvent &) override {   
        ForceRedraw();

        return hui::EventResult::HANDLED;
    }
    
    void Redraw() const override { 
        dr4::Color color = {0, 0, 0, 255};

        if (GetUI()->GetHovered() == this) {
            color = {0, 255, 0, 255};
        }        
        GetTexture().Clear(color);
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

    dr4::Window *window = dr4Backend->CreateWindow();
    assert(window);
    window->Open();
    window->StartTextInput();
    window->SetSize({800, 600});

    roa::UI ui(window, FONT_PATH);
    runUI(ui);
    
    delete window;
}
#pragma once

#include <utility>
#include <chrono>
#include <dlfcn.h>
#include <cstdint>
#include <functional>
#include <chrono>
#include <thread>

#include "hui/ui.hpp"
#include "dr4/event.hpp"
#include "dr4/mouse_buttons.hpp"

#include "cum/manager.hpp"
#include "cum/plugin.hpp"
#include "cum/ifc/dr4.hpp"

#include "hui/container.hpp"
#include "hui/event.hpp"

namespace roa
{

template <typename T>
concept WidgetDerived = std::is_base_of_v<hui::Widget, T>;

template <WidgetDerived T>
class LinContainer : public hui::Container {
    std::vector<T *> children; 
public:
    LinContainer(hui::UI *ui): hui::Container(ui) {}
    hui::EventResult PropagateToChildren(hui::Event &event) override {
        for (Widget *child : children) {
            if (event.Apply(*child) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        }
        return hui::EventResult::UNHANDLED;
    }

    void addWidget(Widget *widget) {
        BecomeParentOf(widget);
        children.push_back(widget);
    }

    void BringToFront(Widget* w) override {
        auto it = std::find(children.begin(), children.end(), w);
        if(it != children.end()) {
            children.erase(it);       
            children.push_back(w);   
        }
    }
};

class FocusButton : public hui::Widget {
public:
    FocusButton(hui::UI *ui): hui::Widget(ui) {}
    
    hui::EventResult OnIdle(hui::IdleEvent &) override {    
        ForceRedraw();

        return hui::EventResult::HANDLED;
    }
    
    void Redraw() const override { 
        dr4::Color color = {0, 0, 0, 255};

        if (GetUI()->GetFocused() == this) {
            color = {255, 0, 0, 255};
        }        
        GetTexture().Clear(color);
    }
    
};

}
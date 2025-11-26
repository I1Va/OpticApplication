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
concept WidgetPtr =
    std::is_pointer_v<T> &&
    std::is_base_of_v<hui::Widget, std::remove_pointer_t<T>>;

template <typename T> 
class ZContainer : public hui::Container {
public:
    ZContainer(hui::UI *ui): hui::Container(ui) {}
    virtual ~ZContainer() = default;
    virtual void BringToFront(T widget) = 0;
};

template <WidgetPtr T>
class ListContainer : public ZContainer<T> {
protected:
    std::list<T> children;
public:
    ListContainer(hui::UI *ui): ZContainer<T>(ui) {}
    ~ListContainer() {
        for (T child : children) {
            delete child;
        }
    }

    hui::EventResult PropagateToChildren(hui::Event &event) override {
        for (T child : children) {
            if (event.Apply(*child) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        }
        return hui::EventResult::UNHANDLED;
    }

    void addWidget(T widget) {
        BecomeParentOf(widget);
        children.push_front(widget);
    }
    
    void BringToFront(T widget) override {
        assert(widget);
    
        auto it = std::find(children.begin(), children.end(), widget);
        if(it != children.end()) {
            children.erase(it);
            children.push_front(widget);
        }
    }


};

template <WidgetPtr T>
class LinContainer : public ZContainer<T> {
    std::vector<T> children; 
public:
    LinContainer(hui::UI *ui): ZContainer<T>(ui) {}
    ~LinContainer() {
        for (T child : children) {
            delete child;
        }
    }

    hui::EventResult PropagateToChildren(hui::Event &event) override {
        for (T child : children) {
            if (event.Apply(*child) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        }
        return hui::EventResult::UNHANDLED;
    }

    void addWidget(T widget) {
        BecomeParentOf(widget);
        children.push_back(widget);
    }

    void BringToFront(T widget) override {
        auto it = std::find(children.begin(), children.end(), widget);
        if(it != children.end()) {
            children.erase(it);       
            children.push_back(widget);   
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
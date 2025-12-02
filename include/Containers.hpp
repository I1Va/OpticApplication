#pragma once
#include <concepts>

#include "hui/container.hpp"
#include "Buttons.hpp"

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

    hui::EventResult PropagateToChildren(hui::Event &event) override {
        std::cout << "30";
        for (auto it = children.rbegin(); it != children.rend(); it++) {
            if (event.Apply(**it) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        }
        return hui::EventResult::UNHANDLED;
    }
public:
    ListContainer(hui::UI *ui): ZContainer<T>(ui) {}
    ~ListContainer() {
        for (T child : children) {
            delete child;
        }
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
            children.push_back(widget);
        }
    }
};

template <WidgetPtr T>
class LinContainer : public ZContainer<T> {
protected:
    std::vector<T> children; 
public:
    LinContainer(hui::UI *ui): ZContainer<T>(ui) {}
    ~LinContainer() {
        for (T child : children) {
            delete child;
        }
    }

    hui::EventResult PropagateToChildren(hui::Event &event) override {
        std::cout << "73";
        for (auto it = children.rbegin(); it != children.rend(); it++) {
            if (event.Apply(**it) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        }
        return hui::EventResult::UNHANDLED;
    }

    void addWidget(T widget) {
        hui::Container::BecomeParentOf(widget);
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


} // namespace roa

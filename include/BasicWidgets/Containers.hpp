#pragma once
#include <concepts>

#include "hui/container.hpp"
#include "BasicWidgets/Buttons.hpp"
#include "Utilities/ROACommon.hpp"

namespace roa
{


template <WidgetDerived T> 
class ZContainer : public hui::Container {
public:
    ZContainer(hui::UI *ui): hui::Container(ui) {}
    virtual ~ZContainer() = default;
    virtual void BringToFront(T *widget) = 0;
};

template <WidgetDerived T>
class ListContainer : public ZContainer<T> {
protected:
    std::list<std::unique_ptr<T>> children;

    hui::EventResult PropagateToChildren(hui::Event &event) override {
        for (auto it = children.rbegin(); it != children.rend(); it++) {
            if (event.Apply(**it) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        }
        return hui::EventResult::UNHANDLED;
    }
public:
    ListContainer(hui::UI *ui): ZContainer<T>(ui) {}
    virtual ~ListContainer() = default;

    void AddWidget(T  *widget) {
        BecomeParentOf(widget);
        children.emplace_front(widget);
    }
    
    void BringToFront(T *widget) override {
        assert(widget);
    
        auto it = std::find(children.begin(), children.end(), widget);
        if(it != children.end()) {
            children.erase(it);
            children.emplace_front(widget);
        }
    }
};

template <WidgetDerived T>
class LinContainer : public ZContainer<T> {
protected:
    std::vector<std::unique_ptr<T>> children; 
    std::vector<std::unique_ptr<T>> erasable;
public:
    LinContainer(hui::UI *ui): ZContainer<T>(ui) {}
    virtual ~LinContainer() = default;

    void AddWidget(T *widget) {
        hui::Container::BecomeParentOf(widget);
        children.emplace(children.begin(), widget);
    }

    void EraseWidget(T *widget) {
        auto it = std::find_if(children.begin(), children.end(), [widget](const auto &ptr){ return ptr.get() == widget; });
        if (it != children.end()) {
            erasable.push_back(std::move(*it));
            children.erase(it);
        }
    }

    void BringToFront(T *widget) override {
        auto it = std::find_if(children.begin(), children.end(), [widget](const auto &ptr){ return ptr.get() == widget; });
        if (it != children.end()) {
            auto uptr = std::move(*it);
            children.erase(it);
            children.insert(children.begin(), std::move(uptr));
        }
    }

protected:
    hui::EventResult PropagateToChildren(hui::Event &event) override {
        for (auto it = children.rbegin(); it != children.rend(); it++) {
            if (event.Apply(**it) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        }
        return hui::EventResult::UNHANDLED;
    }

};


} // namespace roa

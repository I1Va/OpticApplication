#pragma once
#include <concepts>

#include "hui/container.hpp"
#include "BasicWidgets/Buttons.hpp"
#include "Utilities/ROACommon.hpp"

namespace roa
{

class Container : public hui::Container {
protected:
    std::vector<std::unique_ptr<hui::Widget>> children; 
    std::vector<std::unique_ptr<hui::Widget>> erasable;
public:
    using hui::Container::Container;
    Container(const Container&) = delete;
    virtual ~Container() = default;   
    Container& operator=(const Container&) = delete;
    Container(Container&&) = default;
    Container& operator=(Container&&) = default;

    void AddWidget(std::unique_ptr<hui::Widget> widget) {
        hui::Container::BecomeParentOf(widget.get());
        children.push_back(std::move(widget));
        ForceRedraw();
    }

    bool CheckImplicitHover() const {
        hui::Widget *hover = GetUI()->GetHovered();
        while (hover && (hover != this)) {
            if (hover == hover->GetParent()) break;
            hover = hover->GetParent();
        }

        return hover == this;
    }

    void EraseWidget(hui::Widget *widget) {
        auto it = std::find_if(children.begin(), children.end(), [widget](const auto &ptr){ return ptr.get() == widget; });
        if (it != children.end()) {
            erasable.push_back(std::move(*it));
            children.erase(it);
        }
    }

    void BringToFront(hui::Widget *widget) {
        auto it = std::find_if(children.begin(), children.end(), [widget](const auto &ptr){ return ptr.get() == widget; });
        if (it != children.end()) {
            auto uptr = std::move(*it);
            children.erase(it);
            children.insert(children.begin(), std::move(uptr));
        }
    }

protected:
    hui::EventResult PropagateToChildren(hui::Event &event) override {
        for (auto it = children.begin(); it != children.end(); it++) {
            if (event.Apply(**it) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        }
        return hui::EventResult::UNHANDLED;
    }

    hui::EventResult OnIdle(hui::IdleEvent &evt) {
        PropagateToChildren(evt);
        erasable.clear();
        return hui::EventResult::UNHANDLED;
    }
};

} // namespace roa

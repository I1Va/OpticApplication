#pragma once
#include <iostream>
#include "BasicWidgets/Containers.hpp"

namespace roa
{

class Desktop final : public Container {
    std::unique_ptr<hui::Widget> modal;
    bool modalActivated = false;

    std::vector<std::unique_ptr<hui::Widget>> widgets;

public:
    using Container::Container;
    Desktop(const Container&) = delete;
    ~Desktop() = default;  
    Desktop& operator=(const Desktop&) = delete;
    Desktop(Desktop&&) = default;
    Desktop& operator=(Desktop&&) = default;

    hui::EventResult PropagateToChildren(hui::Event &event) override {
        if (modal && modalActivated) return event.Apply(*modal);
        
        for (auto &child : widgets) {
            if (event.Apply(*child) == hui::EventResult::HANDLED) {
                return hui::EventResult::HANDLED;
            }
        }
    
        return hui::EventResult::UNHANDLED;
    }

    void SetModal(std::unique_ptr<hui::Widget> widget) {
        if (modal) {
            std::cerr << "modal widget has been already set\n";
            return;
        }
    
        BecomeParentOf(widget.get());
        modal.reset(widget.release());
    }

    void ActivateModal() { 
        modalActivated = true;
        ForceRedraw(); 
    }
    void DeactivateModal() {
        modalActivated = false; 
        ForceRedraw(); 
    }
    void SwitchModalActiveFlag() {
        modalActivated = !modalActivated;
        ForceRedraw(); 
    }

protected:
    void Redraw() const override {
        GetTexture().Clear({61, 61, 61, 255});

        for (auto &widget : widgets) widget->DrawOn(GetTexture());

        if (modal && modalActivated) {
            modal->DrawOn(GetTexture());
        }
    }
};

} // namespace roa

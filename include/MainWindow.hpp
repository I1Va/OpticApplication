#pragma once
#include <iostream>

#include "Containers.hpp"

namespace roa
{

class MainWindow final : public ZContainer<hui::Widget> {
    std::unique_ptr<hui::Widget> modal;
    bool modalActivated = false;

    std::vector<std::unique_ptr<hui::Widget>> widgets;

public:
    MainWindow(hui::UI *ui): ZContainer<hui::Widget>(ui) {}
    ~MainWindow() = default;

    hui::EventResult PropagateToChildren(hui::Event &event) override {
        if (modal && modalActivated) return event.Apply(*modal);
        

        for (auto &child : widgets) {
            if (event.Apply(*child) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        }
    
        return hui::EventResult::UNHANDLED;
    }

    void AddWidget(hui::Widget *widget) {
        BecomeParentOf(widget);
        widgets.emplace(widgets.begin(), widget);
    }

    void SetModal(hui::Widget *widget) {
        if (modal) {
            std::cerr << "modal widget has been already set\n";
            return;
        }
    
        BecomeParentOf(widget);
        modal.reset(widget);
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

    void BringToFront(hui::Widget *widget) override {
        auto widgetsIt = std::find_if(widgets.begin(), widgets.end(), [widget](const auto &w){ return w.get() == widget; } );
        if(widgetsIt != widgets.end()) {
            widgets.erase(widgetsIt);       
            widgets.emplace(widgets.begin(), widget);
        }
    }

protected:
    void Redraw() const override {
        GetTexture().Clear({20, 20, 20, 255});

        for (auto &widget : widgets) widget->DrawOn(GetTexture());

        if (modal && modalActivated) {
            modal->DrawOn(GetTexture());
        }
    }
};

} // namespace roa

#pragma once
#include "Containers.hpp"

namespace roa
{

class MainWindow final : public ZContainer<hui::Widget> {
    std::vector<std::unique_ptr<hui::Widget>> modals;
    std::vector<std::unique_ptr<hui::Widget>> widgets;

public:
    MainWindow(hui::UI *ui): ZContainer<hui::Widget>(ui) {}
    ~MainWindow() = default;

    hui::EventResult PropagateToChildren(hui::Event &event) override {
        for (auto &child : modals) {
            if (event.Apply(*child) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        }
        for (auto &child : widgets) {
            if (event.Apply(*child) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        }
    
        return hui::EventResult::UNHANDLED;
    }

    void addWidget(hui::Widget *widget) {
        BecomeParentOf(widget);
        widgets.emplace(widgets.begin(), widget);
    }

    void addModal(hui::Widget *widget) {
        BecomeParentOf(widget);
        modals.emplace(modals.begin(), widget);
    }

    void BringToFront(hui::Widget *widget) override {
        auto widgetsIt = std::find_if(widgets.begin(), widgets.end(), [widget](const auto &w){ return w.get() == widget; } );
        if(widgetsIt != widgets.end()) {
            widgets.erase(widgetsIt);       
            widgets.emplace(widgets.begin(), widget);
        }

        auto modalsIt = std::find_if(modals.begin(), modals.end(), [widget](const auto &w){ return w.get() == widget; });
        if(modalsIt != modals.end()) {
            modals.erase(modalsIt);       
            modals.emplace(widgets.begin(), widget);   
        }   
    }

protected:
    void Redraw() const override {
        GetTexture().Clear({50, 50, 50, 255});
        for (auto &widget : widgets) widget->DrawOn(GetTexture());
        for (auto &modal : modals) modal->DrawOn(GetTexture());
    }
};

} // namespace roa

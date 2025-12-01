#pragma once
#include "Containers.hpp"

namespace roa
{

class MainWindow : public ZContainer<hui::Widget *> {
    std::vector<hui::Widget *> modals;
    std::vector<hui::Widget *> widgets;

public:
    MainWindow(hui::UI *ui): ZContainer<hui::Widget *>(ui) {}
    ~MainWindow() {
        for (hui::Widget *child : modals)  delete child;
        for (hui::Widget *child : widgets) delete child;
    }

    hui::EventResult PropagateToChildren(hui::Event &event) override {
        for (hui::Widget *child : modals) {
            if (event.Apply(*child) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        }
        for (hui::Widget *child : widgets) {
            if (event.Apply(*child) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        }
    
        return hui::EventResult::UNHANDLED;
    }

    void addWidget(hui::Widget *widget) {
        BecomeParentOf(widget);
        widgets.push_back(widget);
    }

    void addModal(hui::Widget *widget) {
        BecomeParentOf(widget);
        modals.push_back(widget);
    }

    void BringToFront(hui::Widget *widget) override {
        auto widgetsIt = std::find(widgets.begin(), widgets.end(), widget);
        if(widgetsIt != widgets.end()) {
            widgets.erase(widgetsIt);       
            widgets.push_back(widget);   
        }

        auto modalsIt = std::find(modals.begin(), modals.end(), widget);
        if(modalsIt != modals.end()) {
            modals.erase(modalsIt);       
            modals.push_back(widget);   
        }   
    }

protected:
    void Redraw() const override {
        GetTexture().Clear({50, 50, 50, 255});
        for (hui::Widget * widget : widgets) widget->DrawOn(GetTexture());
        for (hui::Widget * modal : modals) modal->DrawOn(GetTexture());
    }
};

} // namespace roa

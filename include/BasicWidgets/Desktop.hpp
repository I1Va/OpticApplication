#pragma once
#include "BasicWidgets/Containers.hpp"
#include "CompositeWidgets/DropDownMenu.hpp"

namespace roa
{



class Desktop final : public Container {
    std::unique_ptr<dr4::Rectangle> mainMenuBackGround;
    std::vector<DropDownMenu *> mainMenu;
    
    std::unique_ptr<hui::Widget> modal;
    bool modalActivated = false;

public:
    static constexpr float MAIN_MENU_HEIGHT = 20;
    const dr4::Color mainMenuColor = dr4::Color(24, 24, 24, 255);
    const dr4::Color BGColor = dr4::Color(61, 61, 61, 255);
    
    Desktop(hui::UI *ui) : Container(ui), mainMenuBackGround(ui->GetWindow()->CreateRectangle()) {
        assert(ui);
    
        SetSize(ui->GetWindow()->GetSize());
        mainMenuBackGround->SetSize({GetSize().x, MAIN_MENU_HEIGHT});
        mainMenuBackGround->SetFillColor(mainMenuColor);
    }

    Desktop(const Container&) = delete;
    ~Desktop() = default;  
    Desktop& operator=(const Desktop&) = delete;
    Desktop(Desktop&&) = default;
    Desktop& operator=(Desktop&&) = default;
    
    void AddMaiMenuItem(std::unique_ptr<DropDownMenu> item) {
        item->SetPos(calculateMainMenuWidth(), 0);
        mainMenu.push_back(item.get());
        
        auto *itemPtr = item.get();
        AddWidget(std::move(item));
        BringToFront(itemPtr);
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


protected:
    hui::EventResult PropagateToChildren(hui::Event &event) override {
        if (modal && modalActivated) return event.Apply(*modal);
        
        for (auto &child : children) {
            if (event.Apply(*child) == hui::EventResult::HANDLED) {
                return hui::EventResult::HANDLED;
            }
        }
    
        return hui::EventResult::UNHANDLED;
    }

    void Redraw() const override {
        GetTexture().Clear(BGColor);
        mainMenuBackGround->DrawOn(GetTexture());

        for (auto it = children.rbegin(); it != children.rend(); it++) {
            (*it)->DrawOn(GetTexture());
        }
        if (modal && modalActivated) {
            modal->DrawOn(GetTexture());
        }
    }

private:
    float calculateMainMenuWidth() const { 
        float res = 0;
        for (auto item : mainMenu) {
            res += item->GetSize().x;
        }
        return res;
    }
};

} // namespace roa

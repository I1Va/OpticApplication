#pragma once
#include <vector>
#include <unordered_map>

#include "pp/canvas.hpp"
#include "pp/tool.hpp"
#include "cum/ifc/pp.hpp"

#include "hui/widget.hpp"

#include "Containers.hpp"
namespace roa
{


class PPToolButton final : public SimpButton {
    pp::Tool* const tool;
    pp::Tool* selectedTool = nullptr;

public:
    PPToolButton(hui::UI *ui, pp::Tool* const inpTool): SimpButton(ui), tool(inpTool) {}
    ~PPToolButton() = default;

// TODO:
// bool optor::PiskaToolButton::OnMousePress  (const ::dr4::Event& event) {
//     if (isHide_) { return false; }

//     if (state_->hoveredWidget == this && event.mouseButton.button == pressButton_) {
//         if (!isPressed_) {
//             if (*selectedTool_) {
//                 (*selectedTool_)->OnEnd();
//             }
//             *selectedTool_ = tool_;
//         } else {
//             if (*selectedTool_) {
//                 (*selectedTool_)->OnEnd();
//             }
//             *selectedTool_ = nullptr;
//         }
//         return true;
//     }

//     return optor::Widget::OnMousePress(event);
// }

// bool optor::PiskaToolButton::OnMouseRelease(const ::dr4::Event& event) {
//     if (isHide_) { return false; }

//     if (ERROR_HANDLE([this, &event](){return optor::Widget::OnMouseRelease(event);})) {
//         return true;
//     }

//     return false;
// }

// bool optor::PiskaToolButton::OnKeyboardPress(const dr4::Event& event) {
//     if (isHide_) { return false; }

//     if (event.key.sym == dr4::KeyCode::KEYCODE_ESCAPE && *selectedTool_ == tool_ && tool_->IsCurrentlyDrawing()) {
//         (*selectedTool_)->OnBreak();
//         return true;
//     }

//     return optor::WidgetButton::OnKeyboardPress(event);
// }

// void optor::PiskaToolButton::OnIdle() {
//     if (*selectedTool_ == tool_) {
//         isPressed_ = true;
//         rect_->SetFillColor(pressedColor_);
//     } else {
//         isPressed_ = false;
//         rect_->SetFillColor(releasedColor_);
//     }

//     optor::WidgetButton::OnIdle();
// }

// void optor::PiskaToolButton::Draw(dr4::Texture &srcTexture) {
//     if (isHide_) { return; }

//     const dr4::Vec2f pos = rect_->GetPos();

//     rect_->SetPos({0, 0});
//     ERROR_HANDLE([this](){
//         optor::Widget::Draw(*texture_);
//     });
//     rect_->SetPos(pos);

//     ERROR_HANDLE([this](){
//         texture_->Draw(*text_);
//     });

//     ERROR_HANDLE([this, &srcTexture](){
//         srcTexture.Draw(*texture_);
//     });
// }

};


class PPCanvasWidget : public LinContainer<PPToolButton>, public pp::Canvas {
    std::vector<std::unique_ptr<pp::Tool>> tools;
    std::unordered_map<pp::Shape*, std::unique_ptr<pp::Shape>> shapes_;

    pp::ControlsTheme theme;
    pp::Tool* selectedTool = nullptr;
    pp::Shape* selectedShape = nullptr;

public:
    PPCanvasWidget
    (
        hui::UI *ui,
        std::vector<cum::PPToolPlugin*> &toolPlugins
    ) : LinContainer(ui)
    {
        assert(ui);

        for (auto* plugin : toolPlugins) {
            for (auto& tool : plugin->CreateTools(this)) {
                tools.push_back(std::move(tool));
            }
        }

        for (auto &tool : tools) {
            // crete buttons
            roa::PPToolButton *toolButton = new roa::PPToolButton(ui, tool.get());
            
            addWidget(toolButton);
        }
    
        SetPos({20, 20});
        SetSize({GetUI()->GetWindow()->GetSize() - dr4::Vec2f(40, 40)});
    }

protected:

    void Redraw() const override {
        GetTexture().Clear(FULL_TRANSPARENT);
        static dr4::Rectangle *borders = nullptr;
        if (!borders) {
            borders = GetUI()->GetWindow()->CreateRectangle();
            borders->SetSize(GetSize());
            borders->SetBorderColor(RED);
            borders->SetBorderThickness(20);
            borders->SetFillColor(FULL_TRANSPARENT);    
        }
        
        borders->DrawOn(GetTexture());
    }

    void OnSizeChanged() override {

    }
    pp::ControlsTheme GetControlsTheme() const override { return theme; }

    void AddShape(pp::Shape *shape) override {
        assert(shape);
        if (shapes_.find(shape) != shapes_.end()) {
            std::cerr << "AddShape : shape" << shape << "has been already added\n";
            return; 
        }
    
        shapes_.emplace(shape, std::unique_ptr<pp::Shape>(shape));
    }

    void DelShape(pp::Shape *shape) override {
        assert(shape);
        
        if (!shapes_.erase(shape)) {
            std::cerr << "DelShape : shape " << shape << " was not in shape list\n";
        }
    }

    void SetSelectedShape(pp::Shape *shape) override { selectedShape = shape; }
    pp::Shape *GetSelectedShape() const override { return selectedShape; }

    void ShapeChanged(pp::Shape *) override {
        // TODO
    }

    dr4::Window *GetWindow() override { return GetUI()->GetWindow(); }


// bool optor::WidgetPiska::OnMouseMove(const dr4::Event& event) {
//     if (isHide_) { return false; }

//     dr4::Event childEvent(event);
//     childEvent.mouseMove.pos -= AbsCoord() + texture_->GetZero();

//     if (selectedTool_) {
//         if (selectedTool_->OnMouseMove(childEvent.mouseMove)) {
//             return true;
//         }
//     }

//     for (auto& shape : shapes_) {
//         if (shape.second->OnMouseMove(childEvent.mouseMove)) {
//             return true;
//         }
//     }

//     optor::WidgetChildable::OnMouseMove(event);

//     return true;
// }

// bool optor::WidgetPiska::OnMousePress(const dr4::Event& event) {
//     if (isHide_) { return false; }

//     for (auto childIt = children_.rbegin(); childIt != children_.rend(); ++childIt) {
//         if (!(*childIt)->GetMustRemoved() && ERROR_HANDLE([childIt, &event](){
//                 return (*childIt)->OnMousePress(event);
//         })) {
//             return true;
//         }
//     }

//     dr4::Event childEvent(event);
//     childEvent.mouseButton.pos -= AbsCoord() + texture_->GetZero();


//     if (selectedTool_) {
//         if (selectedTool_->OnMouseDown(childEvent.mouseButton)) {
//             return true;
//         }
//     }

//     for (auto& shape : shapes_) {
//         if (shape.second->OnMouseDown(childEvent.mouseButton)) {
//             return true;
//         }
//     }

//     if (event.mouseButton.button == dr4::MouseButtonType::LEFT) {
//         SetSelectedShape(nullptr);
//     }

//     return true;
// }

// bool optor::WidgetPiska::OnMouseRelease(const dr4::Event& event) {
//     if (isHide_) { return false; }

//     dr4::Event childEvent(event);
//     childEvent.mouseButton.pos -= AbsCoord() + texture_->GetZero();

//     if (selectedTool_) {
//         if (selectedTool_->OnMouseUp(childEvent.mouseButton)) {
//             return true;
//         }
//     }

//     for (auto& shape : shapes_) {
//         if (shape.second->OnMouseUp(childEvent.mouseButton)) {
//             return true;
//         }
//     }

//     if (optor::WidgetChildable::OnMouseRelease(event)) {
//         return true;
//     }

//     return true;
// }

// bool optor::WidgetPiska::OnKeyboardPress(const dr4::Event& event) {
//     if (isHide_) { return false; }

//     dr4::Event childEvent(event);

//     if (selectedTool_) {
//         if (selectedTool_->OnKeyDown(childEvent.key)) {
//             return true;
//         }
//     }

//     for (auto& shape : shapes_) {
//         if (shape.second->OnKeyDown(childEvent.key)) {
//             return true;
//         }
//     }

//     if (event.key.sym == dr4::KeyCode::KEYCODE_ESCAPE) {
//         SetMustRemoved(true);
//         return true;
//     }

//     optor::WidgetChildable::OnKeyboardPress(event);

//     return true;
// }

// bool optor::WidgetPiska::OnKeyboardRelease(const dr4::Event& event) {
//     if (isHide_) { return false; } 

//     dr4::Event childEvent(event);

//     if (selectedTool_) {
//         if (selectedTool_->OnKeyUp(childEvent.key)) {
//             return true;
//         }
//     }

//     for (auto& shape : shapes_) {
//         if (shape.second->OnKeyUp(childEvent.key)) {
//             return true;
//         }
//     }

//     optor::WidgetChildable::OnKeyboardRelease(event);

//     return true;
// }

// bool optor::WidgetPiska::OnTextInput(const dr4::Event& event) {
//     if (isHide_) { return false; } 

//     dr4::Event childEvent(event);

//     if (selectedTool_) {
//         if (selectedTool_->OnText(childEvent.text)) {
//             return true;
//         }
//     }

//     for (auto& shape : shapes_) {
//         if (shape.second->OnText(childEvent.text)) {
//             return true;
//         }
//     }

//     optor::WidgetChildable::OnTextInput(event);

//     return true;
// }

// void optor::WidgetPiska::OnIdle() {
//     if (isHide_) { return; }

//     optor::WidgetChildable::OnIdle();
// }






private:
};


} // namespace roa



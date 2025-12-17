#pragma once
#include <vector>
#include <unordered_map>

#include "pp/canvas.hpp"
#include "pp/tool.hpp"
#include "cum/ifc/pp.hpp"

#include "hui/widget.hpp"

#include "BasicWidgets/Containers.hpp"
#include "PPColorPicker.hpp"
#include "CompositeWidgets/DropDownMenu.hpp"
#include "CompositeWidgets/RecordsPanel.hpp"
#include "CompositeWidgets/RecordsPanel.hpp"
#include "PPColorPicker.hpp"

namespace roa
{

class PPCanvasWidget : public Container, public pp::Canvas {
    const int BORDER_RADIUS = 3;
    const int BORDER_THICKNESS = 3;
    const dr4::Color BORDER_COLOR = dr4::Color(255, 255, 0, 255);
    pp::ControlsTheme theme = 
    {
        .shapeFillColor = RED,
        .shapeBorderColor = BLACK,
        .selectColor = GREEN,
        .textColor = BLACK,
        .baseFontSize = 20,
        .handleColor = { 255, 105, 180, 255 },
        .handleHoverColor = { 255, 105, 180, 255 },
        .handleActiveColor = { 255, 105, 180, 255 }
    };

    OutlinerWindow<pp::Tool *> *toolsMenu = nullptr;
    ColorPickerWindow          *colorPicker = nullptr;

    std::unordered_map<pp::Shape*, std::unique_ptr<pp::Shape>> shapes;

    std::vector<std::unique_ptr<pp::Tool>> tools;

    pp::Tool* selectedTool = nullptr;
    pp::Shape* selectedShape = nullptr;

public:
    PPCanvasWidget(hui::UI *ui) : Container(ui)
    {
        assert(ui);

        auto toolsMenuUnique = std::make_unique<OutlinerWindow<pp::Tool *>>(ui);
        toolsMenu = toolsMenuUnique.get();
        
        auto colorPickerUnique = std::make_unique<roa::ColorPickerWindow>(ui, theme);
        colorPicker = colorPickerUnique.get();

        SetSize({GetUI()->GetWindow()->GetSize()});
        
        colorPicker->SetSize({200, 200});
        colorPicker->SetPos(GetSize() - colorPicker->GetSize() - dr4::Vec2f(BORDER_THICKNESS, BORDER_THICKNESS));
        
        colorPicker->SetOnColorChangedAction([&](dr4::Color c){
            theme.shapeFillColor = c;
        });
    
        AddWidget(std::move(colorPickerUnique));
              
        toolsMenu->SetRecordButtonMode(Button::Mode::CAPTURE_MODE);

        AddWidget(std::move(toolsMenuUnique));
    }

    void LoadToolPlugin(cum::PPToolPlugin* toolPlugin) {
        assert(toolPlugin);
        for (auto& tool : toolPlugin->CreateTools(this)) {
            pp::Tool *toolPtr = tool.get();
            toolsMenu->AddRecord
            (
                toolPtr, std::string(tool->Icon()),
                [this, toolPtr]() { 
                    selectedTool = toolPtr; 
                },
                nullptr,
                "./assets/icons/sculptmode_hlt.svg"
            );
            tools.push_back(std::move(tool));
        }
    }

protected:
    void layout() {
        toolsMenu->SetSize(100, 100);
        toolsMenu->SetPos(GetSize().x - BORDER_THICKNESS * 3 - toolsMenu->GetSize().x, BORDER_THICKNESS * 3);

        ForceRedraw();
    }

    void Redraw() const override {
        GetTexture().Clear(FULL_TRANSPARENT);

        
        dr4::Image *backSurface = GetTexture().GetImage(); assert(backSurface);

        DrawBlenderRoundedFrame(
            backSurface->GetWidth(),
            backSurface->GetHeight(),
            BORDER_RADIUS,               
            BORDER_THICKNESS,                 
            BORDER_COLOR,
            [&](int x, int y, dr4::Color c) { backSurface->SetPixel(x,y,c); }
        );

        backSurface->DrawOn(GetTexture());
        
        for (auto &child : children) {
            child->DrawOn(GetTexture());
        }
        
        for (const auto& shape : shapes) {
            shape.second->DrawOn(GetTexture());
        }   
    }

    void OnSizeChanged() override { layout(); }

    pp::ControlsTheme GetControlsTheme() const override { return theme; }

    void AddShape(pp::Shape *shape) override {
        assert(shape);
        if (shapes.find(shape) != shapes.end()) {
            std::cerr << "AddShape : shape" << shape << "has been already added\n";
            return; 
        }
    
        shapes.emplace(shape, std::unique_ptr<pp::Shape>(shape));
    }

    void DelShape(pp::Shape *shape) override {
        assert(shape);
        
        if (!shapes.erase(shape)) {
            std::cerr << "DelShape : shape " << shape << " was not in shape list\n";
        }
    }

    void SetSelectedShape(pp::Shape *shape) override { selectedShape = shape; }
    pp::Shape *GetSelectedShape() const override { return selectedShape; }

    void ShapeChanged(pp::Shape *) override {
        // TODO
    }

    dr4::Window *GetWindow() override { return GetUI()->GetWindow(); }

    hui::EventResult OnMouseMove(hui::MouseMoveEvent &event) {
        if (GetRect().Contains(event.pos)) {
            event.pos -= GetPos();
            if (PropagateToChildren(event) == hui::EventResult::HANDLED) {
                event.pos += GetPos();
                return hui::EventResult::HANDLED;
            } else {
                dr4::Event::MouseMove dr4ChildEvent(event.pos, event.rel);
                if (selectedTool) {
                    if (selectedTool->OnMouseMove(dr4ChildEvent)) {
                        ForceRedraw();
                        return hui::EventResult::HANDLED;
                    }
                }

                for (auto& shape : shapes) {
                    if (shape.second->OnMouseMove(dr4ChildEvent)) {
                        ForceRedraw();
                        return hui::EventResult::HANDLED;
                    }
                }

                return Widget::OnMouseMove(event);
            }
        }
        return hui::EventResult::UNHANDLED;
    }

    hui::EventResult OnMouseDown(hui::MouseButtonEvent &event) {
        if (GetRect().Contains(event.pos)) {
            event.pos -= GetPos();
            if (PropagateToChildren(event) == hui::EventResult::HANDLED) {
                event.pos += GetPos();
                return hui::EventResult::HANDLED;
            } else {
                dr4::Event::MouseButton dr4ChildEvent(event.button, event.pos);

             
                if (selectedTool) {
                    if (selectedTool->OnMouseDown(dr4ChildEvent)) {
                        ForceRedraw();
                        return hui::EventResult::HANDLED;
                    }
                }

                for (auto& shape : shapes) {
                    if (shape.second->OnMouseDown(dr4ChildEvent)) {
                        ForceRedraw();
                        return hui::EventResult::HANDLED;
                    }
                }

                if (dr4ChildEvent.button == dr4::MouseButtonType::LEFT) {
                    SetSelectedShape(nullptr);
                }

                return Widget::OnMouseDown(event);
            }
        }
        return hui::EventResult::UNHANDLED;
    }

    hui::EventResult OnMouseUp(hui::MouseButtonEvent &event) {
    if (GetRect().Contains(event.pos)) {
        event.pos -= GetPos();
        if (PropagateToChildren(event) == hui::EventResult::HANDLED) {
            event.pos += GetPos();
            return hui::EventResult::HANDLED;
        } else {
            dr4::Event::MouseButton dr4ChildEvent(event.button, event.pos);

            if (selectedTool) {
                if (selectedTool->OnMouseUp(dr4ChildEvent)) {
                    ForceRedraw();
                    return hui::EventResult::HANDLED;
                }
            }

            for (auto& shape : shapes) {
                if (shape.second->OnMouseUp(dr4ChildEvent)) {
                    ForceRedraw();
                    return hui::EventResult::HANDLED;
                }
            }

            return Widget::OnMouseUp(event);
        }
    }
    return hui::EventResult::UNHANDLED;
}

    hui::EventResult OnKeyDown(hui::KeyEvent &event) {
        if (PropagateToChildren(event) == hui::EventResult::HANDLED)
            return hui::EventResult::HANDLED;
        
        dr4::Event::KeyEvent dr4ChildEvent(event.key, event.mods);

        if (selectedTool) {
            if (selectedTool->OnKeyDown(dr4ChildEvent)) {
                ForceRedraw();
                return hui::EventResult::HANDLED;
            }
        }

        for (auto& shape : shapes) {
            if (shape.second->OnKeyDown(dr4ChildEvent)) {
                ForceRedraw();
                return hui::EventResult::HANDLED;
            }
        }
    
        return Widget::OnKeyDown(event);
    }

    hui::EventResult OnKeyUp(hui::KeyEvent &event) {
        if (PropagateToChildren(event) == hui::EventResult::HANDLED)
            return hui::EventResult::HANDLED;

        dr4::Event::KeyEvent dr4ChildEvent(event.key, event.mods);

        if (selectedTool) {
            if (selectedTool->OnKeyUp(dr4ChildEvent)) {
                ForceRedraw();
                return hui::EventResult::HANDLED;
            }
        }

        for (auto& shape : shapes) {
            if (shape.second->OnKeyUp(dr4ChildEvent)) {
                ForceRedraw();
                return hui::EventResult::HANDLED;
            }
        }

        return Widget::OnKeyUp(event);
    }

    hui::EventResult OnText(hui::TextEvent &event) {
        if (PropagateToChildren(event) == hui::EventResult::HANDLED)
            return hui::EventResult::HANDLED;


        dr4::Event::TextEvent dr4ChildEvent(event.text);

        if (selectedTool) {
            if (selectedTool->OnText(dr4ChildEvent)) {
                ForceRedraw();
                return hui::EventResult::HANDLED;
            }
        }

        for (auto& shape : shapes) {
            if (shape.second->OnText(dr4ChildEvent)) {
                ForceRedraw();
                return hui::EventResult::HANDLED;
            }
        }

        return Widget::OnText(event);
    }

    hui::EventResult OnIdle(hui::IdleEvent &event) {
        PropagateToChildren(event);
        
        return hui::EventResult::UNHANDLED;
    }

private:
};

} // namespace roa



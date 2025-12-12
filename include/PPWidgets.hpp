#pragma once
#include <vector>
#include <unordered_map>

#include "pp/canvas.hpp"
#include "pp/tool.hpp"
#include "cum/ifc/pp.hpp"

#include "hui/widget.hpp"

#include "BasicWidgets/Containers.hpp"
#include "PPColorPicker.hpp"
#include "DropDownMenu.hpp"
#include "RecordsPanel.hpp"
#include "RecordsPanel.hpp"
#include "PPColorPicker.hpp"

namespace roa
{

class PPToolButton final : public TextButton {
    pp::Tool *const tool;
    pp::Tool **selectedToolSlot = nullptr;

public:
    PPToolButton(hui::UI *ui, pp::Tool* const inpTool, pp::Tool **inpSelectedToolSlot): TextButton(ui), tool(inpTool), selectedToolSlot(inpSelectedToolSlot) {
        assert(ui); 
        SetText(std::string(inpTool->Icon()));
    }
    ~PPToolButton() = default;

protected:
    hui::EventResult OnMouseDown(hui::MouseButtonEvent &event) override {        
        if (TextButton::OnMouseDown(event) == hui::EventResult::HANDLED) {
            if (*selectedToolSlot) (*selectedToolSlot)->OnEnd();    
            if (pressed) *selectedToolSlot = tool;
            else *selectedToolSlot = nullptr;
            
            return hui::EventResult::HANDLED;
        }
        return hui::EventResult::UNHANDLED;
    }

    hui::EventResult OnKeyDown(hui::KeyEvent &event) override {
        if (event.key == dr4::KeyCode::KEYCODE_ESCAPE && *selectedToolSlot == tool && tool->IsCurrentlyDrawing()) {
            (*selectedToolSlot)->OnBreak();
            return hui::EventResult::HANDLED;
        }
        return TextButton::OnKeyDown(event);
    }

    hui::EventResult OnIdle(hui::IdleEvent &event) override {
        TextButton::OnIdle(event);

        if (*selectedToolSlot == tool) {
            if (!pressed) {
                ForceRedraw();
                if (onPressAction) onPressAction();
            }
            pressed = true;    
        } else {
            if (pressed) {
                ForceRedraw();
                if (onUnpressAction) onUnpressAction();
            }
            pressed = false;    
        }

        return hui::EventResult::UNHANDLED;
    }
};

class PPCanvasWidget : public LinContainer<hui::Widget>, public pp::Canvas {
    const int BORDER_RADIUS = 3;
    const int BORDER_THICKNESS = 3;
    const dr4::Color BORDER_COLOR = dr4::Color(255, 255, 0, 255);
    pp::ControlsTheme theme = 
    {
        .shapeFillColor = RED,
        .shapeBorderColor = BLACK,
        .textColor = BLACK,
        .baseFontSize = 20,
        .handleColor = { 255, 105, 180, 255 },
        .handleHoverColor = { 255, 105, 180, 255 },
        .handleActiveColor = { 255, 105, 180, 255 }
    };

    OutlinerWindow<pp::Tool *> *toolsMenu;
    roa::ColorPicker           *colorPicker;

    std::unordered_map<pp::Shape*, std::unique_ptr<pp::Shape>> shapes;
    std::vector<std::unique_ptr<pp::Tool>> tools;

    pp::Tool* selectedTool = nullptr;
    pp::Shape* selectedShape = nullptr;

public:
    PPCanvasWidget
    (
        hui::UI *ui,
        std::vector<cum::PPToolPlugin*> &toolPlugins
    ) : 
        LinContainer(ui),
        toolsMenu(new OutlinerWindow<pp::Tool *>(ui)),
        colorPicker(new roa::ColorPicker(ui, theme))
    {
        assert(ui);

        SetSize({GetUI()->GetWindow()->GetSize()});


        colorPicker->SetPos({50, 50});
        
        colorPicker->onColorChanged = [&](dr4::Color c){
            theme.shapeFillColor = c;
        };
    
        AddWidget(colorPicker);
     
            
        toolsMenu->SetRecordButtonMode(Button::Mode::CAPTURE_MODE);

        AddWidget(toolsMenu);
        
        for (auto* plugin : toolPlugins) {
            for (auto& tool : plugin->CreateTools(this)) {
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
    }

protected:
    void layout() {
        float toolButtonSz = 40;
        dr4::Vec2f toolsInitPos = dr4::Vec2f(0, GetSize().y)
                                + dr4::Vec2f(BORDER_THICKNESS, -BORDER_THICKNESS)
                                + dr4::Vec2f(0, -toolButtonSz);
                            
        toolsMenu->SetSize(100, 100);
        toolsMenu->SetPos(GetSize().x - BORDER_THICKNESS * 2 - toolsMenu->GetSize().x, BORDER_THICKNESS);

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



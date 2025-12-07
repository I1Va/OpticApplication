#pragma once
#include "Containers.hpp"
#include "RecordsPanel.hpp"
#include "Viewport3D.hpp"
#include "ROAGUIRender.hpp"
#include "PropertiesPanel.hpp"

namespace roa 
{
 
inline void setIfStringConvertedToFloat(const std::string &inp, std::function<void(float)> setter) {
    char* end = nullptr;
    float val = std::strtof(inp.c_str(), &end);
    if (*end == '\0' && end != inp.c_str()) {
        setter(val);
    }
}

class EditorWidget final : public ZContainer<hui::Widget> {
    static constexpr double PANEL_LAYOUT_SHARE = 0.4; 
    std::unique_ptr<roa::Viewport3D> Viewport3D = nullptr;

    std::unique_ptr<ObjectsPanel<Primitives *>> objectsPanel = nullptr;
    std::unique_ptr<PropertiesPanel> propertiesPanel = nullptr;

    bool recordsNeedChange = true;

public:
    EditorWidget(hui::UI *ui): 
        ZContainer(ui),
        Viewport3D(new roa::Viewport3D(ui)),
        objectsPanel(new ObjectsPanel<::Primitives *>(ui)),
        propertiesPanel(new PropertiesPanel(ui))
    {
        assert(ui);
    
        objectsPanel->SetOnSelectChangedAction([this](){ recordsNeedChange = true; });

        objectsPanel->SetTitle("Objects");

        BecomeParentOf(Viewport3D.get());
        BecomeParentOf(objectsPanel.get());
        BecomeParentOf(propertiesPanel.get());
    }

    ~EditorWidget() = default;

    void AddObject(::Primitives *object) {
        assert(object);
    
        static size_t AddObjectIter = 0; AddObjectIter++;
    
        Viewport3D->AddObject(object);
        
        objectsPanel->AddObject(
            object, object->typeString() + std::to_string(AddObjectIter),
            [object](){ object->setSelectFlag(true); },
            [object](){ object->setSelectFlag(false); }    
        );
    }
    void AddLight(::Light *light) {
        assert(light);
        Viewport3D->AddLight(light);
    }

    void AddObject(gm::IPoint3 position, ::Primitives *object) {
        assert(object);
    
        static size_t AddObjectIter = 0; AddObjectIter++;
    
        Viewport3D->AddObject(position, object);
        
        objectsPanel->AddObject(
            object, object->typeString() + std::to_string(AddObjectIter),
            [object](){ object->setSelectFlag(true); },
            [object](){ object->setSelectFlag(false); }    
        );
    }
    void AddLight(gm::IPoint3 position, ::Light *light) {
        assert(light);
        Viewport3D->AddLight(position, light);
    }

    std::vector<::Primitives *> &GetPrimitives() { return Viewport3D->GetPrimitives(); }
    std::vector<::Light *>      &GetLights()     { return Viewport3D->GetLights(); }

    SceneManager &GetSceneManager() { return Viewport3D->GetSceneManager(); }

    void BringToFront(hui::Widget *) override {}

protected:
    hui::EventResult PropagateToChildren(hui::Event &event) override {
        if (event.Apply(*Viewport3D) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        // if (event.Apply(*objectsPanel) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        // if (event.Apply(*propertiesPanel) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        return hui::EventResult::UNHANDLED;
    }

    hui::EventResult OnIdle(hui::IdleEvent &event) override {
        if (recordsNeedChange) updateRecords();
        event.Apply(*Viewport3D);
        event.Apply(*objectsPanel);
        event.Apply(*propertiesPanel);

        return hui::EventResult::UNHANDLED;
    }

    void OnSizeChanged() override { 
        layout(); 
    }    

    void Redraw() const override {
        GetTexture().Clear({FULL_TRANSPARENT});

        Viewport3D->DrawOn(GetTexture());        
        objectsPanel->DrawOn(GetTexture());

        // propertiesPanel->DrawOn(GetTexture());
    }

private:
    void layout() {
        float padding = 3;

        float Viewport3DWHCoef = 1.8;
        float Viewport3DHeight = 300;
        Viewport3D->SetSize({Viewport3DWHCoef * Viewport3DHeight, Viewport3DHeight});

        float objectsPanelHWCoef = 1;
        float objectsPanelHeight = 200;
        objectsPanel->SetSize({objectsPanelHWCoef * objectsPanelHeight, objectsPanelHeight});

        dr4::Vec2f objectsPanelPos = {Viewport3D->GetSize().x + padding, 0};
        objectsPanel->SetPos(objectsPanelPos);

        // objectsPanel->SetSize({panelWidth, panelHeight});
        // propertiesPanel->SetSize({panelWidth, panelHeight});

        // objectsPanel->SetPos({sceneWidth, 0});
        // propertiesPanel->SetPos({sceneWidth, panelHeight});
    }

    void updateRecords() {
        std::optional<std::pair<std::string, ::Primitives *>> selectedObject = objectsPanel->GetSelected();
        propertiesPanel->ClearRecords();
        
        if (selectedObject.has_value()) {
            addCordsPoperties(selectedObject.value().second);
            propertiesPanel->SetTitle(selectedObject.value().first);
        } else {
            propertiesPanel->SetTitle("");
        }
        
        recordsNeedChange = false;
    }
    

    void addCordsPoperties(::Primitives *selectedObject) {
        assert(selectedObject);
    
        std::string XContent = std::to_string(selectedObject->position().x());
        std::string YContent = std::to_string(selectedObject->position().y());
        std::string ZContent = std::to_string(selectedObject->position().z());
    
        propertiesPanel->AddProperty
            (
                "X", XContent,
                [selectedObject](const std::string &newCord)
                {
                    setIfStringConvertedToFloat(newCord, [selectedObject](float val) {
                        auto pos = selectedObject->position();
                        pos.setX(val);
                        selectedObject->setPosition(pos);
                    });
                }
            );
        propertiesPanel->AddProperty
            (
                "Y", YContent,
                [selectedObject](const std::string &newCord)
                {
                    setIfStringConvertedToFloat(newCord, [selectedObject](float val) {
                        auto pos = selectedObject->position();
                        pos.setY(val);
                        selectedObject->setPosition(pos);
                    });
                }
            );
        propertiesPanel->AddProperty
        (
            "Z", ZContent,
            [selectedObject](const std::string &newCord)
            {
                setIfStringConvertedToFloat(newCord, [selectedObject](float val) {
                    auto pos = selectedObject->position();
                    pos.setZ(val);
                    selectedObject->setPosition(pos);
                });
            }
        );
    }


};

} // namespace roa

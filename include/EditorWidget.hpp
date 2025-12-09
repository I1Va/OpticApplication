#pragma once
#include "BasicWidgets/Containers.hpp"
#include "RecordsPanel.hpp"
#include "RayTracerWidgets/Viewport3D.hpp"
#include "Utilities/ROAGUIRender.hpp"
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

class EditorWidget final : public LinContainer<hui::Widget> {
    Viewport3DWindow             *viewport3D;
    OutlinerWindow<Primitives *> *outliner;
    PropertiesWindow             *propertiesPanel;

public:
    EditorWidget(hui::UI *ui): 
        LinContainer(ui),
        viewport3D(new Viewport3DWindow(ui)),
        outliner(new OutlinerWindow<Primitives *>(ui)),
        propertiesPanel(new PropertiesWindow(ui))
    {
        assert(ui);
    
        outliner->SetOnSelectChangedAction([this](){ updateRecords(); });

        AddWidget(viewport3D);
        AddWidget(outliner);
        AddWidget(propertiesPanel);
    }

    ~EditorWidget() = default;

    void AddRecord(Primitives *object) {
        assert(object);
    
        static size_t AddObjectIter = 0; AddObjectIter++;
    
        viewport3D->AddRecord(object);
        
        outliner->AddRecord(
            object, object->typeString() + std::to_string(AddObjectIter),
            [this, object](){ object->setSelectFlag(true); },
            [this, object](){ object->setSelectFlag(false); }    
        );
    }

    void AddLight(::Light *light) {
        assert(light);
        viewport3D->AddLight(light);
    }
    void AddRecord(gm::IPoint3 position, Primitives *object) {
        assert(object);
    
        static size_t AddObjectIter = 0; AddObjectIter++;
    
        viewport3D->AddRecord(position, object);
        
        outliner->AddRecord(
            object, object->typeString() + std::to_string(AddObjectIter),
            [object](){ object->setSelectFlag(true); },
            [object](){ object->setSelectFlag(false); }    
        );
    }
    void AddLight(gm::IPoint3 position, ::Light *light) {
        assert(light);
        viewport3D->AddLight(position, light);
    }

    std::vector<::Primitives *> &GetPrimitives() { return viewport3D->GetPrimitives(); }
    std::vector<::Light *>      &GetLights()     { return viewport3D->GetLights(); }

    SceneManager &GetSceneManager() { return viewport3D->GetSceneManager(); }

    void BringToFront(hui::Widget *) override {}

protected:
    void OnSizeChanged() override { 
        layout(); 
    }    

    void Redraw() const override {
        GetTexture().Clear({FULL_TRANSPARENT});
        viewport3D->DrawOn(GetTexture());        
        outliner->DrawOn(GetTexture());
        propertiesPanel->DrawOn(GetTexture());
    }

private:
    void layout() {
        float padding = 3;
    
        float viewport3DWHCoef = 1.8;
        float viewport3DHeight = 30;
        viewport3D->SetSize({viewport3DWHCoef * viewport3DHeight, viewport3DHeight});

        float outlinerHWCoef = 1;
        float outlinerHeight = 200;
        outliner->SetSize({outlinerHWCoef * outlinerHeight, outlinerHeight});

        float propertiesWindowHWCoef = 1;
        float propertiesWindowHeight = 200;
        propertiesPanel->SetSize({propertiesWindowHWCoef * propertiesWindowHeight, propertiesWindowHeight});

        dr4::Vec2f outlinerPos = {viewport3D->GetSize().x + padding, 0};
        outliner->SetPos(outlinerPos);

        dr4::Vec2f propertiesPanelPos = outlinerPos + dr4::Vec2f(0, propertiesWindowHeight + padding);
        propertiesPanel->SetPos(propertiesPanelPos);

        viewport3D->SetSize({viewport3DWHCoef * viewport3DHeight, viewport3DHeight});
    }

    void updateRecords() {
        std::optional<std::pair<std::string, ::Primitives *>> selectedObject = outliner->GetSelected();

        propertiesPanel->ClearRecords();
        if (selectedObject.has_value()) {
            addCordsPoperties(selectedObject.value().second);
            // propertiesPanel->SetLabel(selectedObject.value().first);
        } else {
            // propertiesPanel->SetTitle("");
        }

        ForceRedraw();
    }
    

    void addCordsPoperties(::Primitives *selectedObject) {
        std::cout << "add property\n";
        assert(selectedObject);
    
        std::string XLabel = "Location X";
        std::string YLabel = "                 Y";
        std::string ZLabel = "                 Z";
    
        std::string XContent = std::to_string(selectedObject->position().x());
        std::string YContent = std::to_string(selectedObject->position().y());
        std::string ZContent = std::to_string(selectedObject->position().z());

        std::function<void(const std::string &newCord)> XCordFunction = 
        [selectedObject](const std::string &newCord)
        {
            setIfStringConvertedToFloat(newCord, [selectedObject](float val) {
                auto pos = selectedObject->position();
                pos.setY(val);
                selectedObject->setPosition(pos);
            });
        };
    
        std::function<void(const std::string &newCord)> YCordFunction = 
        [selectedObject](const std::string &newCord)
        {
            setIfStringConvertedToFloat(newCord, [selectedObject](float val) {
                auto pos = selectedObject->position();
                pos.setY(val);
                selectedObject->setPosition(pos);
            });
        };
    
        std::function<void(const std::string &newCord)> ZCordFunction = 
        [selectedObject](const std::string &newCord)
        {
            setIfStringConvertedToFloat(newCord, [selectedObject](float val) {
                auto pos = selectedObject->position();
                pos.setZ(val);
                selectedObject->setPosition(pos);
            });
        };
    
        roa::Property *transformProperty = new roa::Property(GetUI());
        transformProperty->SetLabel("Transform");
        transformProperty->AddPropertyField(XLabel, XContent, XCordFunction);
        transformProperty->AddPropertyField(YLabel, YContent, YCordFunction);
        transformProperty->AddPropertyField(ZLabel, ZContent, ZCordFunction);

        // roa::Property *MaterialProperty = new roa::Property(GetUI());
        // MaterialProperty->SetLabel("Material");
        // MaterialProperty->AddPropertyField("Diffuse  X", "11", nullptr);
        // MaterialProperty->AddPropertyField("         Y", "3", nullptr);
        // MaterialProperty->AddPropertyField("         Z", "12", nullptr);

        propertiesPanel->AddProperty(transformProperty);
    }
};

} // namespace roa

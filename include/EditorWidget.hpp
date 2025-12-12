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
        float viewport3DHeight = 350;
        viewport3D->SetSize({viewport3DWHCoef * viewport3DHeight, viewport3DHeight});

        float outlinerHWCoef = 1;
        float outlinerHeight = (viewport3DHeight - padding) / 2;
        outliner->SetSize({outlinerHWCoef * outlinerHeight, outlinerHeight});

        float propertiesWindowHWCoef = 1;
        float propertiesWindowHeight = (viewport3DHeight - padding) / 2;
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
            addCordsProperties(selectedObject.value().second);
            addMaterialProperties(selectedObject.value().second);
            addSpecialProperties(selectedObject.value().second);
            // propertiesPanel->SetLabel(selectedObject.value().first);
        } else {
            // propertiesPanel->SetTitle("");
        }

        ForceRedraw();
    }
    

    void addCordsProperties(::Primitives *selectedObject) {
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

        propertiesPanel->AddProperty(transformProperty);
    }
    void addMaterialProperties(::Primitives *selectedObject) {
        assert(selectedObject);

        roa::Property *diffuseProperty = new roa::Property(GetUI());
        roa::Property *specularProperty = new roa::Property(GetUI());
        roa::Property *emittedProperty = new roa::Property(GetUI());
    
        fillSpecularProperty(selectedObject, specularProperty);
        fillDiffuseProperty(selectedObject, diffuseProperty);
        fillEmittedProperty(selectedObject, emittedProperty);

        propertiesPanel->AddProperty(diffuseProperty);
        propertiesPanel->AddProperty(specularProperty);
        propertiesPanel->AddProperty(emittedProperty);
    }

    void addSpecialProperties(::Primitives *selectedObject) {
        ::SphereObject *sphere = dynamic_cast<::SphereObject *>(selectedObject);
        if (sphere) {
            roa::Property *sphereProperty = new roa::Property(GetUI());
            fillSphereProperty(sphere, sphereProperty);
            propertiesPanel->AddProperty(sphereProperty);
            return;
        }

        ::PlaneObject *plane = dynamic_cast<::PlaneObject *>(selectedObject);
        if (plane) {
            roa::Property *planeProperty = new roa::Property(GetUI());
            fillPlaneProperty(plane, planeProperty);
            propertiesPanel->AddProperty(planeProperty);
            return;
        }
    }

    void fillSpecularProperty(::Primitives *selectedObject, roa::Property *specularProperty) {
        assert(selectedObject);
        assert(specularProperty);

        auto &m = *selectedObject->material();

        std::string XLabel = "Specular X";
        std::string YLabel = "                 Y";
        std::string ZLabel = "                 Z";

        std::string XContent = std::to_string(m.specular().x());
        std::string YContent = std::to_string(m.specular().y());
        std::string ZContent = std::to_string(m.specular().z());

        auto setX = [selectedObject](const std::string &s){
            setIfStringConvertedToFloat(s, [selectedObject](float v){
                selectedObject->material()->specular().setX(v);
            });
        };
        auto setY = [selectedObject](const std::string &s){
            setIfStringConvertedToFloat(s, [selectedObject](float v){
                selectedObject->material()->specular().setY(v);
            });
        };
        auto setZ = [selectedObject](const std::string &s){
            setIfStringConvertedToFloat(s, [selectedObject](float v){
                selectedObject->material()->specular().setZ(v);
            });
        };

        specularProperty->SetLabel("Specular");
        specularProperty->AddPropertyField(XLabel, XContent, setX);
        specularProperty->AddPropertyField(YLabel, YContent, setY);
        specularProperty->AddPropertyField(ZLabel, ZContent, setZ);
    }
    void fillDiffuseProperty(::Primitives *selectedObject, roa::Property *diffuseProperty) {
        assert(selectedObject);
        assert(diffuseProperty);

        auto &m = *selectedObject->material();

        std::string XLabel = "Diffuse X";
        std::string YLabel = "              Y";
        std::string ZLabel = "              Z";

        std::string XContent = std::to_string(m.diffuse().x());
        std::string YContent = std::to_string(m.diffuse().y());
        std::string ZContent = std::to_string(m.diffuse().z());

        auto setX = [selectedObject](const std::string &s){
            setIfStringConvertedToFloat(s, [selectedObject](float v){
                selectedObject->material()->diffuse().setX(v);
            });
        };
        auto setY = [selectedObject](const std::string &s){
            setIfStringConvertedToFloat(s, [selectedObject](float v){
                selectedObject->material()->diffuse().setY(v);
            });
        };
        auto setZ = [selectedObject](const std::string &s){
            setIfStringConvertedToFloat(s, [selectedObject](float v){
                selectedObject->material()->diffuse().setZ(v);
            });
        };

        diffuseProperty->SetLabel("Diffuse");
        diffuseProperty->AddPropertyField(XLabel, XContent, setX);
        diffuseProperty->AddPropertyField(YLabel, YContent, setY);
        diffuseProperty->AddPropertyField(ZLabel, ZContent, setZ);
    }
    void fillEmittedProperty(::Primitives *selectedObject, roa::Property *emittedProperty) {
        assert(selectedObject);
        assert(emittedProperty);

        auto &m = *selectedObject->material();

        std::string XLabel = "Emitted X";
        std::string YLabel = "              Y";
        std::string ZLabel = "              Z";

        std::string XContent = std::to_string(m.emitted().x());
        std::string YContent = std::to_string(m.emitted().y());
        std::string ZContent = std::to_string(m.emitted().z());

        auto setX = [selectedObject](const std::string &s){
            setIfStringConvertedToFloat(s, [selectedObject](float v){
                selectedObject->material()->emitted().setX(v);
            });
        };
        auto setY = [selectedObject](const std::string &s){
            setIfStringConvertedToFloat(s, [selectedObject](float v){
                selectedObject->material()->emitted().setY(v);
            });
        };
        auto setZ = [selectedObject](const std::string &s){
            setIfStringConvertedToFloat(s, [selectedObject](float v){
                selectedObject->material()->emitted().setZ(v);
            });
        };

        emittedProperty->SetLabel("Emitted");
        emittedProperty->AddPropertyField(XLabel, XContent, setX);
        emittedProperty->AddPropertyField(YLabel, YContent, setY);
        emittedProperty->AddPropertyField(ZLabel, ZContent, setZ);
    }

    void fillSphereProperty(::SphereObject *selectedSphere, roa::Property *property) {
        assert(selectedSphere);
        assert(property);

        float radius = selectedSphere->getRadius();

        std::string radiusLabel   = "Radius";
        std::string radiusContent = std::to_string(radius);

        auto setRadius = [selectedSphere](const std::string &s){
            setIfStringConvertedToFloat(s, [selectedSphere](float v){
                selectedSphere->setRadius(v);
            });
        };

        property->SetLabel("Sphere properties");
        property->AddPropertyField(radiusLabel, radiusContent, setRadius);
    }

    void fillPlaneProperty(::PlaneObject *seletedPlane, roa::Property *property) {
        assert(seletedPlane);
        assert(property);

        std::string XLabel = "Normal X";
        std::string YLabel = "               Y";
        std::string ZLabel = "               Z";

        std::string XContent = std::to_string(seletedPlane->getNormal().x());
        std::string YContent = std::to_string(seletedPlane->getNormal().y());
        std::string ZContent = std::to_string(seletedPlane->getNormal().z());

        auto setX = [seletedPlane](const std::string &s){
            setIfStringConvertedToFloat(s, [seletedPlane](float v){
                gm::IVec3f normal = seletedPlane->getNormal();
                normal.setX(v);
                seletedPlane->setNormal(normal);
            });
        };
        auto setY = [seletedPlane](const std::string &s){
            setIfStringConvertedToFloat(s, [seletedPlane](float v){
                gm::IVec3f normal = seletedPlane->getNormal();
                normal.setY(v);
                seletedPlane->setNormal(normal);
            });
        };
        auto setZ = [seletedPlane](const std::string &s){
            setIfStringConvertedToFloat(s, [seletedPlane](float v){
                gm::IVec3f normal = seletedPlane->getNormal();
                normal.setZ(v);
                seletedPlane->setNormal(normal);
            });
        };

        property->SetLabel("Plane properties");
        property->AddPropertyField(XLabel, XContent, setX);
        property->AddPropertyField(YLabel, YContent, setY);
        property->AddPropertyField(ZLabel, ZContent, setZ);
    }
};

} // namespace roa

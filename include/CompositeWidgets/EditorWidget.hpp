#pragma once
#include <cassert>
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <cstdlib>

#include "BasicWidgets/Containers.hpp"
#include "Utilities/ROAGUIRender.hpp"
#include "RayTracerWidgets/Viewport3D.hpp"

#include "CompositeWidgets/Outliner.hpp"
#include "CompositeWidgets/RecordsPanel.hpp"
#include "CompositeWidgets/PropertiesPanel.hpp"

namespace roa 
{

inline void setIfStringConvertedToFloat(const std::string &inp, std::function<void(float)> setter) {
    char* end = nullptr;
    float val = std::strtof(inp.c_str(), &end);
    if (*end == '\0' && end != inp.c_str()) {
        setter(val);
    }
}

class EditorWidget final : public Container {
    Viewport3DWindow             *viewport3D      = nullptr;
    OutlinerWindow<Primitives *> *outliner        = nullptr;
    PropertiesWindow             *propertiesPanel = nullptr;
    
    RTMaterialManager materialManager;

public:
    EditorWidget(hui::UI *ui): Container(ui)
    {
        assert(ui);
    
        auto viewport3DUnique = std::make_unique<Viewport3DWindow>(ui);
        auto outlinerUnique = std::make_unique<OutlinerWindow<Primitives *>>(ui);
        auto propertiesPanelUnique = std::make_unique<PropertiesWindow>(ui);

        viewport3D = viewport3DUnique.get();
        outliner = outlinerUnique.get();
        propertiesPanel = propertiesPanelUnique.get();
        
        layout();

        outliner->SetOnSelectChangedAction([this](){ updateRecords(); });
        outliner->SetOnDeleteAction([this](Primitives *deletedObject){ EraseRecord(deletedObject); });
        
        auto addObjectDropDown = std::make_unique<Outliner<Primitives *>>(ui);
        addObjectDropDown->SetSize({100, 80});
        addObjectDropDown->SetBGColor({61, 61, 61});
        addObjectDropDown->SetRecordButtonMode(Button::Mode::CAPTURE_MODE);

        addObjectDropDown->AddRecord(nullptr, "Sphere", 
            [this](){
                auto sphereMaterial = materialManager.MakeLambertian({0.0f, 0.8f, 1.0f}); 
                auto sphere = new SphereObject(1.0f, sphereMaterial, &GetSceneManager());
                AddRecord(sphere);
            }, 
            nullptr,
            static_cast<UI*>(GetUI())->GetTexturePack().outlinerSphereIconPath
        );

        addObjectDropDown->AddRecord(nullptr, "Plane", 
            [this](){
                auto planeMaterial = materialManager.MakeLambertian({0.0f, 0.8f, 1.0f}); 
                auto plane = new PlaneObject({0,0,0}, {0,0,1}, planeMaterial, &GetSceneManager());
                AddRecord(plane);
            }, 
            nullptr,
            static_cast<UI*>(GetUI())->GetTexturePack().outlinerPlaneIconPath    
        );

        addObjectDropDown->AddRecord(nullptr, "Polygon", 
            [this]() {
                auto material = materialManager.MakeLambertian({0.0f, 0.8f, 1.0f}); 
                auto polygon = new PolygonObject({{1, 0, 0}, {0, 0, 0}, {0, 0, 1}}, material, &GetSceneManager());
                AddRecord(polygon);
            }, 
            nullptr,
            static_cast<UI*>(GetUI())->GetTexturePack().outlinerPolygonIconPath
        );

        addObjectDropDown->AddRecord(nullptr, "Cube", 
            [this]() {
                auto material = materialManager.MakeLambertian({0.0f, 0.8f, 1.0f}); 
                auto cube = new CubeObject({1, 1, 1}, material, &GetSceneManager());
                AddRecord(cube);
            }, 
            nullptr,
            static_cast<UI*>(GetUI())->GetTexturePack().outlinerCubeIconPath
        );

        auto addObjectMenuUnique = std::make_unique<DropDownMenu>(ui);
        addObjectMenuUnique->SetLabel("add");
        addObjectMenuUnique->SetDropDownWidget(std::move(addObjectDropDown));

        viewport3D->AddTool(std::move(addObjectMenuUnique));
        AddWidget(std::move(viewport3DUnique));
        AddWidget(std::move(outlinerUnique));
        AddWidget(std::move(propertiesPanelUnique));
    }

    ~EditorWidget() = default;

    void AddRecord(Primitives *object) {
        assert(object);
        static size_t AddObjectIter = 0; AddObjectIter++;
        viewport3D->AddRecord(object);
        outliner->AddRecord(object, object->typeString() + std::to_string(AddObjectIter),
            [object](){ object->setSelectFlag(true); },
            [object](){ object->setSelectFlag(false); });
    }

    void EraseRecord(Primitives *deletedObject) {
        assert(deletedObject);
        viewport3D->EraseRecord(deletedObject);
    }

    void AddLight(::Light *light) {
        assert(light);
        viewport3D->AddLight(light);
    }

    void AddRecord(gm::IPoint3 position, Primitives *object) {
        assert(object);
        static size_t AddObjectIter = 0; AddObjectIter++;
        viewport3D->AddRecord(position, object);
        outliner->AddRecord(object, object->typeString() + std::to_string(AddObjectIter),
            [object](){ object->setSelectFlag(true); },
            [object](){ object->setSelectFlag(false); });
    }

    void AddLight(gm::IPoint3 position, ::Light *light) {
        assert(light);
        viewport3D->AddLight(position, light);
    }

    void ClearRecords() {
        viewport3D->ClearRecords();
        outliner->ClearRecords();
    }

    bool SerializeScene(const std::string &path) {
        std::ofstream file(path, std::ios::out | std::ios::trunc);
        if (!file) {
            return false;
        }
        for (auto primitive : viewport3D->GetPrimitives()) {
            file << *primitive << " " << *primitive->material() << "\n";
        }
        for (auto light : viewport3D->GetLights()) {
            file << *light << "\n";
        }
        return true;
    }

    bool DeserializeScene(const std::string &path) {
        std::ifstream file(path);
        if (!file) {
            return false;
        }

        std::string line;
        std::istringstream iss(line);
        
        ClearRecords();
        while (std::getline(file, line)) {
            deserializeString(line);
        }
        return true;
    }

    void deserializeString(const std::string str) {
        std::istringstream iss(str);

        std::string objectName;
        iss >> objectName;
        if (objectName == "Sphere") {
            SphereObject *sphere = new SphereObject(&viewport3D->GetSceneManager());
            iss >> *sphere;
            RTMaterial *material = materialManager.deserializeMaterial(iss);
            sphere->setMaterial(material);

            AddRecord(sphere);
            return;        
        }
        if (objectName == "Plane") {
            PlaneObject *plane = new PlaneObject(&viewport3D->GetSceneManager());
            iss >> *plane;
            RTMaterial *material = materialManager.deserializeMaterial(iss);
            plane->setMaterial(material);

            AddRecord(plane);
            return;        
        }

        if (objectName == "Polygon") {
            PolygonObject *polygon = new PolygonObject(&viewport3D->GetSceneManager());
            iss >> *polygon;
            RTMaterial *material = materialManager.deserializeMaterial(iss);
            polygon->setMaterial(material);

            AddRecord(polygon);
            return;        
        }

        if (objectName == "Cube") {
            CubeObject *cube = new CubeObject(&viewport3D->GetSceneManager());
            iss >> *cube;
            RTMaterial *material = materialManager.deserializeMaterial(iss);
            cube->setMaterial(material);

            AddRecord(cube);
            return;        
        }

         if (objectName == "Light") {
            Light *light = new Light(&viewport3D->GetSceneManager());
            iss >> *light;
            AddLight(light);
            return;        
        }

        std::cerr << "deserializeString failed. Unknown objectName : " << objectName << "\n";
    }

    std::vector<::Primitives *> &GetPrimitives() { return viewport3D->GetPrimitives(); }
    std::vector<::Light *>      &GetLights()     { return viewport3D->GetLights(); }
    SceneManager &GetSceneManager() { return viewport3D->GetSceneManager(); }

protected:
    void Redraw() const override {
        GetTexture().Clear(FULL_TRANSPARENT);
        viewport3D->DrawOn(GetTexture());        
        outliner->DrawOn(GetTexture());
        propertiesPanel->DrawOn(GetTexture());
    }

private:
    void layout() {
        float borderPadding = 30;
        float innerPadding = 3;
    
        float sumWidth = GetUI()->GetWindow()->GetSize().x - borderPadding * 2 - innerPadding;
        float viewport3DWidth = sumWidth / 3 * 2;
        float menuWidth = sumWidth - viewport3DWidth - borderPadding;
        float viewport3DHeight = GetUI()->GetWindow()->GetSize().y - borderPadding * 2;
        float menuHeight = (viewport3DHeight - innerPadding) / 2;

        viewport3D->SetSize({viewport3DWidth, viewport3DHeight});
        outliner->SetSize({menuWidth, menuHeight});
        propertiesPanel->SetSize({menuWidth, menuHeight});

        viewport3D->SetPos({borderPadding, borderPadding});
        outliner->SetPos({viewport3D->GetPos().x + viewport3D->GetSize().x + innerPadding, viewport3D->GetPos().y});
        propertiesPanel->SetPos(outliner->GetPos() + dr4::Vec2f(0, menuHeight + innerPadding));
    }

    void updateRecords() {
        auto selectedObject = outliner->GetSelected();
        propertiesPanel->ClearRecords();

        if (selectedObject.has_value()) {
            addCordsProperties(selectedObject->second);
            addMaterialProperties(selectedObject->second);
            addSpecialProperties(selectedObject->second);
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
    
        auto transformProperty = std::make_unique<roa::Property>(GetUI());
        transformProperty->SetLabel("Transform");
        transformProperty->AddPropertyField(XLabel, XContent, XCordFunction);
        transformProperty->AddPropertyField(YLabel, YContent, YCordFunction);
        transformProperty->AddPropertyField(ZLabel, ZContent, ZCordFunction);

        propertiesPanel->AddProperty(std::move(transformProperty));
    }
    void addMaterialProperties(::Primitives *selectedObject) {
        assert(selectedObject);

        auto diffuseProperty  = std::make_unique<roa::Property>(GetUI());
        auto specularProperty = std::make_unique<roa::Property>(GetUI());
        auto emittedProperty  = std::make_unique<roa::Property>(GetUI());
    
        fillSpecularProperty(selectedObject, specularProperty.get());
        fillDiffuseProperty(selectedObject, diffuseProperty.get());
        fillEmittedProperty(selectedObject, emittedProperty.get());

        propertiesPanel->AddProperty(std::move(diffuseProperty));
        propertiesPanel->AddProperty(std::move(specularProperty));
        propertiesPanel->AddProperty(std::move(emittedProperty));
    }

    void addSpecialProperties(::Primitives *selectedObject) {
        ::SphereObject *sphere = dynamic_cast<::SphereObject *>(selectedObject);
        if (sphere) {
            auto sphereProperty = std::make_unique<roa::Property>(GetUI());
            fillSphereProperty(sphere, sphereProperty.get());
            propertiesPanel->AddProperty(std::move(sphereProperty));
            return;
        }

        ::PlaneObject *plane = dynamic_cast<::PlaneObject *>(selectedObject);
        if (plane) {
            auto planeProperty = std::make_unique<roa::Property>(GetUI());
            fillPlaneProperty(plane, planeProperty.get());
            propertiesPanel->AddProperty(std::move(planeProperty));
            return;
        }

        ::CubeObject *cube = dynamic_cast<::CubeObject *>(selectedObject);
        if (cube) {
            auto cubeProperty = std::make_unique<roa::Property>(GetUI());
            fillCubeProperty(cube, cubeProperty.get());
            propertiesPanel->AddProperty(std::move(cubeProperty));
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

    void fillCubeProperty(::CubeObject *selectedCube, roa::Property *property) {
        assert(selectedCube);
        assert(property);

        std::string XLabel = "HalfSize X";
        std::string YLabel = "                   Y";
        std::string ZLabel = "                   Z";

        std::string XContent = std::to_string(selectedCube->getHalfSize().x());
        std::string YContent = std::to_string(selectedCube->getHalfSize().y());
        std::string ZContent = std::to_string(selectedCube->getHalfSize().z());

        auto setX = [selectedCube](const std::string &s){
            setIfStringConvertedToFloat(s, [selectedCube](float v){
                gm::IVec3f halfSize = selectedCube->getHalfSize();
                halfSize.setX(v);
                selectedCube->setHalfSize(halfSize);
            });
        };
        auto setY = [selectedCube](const std::string &s){
            setIfStringConvertedToFloat(s, [selectedCube](float v){
                gm::IVec3f halfSize = selectedCube->getHalfSize();
                halfSize.setY(v);
                selectedCube->setHalfSize(halfSize);
            });
        };
        auto setZ = [selectedCube](const std::string &s){
            setIfStringConvertedToFloat(s, [selectedCube](float v){
                gm::IVec3f halfSize = selectedCube->getHalfSize();
                halfSize.setZ(v);
                selectedCube->setHalfSize(halfSize);
            });
        };

        property->SetLabel("Cube properties");
        property->AddPropertyField(XLabel, XContent, setX);
        property->AddPropertyField(YLabel, YContent, setY);
        property->AddPropertyField(ZLabel, ZContent, setZ);
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

#pragma once
#include <cassert>
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <cstdlib>
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

class EditorWidget final : public Container {
    Viewport3DWindow             *viewport3D;
    OutlinerWindow<Primitives *> *outliner;
    PropertiesWindow             *propertiesPanel;
    
    RTMaterialManager materialManager;

public:
    EditorWidget(hui::UI *ui): 
        Container(ui),
        viewport3D(new Viewport3DWindow(ui)),
        outliner(new OutlinerWindow<Primitives *>(ui)),
        propertiesPanel(new PropertiesWindow(ui))
    {
        assert(ui);

        layout();

        outliner->SetOnSelectChangedAction([this](){ updateRecords(); });
        
        auto addObjectDropDown = new Outliner<Primitives *>(ui);
        addObjectDropDown->SetSize({400, 50});
        addObjectDropDown->SetBGColor({61, 61, 61});
        addObjectDropDown->SetRecordButtonMode(Button::Mode::CAPTURE_MODE);

        addObjectDropDown->AddRecord(nullptr, "S", [this](){
            auto sphereMaterial = materialManager.MakeLambertian({0.0f, 0.8f, 1.0f}); 
            auto sphere = new SphereObject(1.0f, sphereMaterial, &GetSceneManager());
            AddRecord(sphere);
        }, nullptr);

        addObjectDropDown->AddRecord(nullptr, "P", [this](){
            auto planeMaterial = materialManager.MakeLambertian({0.0f, 0.8f, 1.0f}); 
            auto plane = new PlaneObject({0,0,0}, {0,0,1}, planeMaterial, &GetSceneManager());
            AddRecord(plane);
        }, nullptr);

        auto addObjectMenu = new DropDownMenu(ui);
        addObjectMenu->SetLabel("add");
        addObjectMenu->SetDropDownWidget(addObjectDropDown);
        viewport3D->AddTool(addObjectMenu);

        AddWidget(viewport3D);
        AddWidget(outliner);
        AddWidget(propertiesPanel);
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

    std::vector<::Primitives *> &GetPrimitives() { return viewport3D->GetPrimitives(); }
    std::vector<::Light *>      &GetLights()     { return viewport3D->GetLights(); }
    SceneManager &GetSceneManager() { return viewport3D->GetSceneManager(); }

    void BringToFront(hui::Widget *) override {}

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

    void addCordsProperties(::Primitives *obj);
    void addMaterialProperties(::Primitives *obj);
    void addSpecialProperties(::Primitives *obj);

    void fillSpecularProperty(::Primitives *obj, roa::Property *prop);
    void fillDiffuseProperty(::Primitives *obj, roa::Property *prop);
    void fillEmittedProperty(::Primitives *obj, roa::Property *prop);
    void fillSphereProperty(::SphereObject *obj, roa::Property *prop);
    void fillPlaneProperty(::PlaneObject *obj, roa::Property *prop);
};

} // namespace roa

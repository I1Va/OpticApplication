#pragma once

#include "Containers.hpp"
#include "SceneWidgets.hpp"

namespace roa 
{
    
class EditorWidget : public ZContainer<hui::Widget *> {
    SceneWidget *scene = nullptr;
    ObjectsPanel<RTPrimPanelObject *> objectsPanel = nullptr;

public:
    void AddObject(gm::IPoint3 position, Primitives *object) {
        assert(object);
        scene->AddObject(position, object);

        std::unique_ptr<RTPrimPanelObject *object = new 
        objectsPanel->AddObject();

    }

    void AddLight(gm::IPoint3 position, Light *light) {
        assert(light);
        scene->AddLight(position, light);
    }
};

} // namespace roa

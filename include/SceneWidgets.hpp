#pragma once

#include <chrono>
#include <cmath>

#include "hui/widget.hpp"
#include "RayTracer.h"
#include "Camera.h"

namespace roa
{

// inline SDL_Color convertRTPixelColor(const RTPixelColor color) { return {color.r, color.g, color.b, color.a}; }

// inline void setIfStringConvertedToFloat(const std::string &inp, std::function<void(float)> setter) {
//     char* end = nullptr;
//     float val = std::strtof(inp.c_str(), &end);
//     if (*end == '\0' && end != inp.c_str()) {
//         setter(val);
//     }
// }


// class ObjectListComponent : public Container {
//     static constexpr int BORDER_THIKNESS = 3;
//     static constexpr SDL_Color BORDER_COLOR = BLACK_SDL_COLOR;
//     static constexpr SDL_Color BACK_COLOR = WHITE_SDL_COLOR;

//     static constexpr SDL_Color TEXT_COLOR = BLACK_SDL_COLOR;
//     static constexpr std::size_t OBJECT_RECORD_HEIGHT = 20;
//     TTF_Font* font_ = nullptr;

//     std::vector<Primitives *> objects_{};


//     std::function<void(Primitives *)> onSelect_;
//     std::function<void(Primitives *)> onUnSelect_;

//     Primitives *selectedObject_ = nullptr;

//     bool objectListChanged = true;

// public:
//     ObjectListComponent(int width, int height, TTF_Font* font,
//         std::function<void(Primitives *)> onSelect=nullptr,
//         std::function<void(Primitives *)> onUnSelect=nullptr,
//         Widget *parent=nullptr
//     ):
//         Container(width, height, parent), font_(font),
//         onSelect_(onSelect), onUnSelect_(onUnSelect)
//     {}
    
//     void setObjects(const std::vector<Primitives *> objects) { objects_ = objects; }

//     void unselectObject(Primitives *primitive) {
//         primitive->setSelectFlag(false);
//         if (onUnSelect_) onUnSelect_(primitive);
//     }

//     void selectObject(Primitives *primitive) {

//         selectedObject_ = primitive;
//         selectedObject_->setSelectFlag(true);
//         if (onSelect_) onSelect_(primitive);
//     }

//     void updateRecords() {
//          for (Widget *child : children_) {
//             assert(child);
        
//             if (UIManager_->hovered() == child) UIManager_->setHovered(nullptr);
//             if (UIManager_->mouseActived() == child) UIManager_->setMouseActived(nullptr);
            
//             delete child;
//         }
//         children_.clear();

//         for (size_t i = 0; i < objects_.size(); i++) {
//             Primitives *primitive = objects_[i];
        
//             std::string recordName = primitive->typeString() + " " + std::to_string(i);
            
//             ClickableTextWidget *objectRecord = new ClickableTextWidget(rect_.w - 2 * BORDER_THIKNESS, OBJECT_RECORD_HEIGHT, recordName, font_, 
//                                                                         [this, primitive]() { selectObject(primitive); },
//                                                                         [this, primitive]() { unselectObject(primitive); },
//                                                                         this);
                                                                    
//             addWidget(BORDER_THIKNESS, BORDER_THIKNESS + i * OBJECT_RECORD_HEIGHT, objectRecord);
//         }

//         setRerenderFlag();
//         objectListChanged = false;
//     }

//     bool updateSelfAction() override {
//         if (objectListChanged) updateRecords();       

//         return true; 
//     }

//     void renderSelfAction(SDL_Renderer* renderer) override { 

//         SDL_Rect frame = {0, 0, rect_.w, rect_.h};
//         SDL_SetRenderDrawColor(renderer, BORDER_COLOR.r, BORDER_COLOR.g, BORDER_COLOR.b, BORDER_COLOR.a);
//         SDL_RenderFillRect(renderer, &frame);
    
//         SDL_SetRenderDrawColor(renderer, BACK_COLOR.r, BACK_COLOR.g, BACK_COLOR.b, BACK_COLOR.a);
//         SDL_Rect interior = {BORDER_THIKNESS, BORDER_THIKNESS, rect_.w - 2 * BORDER_THIKNESS, rect_.h - 2 * BORDER_THIKNESS};
//         SDL_RenderFillRect(renderer, &interior);                
//     }

//     Primitives *selectedObject() { return selectedObject_; }
// };

// class ObjectPropertiesComponent : public Container {
//     static constexpr int BORDER_THIKNESS = 3;
//     static constexpr SDL_Color BORDER_COLOR = BLACK_SDL_COLOR;
//     static constexpr SDL_Color BACK_COLOR = WHITE_SDL_COLOR;

//     static constexpr SDL_Color TEXT_COLOR = BLACK_SDL_COLOR;
//     static constexpr std::size_t OBJECT_RECORD_HEIGHT = 20;
//     TTF_Font* font_ = nullptr;

//     Primitives *selected_=nullptr;
//     bool needUpdateRecords = false;

// public:
//     ObjectPropertiesComponent(int width, int height, TTF_Font* font, Widget *parent=nullptr):
//         Container(width, height, parent), font_(font) {}
    
//     void selectObject(Primitives *object) { 
//         selected_ = object; 
//         needUpdateRecords = true;
//     }

//     void updateRecords() {
//         setRerenderFlag();
//         needUpdateRecords = false;
    
//         for (Widget *child : children_) {
//             assert(child);
        
//             if (UIManager_->hovered() == child) UIManager_->setHovered(nullptr);
//             if (UIManager_->mouseActived() == child) UIManager_->setMouseActived(nullptr);
            
//             delete child;
//         }
    
//         children_.clear();

//         if (!selected_) return;


//         int startX = BORDER_THIKNESS;
//         int startY = BORDER_THIKNESS;
//         int recordWidth = rect_.w / 2 - BORDER_THIKNESS * 2;
//         int recordHeight = OBJECT_RECORD_HEIGHT;
//         std::function<void(float)> setter = nullptr;

//         auto addRecord = [&](const std::string &label, const std::string &value,
//                         std::function<void(const std::string&)> callback, int row)
//         {
//             TextWidget *lbl = new TextWidget(recordWidth, recordHeight, label, font_, this);
//             Widget *field;

//             if (callback)
//                 field = new TextInputWidget(recordWidth, recordHeight, value, font_, callback, this);
//             else
//                 field = new TextWidget(recordWidth, recordHeight, value, font_, this);

//             addWidget(startX, startY + recordHeight * row, lbl);
//             addWidget(startX + recordWidth, startY + recordHeight * row, field);
//         };

        
//         // usage
//         addRecord("Name", selected_->typeString(), nullptr, 0);
//         addRecord("Material", selected_->material()->typeString(), nullptr, 1);

//         // position fields
//         addRecord("Pos X", std::to_string(selected_->position().x()),
//                 [this](const std::string &inp) {
//                     setIfStringConvertedToFloat(inp, [this](float val) { selected_->position().setX(val); });
//                 }, 2);

//         addRecord("Pos Y", std::to_string(selected_->position().y()),
//                 [this](const std::string &inp) {
//                     setIfStringConvertedToFloat(inp, [this](float val) { selected_->position().setY(val); });
//                 }, 3);

//         addRecord("Pos Z", std::to_string(selected_->position().z()),
//                 [this](const std::string &inp) {
//                     setIfStringConvertedToFloat(inp, [this](float val) { selected_->position().setZ(val); });
//                 }, 4);

//         // diffuse fields
//         addRecord("Diffuse X", std::to_string(selected_->material()->diffuse().x()),
//             [this](const std::string &inp) {
//                 setIfStringConvertedToFloat(inp, [this](float val) {
//                     selected_->material()->diffuse().setX(val);
//                 });
//             }, 5);
        
//         addRecord("Diffuse Y", std::to_string(selected_->material()->diffuse().y()),
//             [this](const std::string &inp) {
//                 setIfStringConvertedToFloat(inp, [this](float val) {
//                     selected_->material()->diffuse().setY(val);
//                 });
//             }, 6);

//         addRecord("Diffuse Z", std::to_string(selected_->material()->diffuse().z()),
//             [this](const std::string &inp) {
//                 setIfStringConvertedToFloat(inp, [this](float val) {
//                     selected_->material()->diffuse().setZ(val);
//                 });
//             }, 7);

//         // Specular fields
//         addRecord("Specular X", std::to_string(selected_->material()->specular().x()),
//             [this](const std::string &inp) {
//                 setIfStringConvertedToFloat(inp, [this](float val) {
//                     selected_->material()->specular().setX(val);
//                 });
//             }, 8);

//         addRecord("Specular Y", std::to_string(selected_->material()->specular().y()),
//             [this](const std::string &inp) {
//                 setIfStringConvertedToFloat(inp, [this](float val) {
//                     selected_->material()->specular().setY(val);
//                 });
//             }, 9);

//         addRecord("Specular Z", std::to_string(selected_->material()->specular().z()),
//             [this](const std::string &inp) {
//                 setIfStringConvertedToFloat(inp, [this](float val) {
//                     selected_->material()->specular().setZ(val);
//                 });
//             }, 10);

//         // Emitted fields
//         addRecord("Emitted X", std::to_string(selected_->material()->emitted().x()),
//             [this](const std::string &inp) {
//                 setIfStringConvertedToFloat(inp, [this](float val) {
//                     selected_->material()->emitted().setX(val);
//                 });
//             }, 11);

//         addRecord("Emitted Y", std::to_string(selected_->material()->emitted().y()),
//             [this](const std::string &inp) {
//                 setIfStringConvertedToFloat(inp, [this](float val) {
//                     selected_->material()->emitted().setY(val);
//                 });
//             }, 12);

//         addRecord("Emitted Z", std::to_string(selected_->material()->emitted().z()),
//             [this](const std::string &inp) {
//                 setIfStringConvertedToFloat(inp, [this](float val) {
//                     selected_->material()->emitted().setZ(val);
//                 });
//             }, 13);
//     }

//     bool updateSelfAction() override {
//         if (needUpdateRecords) updateRecords();       

//         return true; 
//     }

//     void renderSelfAction(SDL_Renderer* renderer) override { 

//         SDL_Rect frame = {0, 0, rect_.w, rect_.h};
//         SDL_SetRenderDrawColor(renderer, BORDER_COLOR.r, BORDER_COLOR.g, BORDER_COLOR.b, BORDER_COLOR.a);
//         SDL_RenderFillRect(renderer, &frame);
    
//         SDL_SetRenderDrawColor(renderer, BACK_COLOR.r, BACK_COLOR.g, BACK_COLOR.b, BACK_COLOR.a);
//         SDL_Rect interior = {BORDER_THIKNESS, BORDER_THIKNESS, rect_.w - 2 * BORDER_THIKNESS, rect_.h - 2 * BORDER_THIKNESS};
//         SDL_RenderFillRect(renderer, &interior);                
//     }
// };

class SceneWidget : public hui::Widget {
    static inline constexpr int CAMERA_KEY_CONTROL_DELTA = 10;
    static inline constexpr int CAMERA_MOUSE_RELOCATION_SCALE = 2;

    std::unique_ptr<dr4::Image> sceneImage;
    SceneManager sceneManager;
    Camera camera;

public:
    SceneWidget(hui::UI *ui): 
        hui::Widget(ui), 
        sceneImage(ui->GetWindow()->CreateImage())
    { 
        camera.setCenter({0, -6, 1});
        camera.setDirection({0, 3, 0});

        camera.renderProperties.samplesPerScatter = 1;
        camera.renderProperties.samplesPerPixel = 1;
        camera.renderProperties.enableLDirect = false;
        camera.renderProperties.maxRayDepth = 5;
    }

    void addObject(gm::IPoint3 position, Primitives *object) { sceneManager.addObject(position, object); }
    void addLight(gm::IPoint3 position, Light *light) { sceneManager.addLight(position, light); }

    Camera &getCamera() { return camera; }
    SceneManager &getSceneManager() { return sceneManager; }

protected:
    hui::EventResult OnIdle(hui::IdleEvent &) override {
        static std::vector<RTPixelColor> tempBufer;

        std::pair<int, int> screenResolution = {};
        screenResolution.first  = static_cast<int>(sceneImage->GetWidth());
        screenResolution.second = static_cast<int>(sceneImage->GetHeight());
        tempBufer.resize(screenResolution.first * screenResolution.second);       

        camera.render(sceneManager, screenResolution, tempBufer);

        for (int pixelX = 0; pixelX < screenResolution.first; pixelX++) {
            for (int pixelY = 0; pixelY < screenResolution.second; pixelY++) {
                int pixelId = pixelY * screenResolution.first + pixelX;

                dr4::Color pixelCOlor = 
                {
                    tempBufer[pixelId].r,
                    tempBufer[pixelId].g,
                    tempBufer[pixelId].b,
                    tempBufer[pixelId].a
                };

                sceneImage->SetPixel
                (
                    pixelX, 
                    pixelY, 
                    pixelCOlor
                );
            }
        }

        return hui::EventResult::UNHANDLED;
    }

    void OnSizeChanged() { 
        sceneImage->SetSize(GetSize());
    }

    void Redraw() const override {
        sceneImage->DrawOn(GetTexture());

        // if (actived) GetTexture().Clear(pressedColor);
        // else GetTexture().Clear(unpressedColor);
    }

    // void renderSelfAction(SDL_Renderer* renderer) override { 
        
    // }


    // double measureRenderTime(const std::size_t MEASURE_COUNT=1) {
    //     double duration = 0;
    //     for (std::size_t i = 0; i < MEASURE_COUNT; i++) {
    //         auto start = std::chrono::high_resolution_clock::now();
    //         sceneManager_.render(camera_);
    //         auto end = std::chrono::high_resolution_clock::now();  
        
    //         duration += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    //     }

    //     return duration / MEASURE_COUNT;
    // }

};


}
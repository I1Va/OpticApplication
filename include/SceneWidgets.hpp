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

class IPanelObject {
public:
    virtual ~IPanelObject() {};
    virtual const std::string &GetName() = 0;
    virtual void OnSelect() = 0;
    virtual void OnUnSelect() = 0;
};

template<typename P>
concept PointerToPanelObject =
    std::is_pointer_v<P> &&
    std::derived_from<std::remove_pointer_t<P>, IPanelObject>;

class RTPrimPanelObject : public IPanelObject {
    Primitives *object;
public:
    RTPrimPanelObject(Primitives *obj): object(obj) { assert(obj); }
    ~RTPrimPanelObject() = default;
    void OnSelect() override { object->setSelectFlag(true); }
    void OnUnSelect() override { object->setSelectFlag(false); }   
};


template <PointerToPanelObject T>
class ObjectPanel : public LinContainer<TextButton *> {
    static constexpr double RECORD_HW_LAYOUT_SHARE = 0.3; 

    T currentSelected = nullptr; 
public:
    using LinContainer::LinContainer;

    void AddObject(T object) {
        TextButton *record = new TextButton(GetUI());
        record->SetText(object->GetName());
    
        record->SetOnClickAction([this, object]() 
            { 
                object->OnSelect(); 
                currentSelected = object; 
            } 
        );

        record->SetOnUnpressAction([this, object]() 
            { 
                object->OnUnSelect(); 
                if (currentSelected == object) currentSelected = nullptr;
            } 
        );

        addWidget(record);
    }

    T GetSelected() { return currentSelected; }
 
protected:
    void Redraw() const override {
        GetTexture().Clear({255, 132, 0, 255});
        for (TextButton *record : children) {
            record->DrawOn(GetTexture());
        }        
    }
    
    void OnSizeChanged() { relayout(); }

private:
    void relayout() {
        float recordHeight = RECORD_HW_LAYOUT_SHARE * GetSize().x;
        float recordWidth = GetSize().x;
        
        dr4::Vec2f curRecordPos = dr4::Vec2f(0, 0);
        for (TextButton *record : children) {
            record->SetPos(curRecordPos);
            record->SetSize({recordWidth, recordHeight});
            curRecordPos += dr4::Vec2f(0, recordHeight);
        }
    }
};

















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
    static constexpr double CAMERA_ZOOM_DELTA = 0.1;

    std::unique_ptr<dr4::Image> sceneImage;
    SceneManager sceneManager;

    Camera camera;
    bool mouseMiddleKeyPressed  = false;
    bool mouseLeftKeyPressed    = false;

    dr4::Vec2f accumulatedCameraRotation = {0, 0};
    bool       cameraNeedRotation        = false;

    dr4::Vec2f accumulatedCameraRel      = {0, 0};
    bool       cameraNeedRelocation      = false;

    int        accumulatedCameraZoom     = 0;
    bool       cameraNeedZoom            = false;

public:
    SceneWidget(hui::UI *ui): 
        hui::Widget(ui), 
        sceneImage(ui->GetWindow()->CreateImage())
    { 
        camera.setCenter({0, -6, 1});
        camera.setDirection({0, 3, 0});

        camera.renderProperties.samplesPerScatter = 1;
        camera.renderProperties.samplesPerPixel = 1;
        camera.renderProperties.enableLDirect = true;
        camera.renderProperties.maxRayDepth = 5;
    }

    double MeasureRenderTime(const std::size_t MEASURE_COUNT=1) {
        static std::vector<RTPixelColor> tempBufer;
        std::pair<int, int> screenResolution = {};
        screenResolution.first  = static_cast<int>(sceneImage->GetWidth());
        screenResolution.second = static_cast<int>(sceneImage->GetHeight());
        tempBufer.resize(screenResolution.first * screenResolution.second);       
    
        double duration = 0;
        for (std::size_t i = 0; i < MEASURE_COUNT; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            camera.render(sceneManager, screenResolution, tempBufer);
            auto end = std::chrono::high_resolution_clock::now();  
        
            duration += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        }

        return duration / MEASURE_COUNT;
    }

    void AddObject(gm::IPoint3 position, Primitives *object) { sceneManager.addObject(position, object); }
    void AddLight(gm::IPoint3 position, Light *light) { sceneManager.addLight(position, light); }

    std::vector<::Primitives *> &Primitives() { return sceneManager.primitives(); }
    std::vector<::Light *> &Lights() { return sceneManager.lights(); }

    int GetScreenResolutionWidth()  const { return sceneImage->GetWidth();  }
    int GetScreenResolutionHeight() const { return sceneImage->GetHeight(); }

    Camera &GetCamera() { return camera; }
    SceneManager &GetSceneManager() { return sceneManager; }


protected:

    hui::EventResult OnMouseDown(hui::MouseButtonEvent &event) override { 
        if (!GetRect().Contains(event.pos)) return hui::EventResult::UNHANDLED;

        switch (event.button)
        {
            case dr4::MouseButtonType::MIDDLE:
                mouseMiddleKeyPressed = true;
                break;
            case dr4::MouseButtonType::LEFT:
                mouseLeftKeyPressed = true;
                break;
            default:
                break;
        }
        
        GetUI()->ReportFocus(this);
        GetUI()->SetCaptured(this);
        return hui::EventResult::HANDLED;
    }

    hui::EventResult OnMouseUp (hui::MouseButtonEvent &event) override { 
        if (!GetRect().Contains(event.pos) && (GetUI()->GetCaptured() != this)) return hui::EventResult::UNHANDLED;
        switch (event.button)
        {
            case dr4::MouseButtonType::MIDDLE:
                mouseMiddleKeyPressed = false;
                break;
            case dr4::MouseButtonType::LEFT:
                mouseLeftKeyPressed = false;
                break;
            default:
                break;
        }
        GetUI()->SetCaptured(nullptr);
        
        return hui::EventResult::HANDLED;
    }

    hui::EventResult OnIdle(hui::IdleEvent &) override {
        static std::vector<RTPixelColor> tempBufer;

        std::pair<int, int> screenResolution = {};
        screenResolution.first  = static_cast<int>(sceneImage->GetWidth());
        screenResolution.second = static_cast<int>(sceneImage->GetHeight());
        tempBufer.resize(screenResolution.first * screenResolution.second); 
        
        if (cameraNeedRotation  ) applyCameraRotation();
        if (cameraNeedRelocation) applyCameraRelocation();
        if (cameraNeedZoom)       applyCameraZoom();
        
        camera.render(sceneManager, screenResolution, tempBufer);
        ForceRedraw();

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

    hui::EventResult OnKeyDown(hui::KeyEvent &event) override {
        if (GetUI()->GetFocused() != this) return hui::EventResult::UNHANDLED;

        switch (event.key) {
            case dr4::KeyCode::KEYCODE_LEFT:
                accumulatedCameraRel += dr4::Vec2f(CAMERA_KEY_CONTROL_DELTA, 0);
                cameraNeedRelocation = true;
                return hui::EventResult::HANDLED;
            
            case dr4::KeyCode::KEYCODE_RIGHT:
                accumulatedCameraRel += dr4::Vec2f(-CAMERA_KEY_CONTROL_DELTA, 0);
                cameraNeedRelocation = true;
                return hui::EventResult::HANDLED;

            case dr4::KeyCode::KEYCODE_UP: 
                accumulatedCameraRel += dr4::Vec2f(0, CAMERA_KEY_CONTROL_DELTA);
                cameraNeedRelocation = true;
                return hui::EventResult::HANDLED;
            
            case dr4::KeyCode::KEYCODE_DOWN: 
                accumulatedCameraRel += dr4::Vec2f(0, -CAMERA_KEY_CONTROL_DELTA);
                cameraNeedRelocation = true;
                return hui::EventResult::HANDLED;
            default:
                break;
        }

        return hui::EventResult::UNHANDLED;
    }

    hui::EventResult OnMouseMove(hui::MouseMoveEvent &event) {
        if (mouseMiddleKeyPressed) {
            accumulatedCameraRotation += event.rel;
            cameraNeedRotation = true;
            return hui::EventResult::HANDLED;
        } 
        if (mouseLeftKeyPressed) {
            accumulatedCameraRel += event.rel * CAMERA_MOUSE_RELOCATION_SCALE;
            cameraNeedRelocation = true;
            return hui::EventResult::HANDLED;
        }

        return hui::EventResult::UNHANDLED;
    }

    hui::EventResult OnMouseWheel(hui::MouseWheelEvent &event) override {
        accumulatedCameraZoom += event.delta.y;
        cameraNeedZoom = true;
        return hui::EventResult::HANDLED;
    }

    void applyCameraRelocation() {
        double dx = (double) accumulatedCameraRel.x / GetScreenResolutionWidth()  * camera.viewPort().VIEWPORT_WIDTH;
        double dy = (double) accumulatedCameraRel.y / GetScreenResolutionHeight() * camera.viewPort().VIEWPORT_HEIGHT;

        gm::IVec3f motionVec = camera.viewPort().rightDir_ * dx + camera.viewPort().downDir_ * dy;
        camera.move(motionVec * (-1));
    
        accumulatedCameraRel = {0, 0};
        cameraNeedRelocation = false;
    }

    void applyCameraRotation() {
        double widthRadians  = (double) accumulatedCameraRotation.x / GetScreenResolutionWidth()  * camera.viewPort().viewAngle_.x();
        double heightRadians = (double) accumulatedCameraRotation.y / GetScreenResolutionHeight() * camera.viewPort().viewAngle_.y();
        camera.rotate(-widthRadians, -heightRadians);
    
        accumulatedCameraRotation = {0, 0};
        cameraNeedRotation = false;
    }

    void applyCameraZoom() {
        gm::IVec3f zoomVec = camera.direction() * CAMERA_ZOOM_DELTA * accumulatedCameraZoom;
        camera.move(zoomVec);

        accumulatedCameraZoom = 0;
        cameraNeedZoom = false;
    }


};


}
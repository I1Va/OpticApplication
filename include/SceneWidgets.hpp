#pragma once

#include <chrono>
#include <cmath>

#include "hui/widget.hpp"
#include "Camera.h"
#include "RayTracer.h"

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

    void AddObject(gm::IPoint3 position, Primitives *object) { sceneManager.addObject(position, object); }
    void AddLight(gm::IPoint3 position, Light *light) { sceneManager.addLight(position, light); }

    std::vector<::Primitives *> &Primitives() { return sceneManager.primitives(); }
    std::vector<::Light *>      &Lights()     { return sceneManager.lights(); }

    int GetScreenResolutionWidth()  const { return sceneImage->GetWidth();  }
    int GetScreenResolutionHeight() const { return sceneImage->GetHeight(); }

    Camera &GetCamera() { return camera; }
    SceneManager &GetSceneManager() { return sceneManager; }

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
        if (GetUI()->GetFocused() != this) return hui::EventResult::UNHANDLED;

        accumulatedCameraZoom += event.delta.y;
        cameraNeedZoom = true;
        return hui::EventResult::HANDLED;
    }

    void Redraw() const override {
        sceneImage->DrawOn(GetTexture());
    }

    void OnSizeChanged() override { 
        sceneImage->SetSize(GetSize());
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
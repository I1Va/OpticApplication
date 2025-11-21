#include <utility>
#include <chrono>
#include <dlfcn.h>
#include <cstdint>
#include <functional>
#include <chrono>
#include <thread>

#include <SDL2/SDL.h>

#include "RayTracer.h"
#include "Camera.h"
#include "hui/ui.hpp"
#include "dr4/event.hpp"
#include "dr4/mouse_buttons.hpp"
#include "misc/dr4_ifc.hpp"
#include "hui/container.hpp"
#include "hui/event.hpp"


const char PLAGIN_PATH[] = "external/libAIPlugin.so";
const char FONT_PATH[] = "Roboto/RobotoFont.ttf";
typedef dr4::DR4Backend* (*DR4BackendFunction)();
const std::pair<int, int> MAIN_WINDOW_SIZE = {600, 600};
const int APP_BORDER_SIZE = 20;
const int CAMERA_KEY_CONTROL_DELTA = 10;
const int CAMERA_MOUSE_RELOCATION_SCALE = 2;
const std::pair<int, int> SCREEN_RESOLUTION = {600, 600};

namespace roa
{

inline dr4::Color convertRTPixelColor(const RTPixelColor color) { return {color.r, color.g, color.b, color.a}; }

class CameraWindow : public hui::Widget {
    static constexpr double CAMERA_ZOOM_DELTA = 0.1;

    Camera *camera_ = nullptr;

    gm::IVec2f   accumulatedCameraRotation_ = {0, 0};
    bool           cameraNeedRotation_        = false;

    gm::IVec2f   accumulatedCameraRel_      = {0, 0};
    bool           cameraNeedRelocation_      = false;

    int            accumulatedCameraZoom_     = 0;
    bool           cameraNeedZoom_            = false;

    dr4::Image *image = nullptr;


    bool middleMouseKeyPressed = false;
    bool leftMouseKeyPressed = false;

  
    hui::EventResult OnKeyDown(hui::KeyEvent &event) override {
        switch (event.key) {
            case dr4::KeyCode::KEYCODE_A:    
                accumulatedCameraRotation_ += gm::IVec2f (-CAMERA_KEY_CONTROL_DELTA, 0);
                cameraNeedRotation_ = true;
                return hui::EventResult::HANDLED;

            case dr4::KeyCode::KEYCODE_D:
                accumulatedCameraRotation_ += gm::IVec2f (CAMERA_KEY_CONTROL_DELTA, 0);
                cameraNeedRotation_ = true;
                return hui::EventResult::HANDLED;

            case dr4::KeyCode::KEYCODE_W: 
                accumulatedCameraRotation_ += gm::IVec2f (0, -CAMERA_KEY_CONTROL_DELTA);
                cameraNeedRotation_ = true;
                return hui::EventResult::HANDLED;
            
            case dr4::KeyCode::KEYCODE_S:
                accumulatedCameraRotation_ += gm::IVec2f (0, CAMERA_KEY_CONTROL_DELTA);
                cameraNeedRotation_ = true;
                return hui::EventResult::HANDLED;
                
            case dr4::KeyCode::KEYCODE_LEFT:
                accumulatedCameraRel_ += gm::IVec2f (CAMERA_KEY_CONTROL_DELTA, 0);
                cameraNeedRelocation_ = true;
                return hui::EventResult::HANDLED;
            
            case dr4::KeyCode::KEYCODE_RIGHT:    
                accumulatedCameraRel_ += gm::IVec2f (-CAMERA_KEY_CONTROL_DELTA, 0);
                cameraNeedRelocation_ = true;
                return hui::EventResult::HANDLED;

            case dr4::KeyCode::KEYCODE_UP: 
                accumulatedCameraRel_ += gm::IVec2f (0, CAMERA_KEY_CONTROL_DELTA);
                cameraNeedRelocation_ = true;
                return hui::EventResult::HANDLED;
            
            case dr4::KeyCode::KEYCODE_DOWN:
                accumulatedCameraRel_ += gm::IVec2f (0, -CAMERA_KEY_CONTROL_DELTA);
                cameraNeedRelocation_ = true;
                return hui::EventResult::HANDLED;
            default:
                return hui::EventResult::UNHANDLED;
        }

        return hui::EventResult::UNHANDLED;;
    }

    hui::EventResult OnMouseDown(hui::MouseButtonEvent &event) override {
        GetUI()->ReportFocus(this);
    
        if (event.button == dr4::MouseButtonType::LEFT) leftMouseKeyPressed = event.pressed;
        if (event.button == dr4::MouseButtonType::MIDDLE) middleMouseKeyPressed = event.pressed;

        return hui::EventResult::HANDLED;
    } 

    hui::EventResult OnMouseUp(hui::MouseButtonEvent &event) override {
        if (event.button == dr4::MouseButtonType::LEFT) leftMouseKeyPressed = event.pressed;
        if (event.button == dr4::MouseButtonType::MIDDLE) middleMouseKeyPressed = event.pressed;

        return hui::EventResult::HANDLED;
    }

    hui::EventResult OnMouseWheel(hui::MouseWheelEvent &evt) {
        accumulatedCameraZoom_ += evt.delta.y;
        cameraNeedZoom_ = true;
        return hui::EventResult::HANDLED;
    }

    hui::EventResult OnMouseMove(hui::MouseMoveEvent &evt) override {
        GetUI()->ReportHover(this);
        if (middleMouseKeyPressed) {
            accumulatedCameraRotation_ += gm::IVec2f(evt.rel.x, evt.rel.y);
            cameraNeedRotation_ = true;
            return hui::EventResult::HANDLED;
        }

        if (leftMouseKeyPressed) {
            dr4::Vec2f delta = evt.rel * CAMERA_MOUSE_RELOCATION_SCALE;
            accumulatedCameraRel_ += gm::IVec2f(delta.x, delta.y);
            cameraNeedRelocation_ = true;
            return hui::EventResult::HANDLED;
        }
        return hui::EventResult::UNHANDLED;;
    }
    
    void applyCameraRelocation() {
        double dx = (double) accumulatedCameraRel_.x() / camera_->screenResolution().first * camera_->viewPort().VIEWPORT_WIDTH;
        double dy = (double) accumulatedCameraRel_.y() / camera_->screenResolution().second * camera_->viewPort().VIEWPORT_HEIGHT;

        gm::IVec3f motionVec = camera_->viewPort().rightDir_ * dx + camera_->viewPort().downDir_ * dy;
        camera_->move(motionVec * (-1));
    
        accumulatedCameraRel_ = {0, 0};
        cameraNeedRelocation_ = false;
    }

    void applyCameraRotation() {
        double widthRadians  = (double) accumulatedCameraRotation_.x() / camera_->screenResolution().first * camera_->viewAngle().x();
        double heightRadians = (double) accumulatedCameraRotation_.y() / camera_->screenResolution().second * camera_->viewAngle().y();
        camera_->rotate(-widthRadians, -heightRadians);
    
        accumulatedCameraRotation_ = {0, 0};
        cameraNeedRotation_ = false;
    }

    void applyCameraZoom() {
        gm::IVec3f zoomVec = camera_->direction() * CAMERA_ZOOM_DELTA * accumulatedCameraZoom_;
        camera_->move(zoomVec);

        accumulatedCameraZoom_ = 0;
        cameraNeedZoom_ = false;
    }

      hui::EventResult OnIdle([[maybe_unused]] hui::IdleEvent &evt) override {
        if (cameraNeedRotation_  ) applyCameraRotation();
        if (cameraNeedRelocation_) applyCameraRelocation();
        if (cameraNeedZoom_)       applyCameraZoom();
        
        ForceRedraw();

        return hui::EventResult::HANDLED;
    }

    void updateTexture(int width, int height) const
    {
        image->SetSize({width, height});
    
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                dr4::Color color = convertRTPixelColor(camera_->getPixel(i, j));
                image->SetPixel(i, j, color);
            }
        }

        image->DrawOn(GetTexture());
    }

    void Redraw() const override {
        if (!camera_) return;
        updateTexture(camera_->screenResolution().first, camera_->screenResolution().second);
    }



public:
    void setCamera(Camera *camera) { 
        assert(camera);
        camera_ = camera; 
    }

    CameraWindow(hui::UI *ui): hui::Widget(ui), image(ui->GetWindow()->CreateImage()) {}
};

double measureRenderTime(SceneManager &sceneManager, Camera &camera) {
    const std::size_t MEASURE_COUNT = 100;
    
    double duration = 0;
    for (std::size_t i = 0; i < MEASURE_COUNT; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        sceneManager.render(camera);
        auto end = std::chrono::high_resolution_clock::now();  
     
        duration += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    }

    return duration / MEASURE_COUNT;
}

class LinContainer : public hui::Container {
    std::vector<hui::Widget *> children_; 
public:
    LinContainer(hui::UI *ui): hui::Container(ui) {}
    hui::EventResult PropagateToChildren(hui::Event &event) override {
        for (Widget *child : children_) {

            if (event.Apply(*child) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
            
        }
        return hui::EventResult::UNHANDLED;
    }

    void addWidget(Widget *widget) {
        BecomeParentOf(widget);
        children_.push_back(widget);
    }

    void Redraw() const override { 
        GetTexture().Clear({
            uint8_t(int(GetSize().x) % 255),
            uint8_t(int(GetSize().y) % 255),
            0,
            255});
        
        for (Widget *child : children_) {
            child->GetFreshTexture().DrawOn(GetTexture());
        }
        
    }
};

}

int main() {
    void *backendLib = dlopen(PLAGIN_PATH, RTLD_LAZY); assert(backendLib);
    DR4BackendFunction func = (DR4BackendFunction) dlsym(backendLib, dr4::DR4BackendFunctionName); assert(func);
    dr4::DR4Backend *backend = func(); assert(backend);
    
    dr4::Window *window = backend->CreateWindow(); assert(window);

    window->SetSize({float(MAIN_WINDOW_SIZE.first), float(MAIN_WINDOW_SIZE.second)});
    window->StartTextInput();
    window->Open();
    window->Display();

    hui::UI ui(window);

    roa::LinContainer *mainWindow = new roa::LinContainer(&ui);
    mainWindow->SetPos(0, 0);
    mainWindow->SetSize(500.0f, 500.0f);
    ui.SetRoot(mainWindow);


        
    
    SceneManager sceneManager;
    RTMaterial *groundMaterial = new RTLambertian({0.8, 0.8, 0.0});    
    RTMaterial *midSphereMaterial = new RTLambertian({0.1, 0.2, 0.5});
    RTMaterial *rightSphereMaterial = new RTMetal({0.8, 0.8, 0.8}, 0.3);
    RTMaterial *glassMaterial = new RTDielectric({1.0, 1.0, 1.0}, 1.50);
    RTMaterial *sunMaterial = new RTEmissive(gm::IVec3f(1.0, 0.95, 0.9) * 10);
    // SphereObject *light = new SphereObject(1, lightSrcMaterial, &sceneManager);

    SphereObject *sun = new SphereObject(1, sunMaterial, &sceneManager);

    Light *light = new Light
    (
        /* ambientIntensity  */  gm::IVec3f(0.2, 0.2, 0.2),
        /* defuseIntensity   */  gm::IVec3f(0.8, 0.7, 0.6),
        /* specularIntensity */  gm::IVec3f(0.7, 0.7, 0),
        /* viewLightPow      */  15.0
    );

    SphereObject    *midSphere = new SphereObject(1, midSphereMaterial, &sceneManager);
    SphereObject    *rightSphere = new SphereObject(1, rightSphereMaterial, &sceneManager);
  
    PlaneObject     *ground = new PlaneObject({0, 0, 0}, {0, 0, 1}, groundMaterial, &sceneManager);



    SphereObject    *glassSphere = new SphereObject(1, glassMaterial, &sceneManager);

    
    sceneManager.addObject({0, 0, -100}, ground);
    sceneManager.addObject({0, 0, 1}, glassSphere);
    // // sceneManager.addObject({-2, 0, 1}, leftBubbleSphere);
    sceneManager.addObject({0, 4, 3}, midSphere);
    sceneManager.addObject({2, 0, 1}, rightSphere);

    sceneManager.addLight({0, 0, 10}, light);
    sceneManager.addObject({-2, 0, 4}, sun);

    Camera camera(/*center*/{0, -6, 1}, /*direction*/{0, 3, 0}, SCREEN_RESOLUTION);
    camera.setSamplesPerPixel(1);
    camera.setSamplesPerScatter(1);
    // camera.disableLDirect();
    camera.setMaxRayDepth(5);
    camera.setThreadPixelbunchSize(100);
    camera.disableParallelRender();



    

    roa::CameraWindow *cameraWindow = new roa::CameraWindow(&ui);
    cameraWindow->SetSize({500, 500});
    cameraWindow->setCamera(&camera);

    mainWindow->addWidget(cameraWindow);

   
    // roa::LinContainer *c1 = new roa::LinContainer(&ui);
    // c1->SetPos(50, 50);
    // c1->SetSize(400, 400);
    // mainWindow->addWidget(c1);

    // roa::LinContainer *c2 = new roa::LinContainer(&ui);
    // c2->SetPos(50, 50);
    // c2->SetSize(300, 300);
    // c1->addWidget(c2);

    // roa::LinContainer *c3 = new roa::LinContainer(&ui);
    // c3->SetPos(50, 50);
    // c3->SetSize(200, 200);
    // c2->addWidget(c3);


    double frameDelaySecs_ = 0.032;
    while (window->IsOpen()) {
        while (true) {
            auto evt = window->PollEvent();
            if (!evt.has_value()) break; 

            if (evt->type == dr4::Event::Type::QUIT ||
            (evt->type == dr4::Event::Type::KEY_DOWN && evt->key.sym == dr4::KeyCode::KEYCODE_ESCAPE)) {
                window->Close();
                break;
            }
            ui.ProcessEvent(evt.value());
        }

        // one idle/update + one render
        double frameStartSecs = ui.GetWindow()->GetTime();
        hui::IdleEvent idleEvent;
        idleEvent.absTime = frameStartSecs;
        idleEvent.deltaTime = frameDelaySecs_;
        ui.OnIdle(idleEvent);

        // expensive: consider moving to worker thread or reducing work per frame
        camera.render(sceneManager);

        window->Clear({50,50,50,255});
        if (ui.GetTexture()) window->Draw(*ui.GetTexture());
        window->Display();

        //std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

#include <utility>
#include <chrono>
#include <dlfcn.h>
#include <cstdint>
#include <functional>
#include <chrono>
#include <thread>

#include "RayTracer.h"
#include "Camera.h"
#include "hui/ui.hpp"
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
// inline SDL_Color convertRTPixelColor(const RTPixelColor color) { return {color.r, color.g, color.b, color.a}; }

// class CameraWindow : public Widget {
//     static constexpr double CAMERA_ZOOM_DELTA = 0.1;

//     Camera *camera_ = nullptr;

//     gm::IVec2f   accumulatedCameraRotation_ = {0, 0};
//     bool           cameraNeedRotation_        = false;

//     gm::IVec2f   accumulatedCameraRel_      = {0, 0};
//     bool           cameraNeedRelocation_      = false;

//     int            accumulatedCameraZoom_     = 0;
//     bool           cameraNeedZoom_            = false;


//     bool onKeyDownSelfAction(const KeyEvent &event) override {
//         switch (event.sym) {
//             case SDLK_a:    
//                 accumulatedCameraRotation_ += gm::IVec2f (-CAMERA_KEY_CONTROL_DELTA, 0);
//                 cameraNeedRotation_ = true;
//                 return false;

//             case SDLK_d:
//                 accumulatedCameraRotation_ += gm::IVec2f (CAMERA_KEY_CONTROL_DELTA, 0);
//                 cameraNeedRotation_ = true;
//                 return false;

//             case SDLK_w: 
//                 accumulatedCameraRotation_ += gm::IVec2f (0, -CAMERA_KEY_CONTROL_DELTA);
//                 cameraNeedRotation_ = true;
//                 return false;
            
//             case SDLK_s:
//                 accumulatedCameraRotation_ += gm::IVec2f (0, CAMERA_KEY_CONTROL_DELTA);
//                 cameraNeedRotation_ = true;
//                 return false;
                
//             case SDLK_LEFT:
//                 accumulatedCameraRel_ += gm::IVec2f (CAMERA_KEY_CONTROL_DELTA, 0);
//                 cameraNeedRelocation_ = true;
//                 return false;
            
//             case SDLK_RIGHT:    
//                 accumulatedCameraRel_ += gm::IVec2f (-CAMERA_KEY_CONTROL_DELTA, 0);
//                 cameraNeedRelocation_ = true;
//                 return false;

//             case SDLK_UP: 
//                 accumulatedCameraRel_ += gm::IVec2f (0, CAMERA_KEY_CONTROL_DELTA);
//                 cameraNeedRelocation_ = true;
//                 return false;
            
//             case SDLK_DOWN:
//                 accumulatedCameraRel_ += gm::IVec2f (0, -CAMERA_KEY_CONTROL_DELTA);
//                 cameraNeedRelocation_ = true;
//                 return false;
//         }

//         return true;
//     }

//     bool onMouseWheelSelfAction(const MouseWheelEvent &event) override {
//         accumulatedCameraZoom_ += event.rot.y;
//         cameraNeedZoom_ = true;
//         return false;
//     }

//     bool onMouseMoveSelfAction(const MouseMotionEvent &event) override {
//         switch (event.button) {
//             case SDL_BUTTON_MIDDLE:
//                 accumulatedCameraRotation_ += event.rel;
//                 cameraNeedRotation_ = true;
//                 return false;
//             case SDL_BUTTON_LEFT:
//                 accumulatedCameraRel_ += event.rel * CAMERA_MOUSE_RELOCATION_SCALE;
//                 cameraNeedRelocation_ = true;
//                 return false;
//         }
//         return true;
//     }

//     void applyCameraRelocation() {
//         double dx = (double) accumulatedCameraRel_.x / camera_->screenResolution().first * camera_->viewPort().VIEWPORT_WIDTH;
//         double dy = (double) accumulatedCameraRel_.y / camera_->screenResolution().second * camera_->viewPort().VIEWPORT_HEIGHT;

//         gm::IVec3f motionVec = camera_->viewPort().rightDir_ * dx + camera_->viewPort().downDir_ * dy;
//         camera_->move(motionVec * (-1));
    
//         accumulatedCameraRel_ = {0, 0};
//         cameraNeedRelocation_ = false;
//     }

//     void applyCameraRotation() {
//         double widthRadians  = (double) accumulatedCameraRotation_.x / camera_->screenResolution().first * camera_->viewAngle().x();
//         double heightRadians = (double) accumulatedCameraRotation_.y / camera_->screenResolution().second * camera_->viewAngle().y();
//         camera_->rotate(-widthRadians, -heightRadians);
    
//         accumulatedCameraRotation_ = {0, 0};
//         cameraNeedRotation_ = false;
//     }

//     void applyCameraZoom() {
//         gm::IVec3f zoomVec = camera_->direction() * CAMERA_ZOOM_DELTA * accumulatedCameraZoom_;
//         camera_->move(zoomVec);

//         accumulatedCameraZoom_ = 0;
//         cameraNeedZoom_ = false;
//     }

//     bool updateSelfAction() override {
//         if (cameraNeedRotation_  ) applyCameraRotation();
//         if (cameraNeedRelocation_) applyCameraRelocation();
//         if (cameraNeedZoom_)       applyCameraZoom();
        
//         setRerenderFlag();

//         return true;
//     }

//     void updateTexture(SDL_Renderer *renderer, const std::vector<RTPixelColor>& pixels, int width, int height)
//     {
        
//         for (int i = 0; i < width; i++) {
//             for (int j = 0; j < height; j++) {
//                 SDL_Color color = convertRTPixelColor(camera_->getPixel(i, j));
//                 SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a); 
//                 SDL_RenderDrawPoint(renderer, i, j); 
//             }
//         }
//     }

//     void renderSelfAction(SDL_Renderer *renderer) override {
//         assert(renderer);
//         if (!camera_) return;
//         updateTexture(renderer, camera_->pixels(), camera_->screenResolution().first, camera_->screenResolution().second);
//     }

// public:
//     void setCamera(Camera *camera) { 
//         assert(camera);
//         camera_ = camera; 
//     }

//     CameraWindow(int width, int height, Widget * parent=nullptr): Widget(width, height, parent) {}
// };

// double measureRenderTime(SceneManager &sceneManager, Camera &camera) {
//     const std::size_t MEASURE_COUNT = 100;
    
//     double duration = 0;
//     for (std::size_t i = 0; i < MEASURE_COUNT; i++) {
//         auto start = std::chrono::high_resolution_clock::now();
//         sceneManager.render(camera);
//         auto end = std::chrono::high_resolution_clock::now();  
     
//         duration += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
//     }

//     return duration / MEASURE_COUNT;
// }

class LinContainer : public hui::Container {
    std::vector<hui::Widget *> children_; 
public:
    LinContainer(hui::UI *ui): hui::Container(ui) {}
    hui::EventResult PropogateToChildren(hui::Event &event) override {
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
        GetTexture().Clear({(int)GetSize().x % 255, (int)GetSize().y % 255, 0, 255});
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

    window->SetSize({MAIN_WINDOW_SIZE.first, MAIN_WINDOW_SIZE.second});
    window->StartTextInput();
    window->Open();
    window->Display();

    hui::UI ui(window);

    roa::LinContainer *mainWindow = new roa::LinContainer(&ui);
    mainWindow->SetPos(0, 0);
    mainWindow->SetSize(500, 500);
    ui.SetRoot(mainWindow);

    
    roa::LinContainer *c1 = new roa::LinContainer(&ui);
    c1->SetPos(50, 50);
    c1->SetSize(400, 400);
    mainWindow->addWidget(c1);

    roa::LinContainer *c2 = new roa::LinContainer(&ui);
    c2->SetPos(50, 50);
    c2->SetSize(300, 300);
    c1->addWidget(c2);

    roa::LinContainer *c3 = new roa::LinContainer(&ui);
    c3->SetPos(50, 50);
    c3->SetSize(200, 200);
    c2->addWidget(c3);



    double frameDelaySecs_ = 0.032;
    while (window->IsOpen()) {
        while (1) {
            if (ui.GetHovered()) 
                std::cout << "hovered : " << ui.GetHovered()->GetSize().x << "\n";
            if (ui.GetFocused()) 
                std::cout << "focused : " << ui.GetFocused()->GetSize().x << "\n";

            auto evt = window->PollEvent();
            double frameStartSecs = ui.GetWindow()->GetTime();

            if (evt.has_value()) {
                if (evt.value().type == dr4::Event::Type::QUIT ||
                    (evt.value().type == dr4::Event::Type::KEY_DOWN && evt.value().key.sym == dr4::KeyCode::KEYCODE_ESCAPE)) {
                        window->Close();
                        break;
                }
        
                ui.ProcessEvent(evt.value());
            }
    
            hui::IdleEvent idleEvent;
            idleEvent.absTime = frameStartSecs;
            idleEvent.deltaTime = frameDelaySecs_;
            ui.OnIdle(idleEvent);

            // render
            window->Clear({50, 50, 50, 255});
            if (ui.GetTexture()) {
                window->Draw(*ui.GetTexture());
            }
            
            window->Display(); 
            
            double frameTimeSecs = ui.GetWindow()->GetTime() - frameStartSecs;
            if (frameDelaySecs_ > frameTimeSecs) 
            std::this_thread::sleep_for(std::chrono::duration<double>(frameTimeSecs));
        }
    }






    // SceneManager sceneManager;

    // CameraWindow *cameraWindow = new CameraWindow(SCREEN_RESOLUTION.first, SCREEN_RESOLUTION.second, mainWindow);
    // mainWindow->addWidget(0, 0, cameraWindow);




    // RTMaterial *groundMaterial = new RTLambertian({0.8, 0.8, 0.0});    
    // RTMaterial *midSphereMaterial = new RTLambertian({0.1, 0.2, 0.5});
    // RTMaterial *rightSphereMaterial = new RTMetal({0.8, 0.8, 0.8}, 0.3);
    // RTMaterial *glassMaterial = new RTDielectric({1.0, 1.0, 1.0}, 1.50);
    // RTMaterial *sunMaterial = new RTEmissive(gm::IVec3f(1.0, 0.95, 0.9) * 10);






    // // SphereObject *light = new SphereObject(1, lightSrcMaterial, &sceneManager);

    // SphereObject *sun = new SphereObject(1, sunMaterial, &sceneManager);

    // Light *light = new Light
    // (
    //     /* ambientIntensity  */  gm::IVec3f(0.2, 0.2, 0.2),
    //     /* defuseIntensity   */  gm::IVec3f(0.8, 0.7, 0.6),
    //     /* specularIntensity */  gm::IVec3f(0.7, 0.7, 0),
    //     /* viewLightPow      */  15.0
    // );

    // SphereObject    *midSphere = new SphereObject(1, midSphereMaterial, &sceneManager);
    // SphereObject    *rightSphere = new SphereObject(1, rightSphereMaterial, &sceneManager);
  
    // PlaneObject     *ground = new PlaneObject({0, 0, 0}, {0, 0, 1}, groundMaterial, &sceneManager);



    // SphereObject    *glassSphere = new SphereObject(1, glassMaterial, &sceneManager);

    
    // sceneManager.addObject({0, 0, -100}, ground);
    // sceneManager.addObject({0, 0, 1}, glassSphere);
    // // // sceneManager.addObject({-2, 0, 1}, leftBubbleSphere);
    // sceneManager.addObject({0, 4, 3}, midSphere);
    // sceneManager.addObject({2, 0, 1}, rightSphere);

    // sceneManager.addLight({0, 0, 10}, light);
    // sceneManager.addObject({-2, 0, 4}, sun);

    // Camera camera(/*center*/{0, -6, 1}, /*direction*/{0, 3, 0}, SCREEN_RESOLUTION);
    // camera.setSamplesPerPixel(1);
    // camera.setSamplesPerScatter(1);
    // // camera.disableLDirect();
    // camera.setMaxRayDepth(5);
    // camera.setThreadPixelbunchSize(100);
    // // camera.disableParallelRender();




    // cameraWindow->setCamera(&camera);


    // // // !!!!! WARNING  

    // std::cout << "renderTime : " << measureRenderTime(sceneManager, camera) << '\n';

        
    // application.addUserEvent([&sceneManager, &camera](int deltaMS) { sceneManager.render(camera);});
    // application.run();

}


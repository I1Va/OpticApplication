#include <iostream>
#include <chrono>

#include "MyGUI.h"
#include "RayTracer.h"
#include "Camera.h"
#include "ScrollBar.h"


const std::pair<int, int> MAIN_WINDOW_SIZE = {600, 600};
const int APP_BORDER_SIZE = 20;
const int CAMERA_KEY_CONTROL_DELTA = 10;
const int CAMERA_MOUSE_RELOCATION_SCALE = 2;
const std::pair<int, int> SCREEN_RESOLUTION = {600, 600};

inline SDL_Color convertRTPixelColor(const RTPixelColor color) { return {color.r, color.g, color.b, color.a}; }

class CameraWindow : public Widget {
    static constexpr double CAMERA_ZOOM_DELTA = 0.1;

    Camera *camera_ = nullptr;

    gm_dot<int, 2> accumulatedCameraRotation_ = {0, 0};
    bool           cameraNeedRotation_        = false;

    gm_dot<int, 2> accumulatedCameraRel_      = {0, 0};
    bool           cameraNeedRelocation_      = false;

    int            accumulatedCameraZoom_     = 0;
    bool           cameraNeedZoom_            = false;


    bool onKeyDownSelfAction(const KeyEvent &event) override {
        switch (event.sym) {
            case SDLK_a:    
                accumulatedCameraRotation_ += gm_dot<int, 2> (-CAMERA_KEY_CONTROL_DELTA, 0);
                cameraNeedRotation_ = true;
                return false;

            case SDLK_d:
                accumulatedCameraRotation_ += gm_dot<int, 2> (CAMERA_KEY_CONTROL_DELTA, 0);
                cameraNeedRotation_ = true;
                return false;

            case SDLK_w: 
                accumulatedCameraRotation_ += gm_dot<int, 2> (0, -CAMERA_KEY_CONTROL_DELTA);
                cameraNeedRotation_ = true;
                return false;
            
            case SDLK_s:
                accumulatedCameraRotation_ += gm_dot<int, 2> (0, CAMERA_KEY_CONTROL_DELTA);
                cameraNeedRotation_ = true;
                return false;
                
            case SDLK_LEFT:
                accumulatedCameraRel_ += gm_dot<int, 2> (CAMERA_KEY_CONTROL_DELTA, 0);
                cameraNeedRelocation_ = true;
                return false;
            
            case SDLK_RIGHT:    
                accumulatedCameraRel_ += gm_dot<int, 2> (-CAMERA_KEY_CONTROL_DELTA, 0);
                cameraNeedRelocation_ = true;
                return false;

            case SDLK_UP: 
                accumulatedCameraRel_ += gm_dot<int, 2> (0, CAMERA_KEY_CONTROL_DELTA);
                cameraNeedRelocation_ = true;
                return false;
            
            case SDLK_DOWN:
                accumulatedCameraRel_ += gm_dot<int, 2> (0, -CAMERA_KEY_CONTROL_DELTA);
                cameraNeedRelocation_ = true;
                return false;
        }

        return true;
    }

    bool onMouseWheelSelfAction(const MouseWheelEvent &event) override {
        accumulatedCameraZoom_ += event.rot.y;
        cameraNeedZoom_ = true;
        return false;
    }

    bool onMouseMoveSelfAction(const MouseMotionEvent &event) override {
        switch (event.button) {
            case SDL_BUTTON_MIDDLE:
                accumulatedCameraRotation_ += event.rel;
                cameraNeedRotation_ = true;
                return false;
            case SDL_BUTTON_LEFT:
                accumulatedCameraRel_ += event.rel * CAMERA_MOUSE_RELOCATION_SCALE;
                cameraNeedRelocation_ = true;
                return false;
        }
        return true;
    }

    void applyCameraRelocation() {
        double dx = (double) accumulatedCameraRel_.x / camera_->screenResolution().first * camera_->viewPort().VIEWPORT_WIDTH;
        double dy = (double) accumulatedCameraRel_.y / camera_->screenResolution().second * camera_->viewPort().VIEWPORT_HEIGHT;

        gm::IVec3f motionVec = camera_->viewPort().rightDir_ * dx + camera_->viewPort().downDir_ * dy;
        camera_->move(motionVec * (-1));
    
        accumulatedCameraRel_ = {0, 0};
        cameraNeedRelocation_ = false;
    }

    void applyCameraRotation() {
        double widthRadians  = (double) accumulatedCameraRotation_.x / camera_->screenResolution().first * camera_->viewAngle().x();
        double heightRadians = (double) accumulatedCameraRotation_.y / camera_->screenResolution().second * camera_->viewAngle().y();
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

    bool updateSelfAction() override {
        if (cameraNeedRotation_  ) applyCameraRotation();
        if (cameraNeedRelocation_) applyCameraRelocation();
        if (cameraNeedZoom_)       applyCameraZoom();
        
        setRerenderFlag();

        return true;
    }

    void updateTexture(SDL_Renderer *renderer, const std::vector<RTPixelColor>& pixels, int width, int height)
    {
        
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                SDL_Color color = convertRTPixelColor(camera_->getPixel(i, j));
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a); 
                SDL_RenderDrawPoint(renderer, i, j); 
            }
        }
    }

    void renderSelfAction(SDL_Renderer *renderer) override {
        assert(renderer);
        if (!camera_) return;
        updateTexture(renderer, camera_->pixels(), camera_->screenResolution().first, camera_->screenResolution().second);
    }

public:
    void setCamera(Camera *camera) { 
        assert(camera);
        camera_ = camera; 
    }

    CameraWindow(int width, int height, Widget * parent=nullptr): Widget(width, height, parent) {}
};



// class ObjectsListWidget : public Widget {

// };


// class OjectsListWindow : public Window {
//     ObjectsListWidget *objectList = nullptr;
//     ScrollBar *scalbar = nullptr;
// };


// class SceneWindow : public Window {
//     CameraWindow *cameraWindow = nullptr;
    
//     SceneWindow(int w, int h, Widget *parent=nullptr): Window(w, h, this) { 
//         cameraWindow = new CameraWindow(SCREEN_RESOLUTION.first, SCREEN_RESOLUTION.second, this);
//         addWidget(0, 0, cameraWindow);
//     }
// };



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

int main() {
    UIManager application(MAIN_WINDOW_SIZE.first, MAIN_WINDOW_SIZE.second, 10);
    Container *mainWindow = new Container(MAIN_WINDOW_SIZE.first - 2 * APP_BORDER_SIZE, MAIN_WINDOW_SIZE.second - 2 * APP_BORDER_SIZE);
    application.setMainWidget(APP_BORDER_SIZE, APP_BORDER_SIZE, mainWindow);

    SceneManager sceneManager;

    CameraWindow *cameraWindow = new CameraWindow(SCREEN_RESOLUTION.first, SCREEN_RESOLUTION.second, mainWindow);
    mainWindow->addWidget(0, 0, cameraWindow);






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
    // camera.disableParallelRender();




    cameraWindow->setCamera(&camera);


    // // !!!!! WARNING  

    std::cout << "renderTime : " << measureRenderTime(sceneManager, camera) << '\n';

        
    application.addUserEvent([&sceneManager, &camera](int deltaMS) { sceneManager.render(camera);});
    application.run();

}


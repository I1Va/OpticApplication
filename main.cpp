#include <iostream>
#include <chrono>
#include <string>

#include "MyGUI.h"
#include "RayTracer.h"
#include "Camera.h"
#include "ScrollBar.h"
#include "TextWidget.hpp"



// const ButtonTexturePath scrollBarTopBtnPath    = {"images/scrollBar/topButton/unpressed.png", "images/scrollBar/topButton/pressed.png"};
// const ButtonTexturePath scrollBarBottomBtnPath = {"images/scrollBar/bottomButton/unpressed.png", "images/scrollBar/bottomButton/pressed.png"};
// const ButtonTexturePath scrollThumbBtnPath     = {"images/scrollBar/thumbButton/unpressed.png", "images/scrollBar/thumbButton/pressed.png"};


const char FONT_PATH[] = "fonts/Roboto/RobotoFont.ttf";
const std::pair<int, int> MAIN_WINDOW_SIZE = {1000, 600};
const int APP_BORDER_SIZE = 20;
const int CAMERA_KEY_CONTROL_DELTA = 10;
const int CAMERA_MOUSE_RELOCATION_SCALE = 2;
const std::pair<int, int> RENDER_SCREEN_RESOLUTION = {600, 600};

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

class ObjectListWidget : public Container {
    static constexpr std::size_t BORDER_THIKNESS = 3;
    static constexpr SDL_Color BORDER_COLOR = BLACK_SDL_COLOR;
    static constexpr SDL_Color BACK_COLOR = WHITE_SDL_COLOR;

    static constexpr SDL_Color TEXT_COLOR = BLACK_SDL_COLOR;
    static constexpr std::size_t OBJECT_RECORD_HEIGHT = 20;
    TTF_Font* font_ = nullptr;

    std::vector<Primitives *> objects_{};
    Primitives *selectedObject_ = nullptr;

    bool objectListChanged = true;

public:
    ObjectListWidget(int width, int height, TTF_Font* font, Widget *parent=nullptr):
        Container(width, height, parent), font_(font) {}
    
    void setObjects(const std::vector<Primitives *> objects) { objects_ = objects; }

    void unselectObject(Primitives *primitive) {
        primitive->setSelectFlag(false);
    }

    void selectObject(Primitives *primitive) {
        selectedObject_ = primitive;
        selectedObject_->setSelectFlag(true);
    }

    void updateRecords() {
         for (Widget *child : children_) {
            assert(child);
        
            if (UIManager_->hovered() == child) UIManager_->setHovered(nullptr);
            if (UIManager_->mouseActived() == child) UIManager_->setMouseActived(nullptr);
            
            delete child;
        }
        children_.clear();

        for (size_t i = 0; i < objects_.size(); i++) {
            Primitives *primitive = objects_[i];
        
            std::string recordName = std::to_string(i) + ") " + primitive->typeString();
            
            ClickableTextWidget *objectRecord = new ClickableTextWidget(rect_.w - 2 * BORDER_THIKNESS, OBJECT_RECORD_HEIGHT, recordName, font_, 
                                                                        [this, primitive]() { selectObject(primitive); },
                                                                        [this, primitive]() { unselectObject(primitive); },
                                                                        this);
                                                                    
            addWidget(BORDER_THIKNESS, BORDER_THIKNESS + i * OBJECT_RECORD_HEIGHT, objectRecord);
        }

        setRerenderFlag();
        objectListChanged = false;
    }

    bool updateSelfAction() override {
        if (objectListChanged) updateRecords();       

        return true; 
    }

    void renderSelfAction(SDL_Renderer* renderer) override { 

        SDL_Rect frame = {0, 0, rect_.w, rect_.h};
        SDL_SetRenderDrawColor(renderer, BORDER_COLOR.r, BORDER_COLOR.g, BORDER_COLOR.b, BORDER_COLOR.a);
        SDL_RenderFillRect(renderer, &frame);
    
        SDL_SetRenderDrawColor(renderer, BACK_COLOR.r, BACK_COLOR.g, BACK_COLOR.b, BACK_COLOR.a);
        SDL_Rect interior = {BORDER_THIKNESS, BORDER_THIKNESS, rect_.w - 2 * BORDER_THIKNESS, rect_.h - 2 * BORDER_THIKNESS};
        SDL_RenderFillRect(renderer, &interior);                
    }

    Primitives *selectedObject() { return selectedObject_; }
};

// class ObjectPropertiesWidget : public Container {

// public:
//     ObjectPropertiesWidget(int width, int height, Widget *parent=nullptr): Container(width, height, parent) {}




// };



class SceneWindow : public Container {
    SceneManager sceneManager_;
    Camera camera_;
    CameraWindow *cameraWindow_ = nullptr;

public:
    SceneWindow(int w, int h, Widget *parent=nullptr): 
        Container(w, h, parent), 
        sceneManager_(), camera_(/*center*/{0, -6, 1}, /*direction*/{0, 3, 0}, RENDER_SCREEN_RESOLUTION) 
    { 
        camera_.setSamplesPerScatter(1);
        camera_.setSamplesPerPixel(1);
        camera_.disableLDirect();
        camera_.setMaxRayDepth(5);
        camera_.setThreadPixelbunchSize(100);
        // camera.disableParallelRender();

        cameraWindow_ = new CameraWindow(w, h, this);

        cameraWindow_->setCamera(&camera_);
        addWidget(0, 0, cameraWindow_);
    }

    void addObject(gm::IPoint3 position, Primitives *object) { sceneManager_.addObject(position, object); }
    void addLight(gm::IPoint3 position, Light *light) { sceneManager_.addLight(position, light); }
    void renderSelfAction(SDL_Renderer* renderer) override { 
        camera_.render(sceneManager_);
        cameraWindow_->render(renderer);
    }


    double measureRenderTime(const std::size_t MEASURE_COUNT=1) {
        double duration = 0;
        for (std::size_t i = 0; i < MEASURE_COUNT; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            sceneManager_.render(camera_);
            auto end = std::chrono::high_resolution_clock::now();  
        
            duration += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        }

        return duration / MEASURE_COUNT;
    }



    const Camera &camera() const { return camera_; }
    const SceneManager &sceneManager() const { return sceneManager_; }


};


void textReceived(const std::string &str) {
    std::cout << "textReceived : `" << str << "`" << "\n";
}

int main() {
    UIManager application(MAIN_WINDOW_SIZE.first, MAIN_WINDOW_SIZE.second, 10);


    TTF_Font* font = application.createFont(FONT_PATH, 18);

    Container *mainWindow = new Container(MAIN_WINDOW_SIZE.first - 2 * APP_BORDER_SIZE, MAIN_WINDOW_SIZE.second - 2 * APP_BORDER_SIZE);
    application.setMainWidget(APP_BORDER_SIZE, APP_BORDER_SIZE, mainWindow);

    SceneWindow *sceneWindow = new SceneWindow(RENDER_SCREEN_RESOLUTION.first, RENDER_SCREEN_RESOLUTION.second, mainWindow);
    mainWindow->addWidget(0, 0, sceneWindow);

    ObjectListWidget * objectListWidget = new ObjectListWidget(200, 150, font, mainWindow);
    mainWindow->addWidget(650, 100, objectListWidget);
   


    TextInputWidget *textField = new TextInputWidget(300, 50, "23423", font, textReceived, mainWindow);
    mainWindow->addWidget(650, 0, textField);






    RTMaterial *groundMaterial = new RTLambertian({0.8, 0.8, 0.0});    
    RTMaterial *midSphereMaterial = new RTLambertian({0.1, 0.2, 0.5});
    RTMaterial *rightSphereMaterial = new RTMetal({0.8, 0.8, 0.8}, 0.3);
    RTMaterial *glassMaterial = new RTDielectric({1.0, 1.0, 1.0}, 1.50);
    RTMaterial *sunMaterial = new RTEmissive(gm::IVec3f(1.0, 0.95, 0.9) * 10);






    // SphereObject *light = new SphereObject(1, lightSrcMaterial, &sceneManager);

    SphereObject *sun = new SphereObject(1, sunMaterial, &sceneWindow->sceneManager());

    Light *light = new Light
    (
        /* ambientIntensity  */  gm::IVec3f(0.2, 0.2, 0.2),
        /* defuseIntensity   */  gm::IVec3f(0.8, 0.7, 0.6),
        /* specularIntensity */  gm::IVec3f(0.7, 0.7, 0),
        /* viewLightPow      */  15.0
    );

    SphereObject    *midSphere = new SphereObject(1, midSphereMaterial, &sceneWindow->sceneManager());
    SphereObject    *rightSphere = new SphereObject(1, rightSphereMaterial, &sceneWindow->sceneManager());
  
    PlaneObject     *ground = new PlaneObject({0, 0, 0}, {0, 0, 1}, groundMaterial, &sceneWindow->sceneManager());



    SphereObject    *glassSphere = new SphereObject(1, glassMaterial, &sceneWindow->sceneManager());

    
    sceneWindow->addObject({0, 0, -100}, ground);
    sceneWindow->addObject({0, 0, 1}, glassSphere);
    // // sceneManager.addObject({-2, 0, 1}, leftBubbleSphere);
    sceneWindow->addObject({0, 4, 3}, midSphere);
    sceneWindow->addObject({2, 0, 1}, rightSphere);

    sceneWindow->addLight({0, 0, 10}, light);
    sceneWindow->addObject({-2, 0, 4}, sun);

    objectListWidget->setObjects(sceneWindow->sceneManager().primitives());


 

    // // !!!!! WARNING  

    std::cout << "renderTime : " << sceneWindow->measureRenderTime() << "\n"; 

        
    // application.addUserEvent([sceneWindow](int deltaMS) { sceneWindow->render();});
    application.run();

}


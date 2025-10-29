#include <iostream>
#include <chrono>
#include <string>

#include "MyGUI.h"
#include "RayTracer.h"
#include "Camera.h"
#include "ScrollBar.h"



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

 
class TextWidget : public Widget {
    static constexpr SDL_Color DEFAULT_TEXT_COLOR=BLACK_SDL_COLOR;
    static constexpr SDL_Color DEFAULT_BACKGROUND_COLOR=WHITE_SDL_COLOR;
protected:
    std::string text_;
    TTF_Font* font_;
    SDL_Color textColor_;
    SDL_Color backColor_;
    

public:
    TextWidget
    (
        const std::size_t width, const std::size_t height,
        const std::string &text, TTF_Font *font,
        Widget *parent=nullptr
    ): 
        Widget(width, height, parent),
        text_(text), 
        font_(font),
        textColor_(DEFAULT_TEXT_COLOR),
        backColor_(DEFAULT_BACKGROUND_COLOR)
    { 
        assert(font_); 
    }

    void renderSelfAction(SDL_Renderer* renderer) override {
        assert(renderer);

        SDL_SetRenderDrawColor(renderer, backColor_.r, backColor_.g, backColor_.b, backColor_.a);
        SDL_Rect full = {0, 0, rect_.w, rect_.h};
        SDL_RenderFillRect(renderer, &full);

        SDL_Rect textRect = getTextSize(font_, text_.c_str());
        SDL_Texture* textTexture = createFontTexture(font_, text_.c_str(), textColor_, renderer);
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
    }

    void setText(const std::string &str)     { text_ = str; setRerenderFlag();        }
    void setBackColor(const SDL_Color color) { backColor_ = color; setRerenderFlag(); }
    void setTextColor(const SDL_Color color) { textColor_ = color; setRerenderFlag(); }
    
    const std::string &text() const { return text_; } 
};

class TextInputWidget : public TextWidget {
    std::function<void(const std::string)> onEnter_;
    bool needOnEnterCall_ = false;
public:
    TextInputWidget
    (
        const std::size_t width, const std::size_t height,
        const std::string &text,
        TTF_Font *font,
        std::function<void(const std::string)> onEnter=nullptr,
        Widget *parent=nullptr
    ):  
        TextWidget(width, height, text, font, parent),
        onEnter_(onEnter) {}

    bool updateSelfAction() {
        if (needOnEnterCall_) {
            if (onEnter_) onEnter_(text_);
            needOnEnterCall_ = false;
            return true;
        }

        return false;
    }

    bool onKeyDownSelfAction(const KeyEvent &event) override {
        if (event.sym == SDLK_BACKSPACE && !text_.empty()) {
            text_.pop_back();
            setRerenderFlag();
            return false;
        }

        if (event.sym >= SDLK_0 && event.sym <= SDLK_9) {
            text_.push_back(static_cast<char>('0' + (event.sym - SDLK_0)));
            setRerenderFlag();
            return false;
        }

        if (event.sym >= SDLK_a && event.sym <= SDLK_z) {
            bool shift = (event.keymod & KMOD_SHIFT);
            char c = static_cast<char>(shift ? ('A' + (event.sym - SDLK_a))
                                             : ('a' + (event.sym - SDLK_a)));
            text_.push_back(c);
            setRerenderFlag();
            return false;
        }

        if (event.sym == SDLK_SPACE) {
            text_.push_back(' ');
            setRerenderFlag();
            return false;
        }

        if (event.sym == SDLK_PERIOD) {
            text_.push_back('.');
            setRerenderFlag();
            return false;
        }

        if (event.sym == SDLK_COMMA) {
            text_.push_back(',');
            setRerenderFlag();
            return false;
        }

        if (event.sym == SDLK_KP_ENTER || event.sym == SDLK_RETURN) {
            setRerenderFlag();
            needOnEnterCall_ = true;
            return false;
        }

        return true;
    }

    void setOnEnter( std::function<void(const std::string &)> onEnter) { onEnter_ = onEnter; }
};


class Button : public Widget {
protected:
    bool pressed_ = false;
    std::function<void()> onClickFunction_= nullptr;

public:
    Button
    (
        int width, int height,
        std::function<void()> onClickFunction=nullptr, Widget *parent=nullptr
    ) : Widget(width, height, parent),  
        onClickFunction_(onClickFunction)
    {}

    // bool onMouseUpSelfAction(const MouseButtonEvent &event) {
    //     if (event.button == SDL_BUTTON_LEFT) {
    //         pressed_ = false;
    //         setRerenderFlag();
    //         return true;
    //     }
    //     return false;
    // }

    bool onMouseDownSelfAction(const MouseButtonEvent &event) {
       
        if (event.button == SDL_BUTTON_LEFT) {
            pressed_ = !pressed_;
            setRerenderFlag();
            if (onClickFunction_) onClickFunction_();
            return true;
        }
        return false;
    }

    virtual void setPressedTexture(SDL_Renderer* renderer) {
        assert(renderer);

        SDL_Rect buttonRect = {0, 0, rect_.w, rect_.h};
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        SDL_RenderFillRect(renderer, &buttonRect);
    }

    virtual void setUnPressedTexture(SDL_Renderer* renderer) {
        assert(renderer);

        SDL_Rect buttonRect = {0, 0, rect_.w, rect_.h};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &buttonRect);
    }

    void renderSelfAction(SDL_Renderer* renderer) override {
        if (pressed_) {
            setPressedTexture(renderer);
        } else {
            setUnPressedTexture(renderer);
        }
    }
};

class ClickableTextWidget : public Button {
    static constexpr SDL_Color PRESSED_BACK_COLOR = {128, 128, 128, 255};
    static constexpr SDL_Color UNPRESSED_BACK_COLOR = WHITE_SDL_COLOR;
    static constexpr SDL_Color TEXT_COLOR = BLACK_SDL_COLOR;
    TextWidget textWidget_;

public:
    ClickableTextWidget
    (   
        int width, int height, 
        const std::string &text,
        TTF_Font *font,
        std::function<void()> onClickFunction,
        Widget *parent=nullptr
    ): 
        Button(width, height, onClickFunction, parent),
        textWidget_(width, height, text, font, parent)
    {}

protected:

    void setPressedTexture(SDL_Renderer* renderer) override {
        assert(renderer);      
        SDL_Rect buttonRect = {0, 0, rect_.w, rect_.h};
        SDL_SetRenderDrawColor(renderer, PRESSED_BACK_COLOR.r, PRESSED_BACK_COLOR.g, PRESSED_BACK_COLOR.b, PRESSED_BACK_COLOR.a);
        SDL_RenderFillRect(renderer, &buttonRect);

        textWidget_.setBackColor(PRESSED_BACK_COLOR);
        textWidget_.render(renderer);
    
        SDL_Rect dst = textWidget_.rect();
        SDL_RenderCopy(renderer, textWidget_.texture(), NULL, &dst);
    }

    void setUnPressedTexture(SDL_Renderer* renderer) override {
        assert(renderer);      

        SDL_Rect buttonRect = {0, 0, rect_.w, rect_.h};
        SDL_SetRenderDrawColor(renderer, UNPRESSED_BACK_COLOR.r, UNPRESSED_BACK_COLOR.g, UNPRESSED_BACK_COLOR.b, UNPRESSED_BACK_COLOR.a);
        SDL_RenderFillRect(renderer, &buttonRect);

        textWidget_.setBackColor(UNPRESSED_BACK_COLOR);
        textWidget_.render(renderer);
    
        SDL_Rect dst = textWidget_.rect();
        SDL_RenderCopy(renderer, textWidget_.texture(), NULL, &dst);
    }
};



// // Material properties (shader, color, roughness, etc.)
// class MaterialPropertiesWidget : public Widget {
// public:
//     Material* target;
//     Slider colorR, colorG, colorB;
//     Slider roughness, metallic;
//     void draw() override;
//     void update() override;
// };

// // Transformation controls (position, rotation, scale)
// class TransformWidget : public Widget {
// public:
//     Object* target;
//     Vec3Input position, rotation, scale; // custom composed widgets
//     void draw() override;
//     void update() override;
// };


// class PropertiesPanel : public Container {
// public:
//     ObjectListWidget objList;
//     MaterialPropertiesWidget matProps;
//     TransformWidget transform;
//     void draw() override;
//     void update() override;
// };



// List of scene objects
class ObjectListWidget : public Container {
    static constexpr SDL_Color TEXT_COLOR = BLACK_SDL_COLOR;
    static constexpr std::size_t OBJECT_RECORD_HEIGHT = 20;
    TTF_Font* font_ = nullptr;

    std::vector<Primitives *> objects_{};
    int selectedIndex_ = -1;

    bool objectListChanged = true;

public:
    ObjectListWidget(int width, int height, TTF_Font* font, Widget *parent=nullptr):
        Container(width, height, parent), font_(font) {}
    

    void setObjects(const std::vector<Primitives *> objects) { objects_ = objects; }

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
            
            ClickableTextWidget *objectRecord = new ClickableTextWidget(rect_.w, OBJECT_RECORD_HEIGHT, recordName, font_, nullptr, this);
            addWidget(0, i * OBJECT_RECORD_HEIGHT, objectRecord);
        }

        setRerenderFlag();
        objectListChanged = false;
    }

    bool updateSelfAction() override {
        if (objectListChanged) updateRecords();       

        return true; 
    }
};


//   int width, int height, 
//         const std::string &text,
//         TTF_Font *font,
//         std::function<void()> onClickFunction,
//         Widget *parent=nullptr



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

    ObjectListWidget * objectListWidget = new ObjectListWidget(200, 500, font, mainWindow);
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


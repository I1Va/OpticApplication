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

#include "cum/manager.hpp"
#include "cum/plugin.hpp"
#include "cum/ifc/dr4.hpp"

#include "hui/container.hpp"
#include "hui/event.hpp"


const char FONT_PATH[] = "Roboto/RobotoFont.ttf";
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

      hui::EventResult OnIdle(hui::IdleEvent &) override {
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
        try {
            hui::IdleEvent idleEvent = dynamic_cast<hui::IdleEvent&>(event);
            bool handled = false;
            for (Widget *child : children_) {
                if (idleEvent.Apply(*child) == hui::EventResult::HANDLED) handled = true;
            }

            if (handled) return hui::EventResult::HANDLED;
            else return hui::EventResult::UNHANDLED;
        } catch (std::exception &exc) { 
            for (Widget *child : children_) {
                if (event.Apply(*child) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
            }
        
            return hui::EventResult::UNHANDLED;
        }
    }

    // void deleteWidget(int idx) {
    //     UnbecomeParentOf(children_[idx]);
    //     children_.erase(idx);
    // }

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

class FocusButton : public hui::Widget {
public:
    FocusButton(hui::UI *ui): hui::Widget(ui) {}
    
    hui::EventResult OnIdle(hui::IdleEvent &) override {    
        ForceRedraw();

        return hui::EventResult::HANDLED;
    }
    
    void Redraw() const override { 
        dr4::Color color = {0, 0, 0, 255};

        if (GetUI()->GetFocused() == this) {
            color = {255, 0, 0, 255};
        }        
        GetTexture().Clear(color);
    }
};

class Button : public hui::Widget {
    bool pressed = true;

public:
    Button(hui::UI *ui): hui::Widget(ui) {}
    
    hui::EventResult OnMouseDown(hui::MouseButtonEvent &event) override {
        if (!GetRect().Contains(event.pos)) return hui::EventResult::UNHANDLED;

        GetUI()->ReportFocus(this);
        pressed = !pressed;
        ForceRedraw();

        return hui::EventResult::HANDLED;
    } 
    
    void Redraw() const override { 
        dr4::Color color = {0, 0, 0, 255};

        if (pressed) {
            color = {0, 0, 255, 255};
        }        
        GetTexture().Clear(color);
    }
};

class HoverButton : public hui::Widget {
public:
    HoverButton(hui::UI *ui): hui::Widget(ui) {}
    
    hui::EventResult OnIdle(hui::IdleEvent &) override {   
        ForceRedraw();

        return hui::EventResult::HANDLED;
    }
    
    void Redraw() const override { 
        dr4::Color color = {0, 0, 0, 255};

        if (GetUI()->GetHovered() == this) {
            color = {0, 255, 0, 255};
        }        
        GetTexture().Clear(color);
    }
};

// class Text : public hui::Widget {

// };

}

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        std::cerr << "Expected one argument: dr4 backend path\n";
        return 1;
    }
    const char *dr4BackendPath = argv[1];

    cum::Manager pluginManager;
    pluginManager.LoadFromFile(dr4BackendPath);
    auto *dr4Backend = pluginManager.GetAnyOfType<cum::DR4BackendPlugin>();
    assert(dr4Backend);


    std::cout << "dr4 plugin identifier  : " << dr4Backend->GetIdentifier() << "\n";
    std::cout << "dr4 plugin description : " << dr4Backend->GetDescription() << "\n";

    
    dr4::Window *window = dr4Backend->CreateWindow();


    window->SetSize({float(MAIN_WINDOW_SIZE.first), float(MAIN_WINDOW_SIZE.second)});
    window->StartTextInput();
    window->Open();
    window->Display();

    hui::UI ui(window);

    roa::LinContainer *mainWindow = new roa::LinContainer(&ui);
    mainWindow->SetPos(0, 0);
    mainWindow->SetSize(500.0f, 500.0f);
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

    roa::FocusButton *focusbutton = new roa::FocusButton(&ui);
    focusbutton->SetPos(10, 10);
    focusbutton->SetSize(50, 50);
    c3->addWidget(focusbutton);

    roa::HoverButton * hoverButton = new roa::HoverButton(&ui);
    hoverButton->SetPos(60, 60);
    hoverButton->SetSize(40, 40);
    c3->addWidget(hoverButton);


    roa::Button *button = new roa::Button(&ui);
    button->SetPos(70, 0);
    button->SetSize(30, 30);
    c3->addWidget(button);


    // dr4::Font *font = window->CreateFont();
    // font->LoadFromFile(FONT_PATH)
    
    





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

        double frameStartSecs = ui.GetWindow()->GetTime();
        hui::IdleEvent idleEvent;
        idleEvent.absTime = frameStartSecs;
        idleEvent.deltaTime = frameDelaySecs_;
        ui.OnIdle(idleEvent);

        window->Clear({50,50,50,255});
        if (ui.GetTexture()) window->Draw(*ui.GetTexture());
        window->Display();

        //std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

#include <iostream>

#include "MyGUI.h"
#include "RayTracer.h"
#include "Camera.h"

const std::pair<int, int> MAIN_WINDOW_SIZE = {900, 700};
const int APP_BORDER_SIZE = 20;

const std::pair<int, int> SCREEN_RESOLUTION = {800, 600};

class CameraWindow : public Widget {
    const Camera *camera_ = nullptr;

    bool updateSelfAction() override {
        setRerenderFlag();

        return true;
    }

    void updateTexture(SDL_Renderer *renderer, const std::vector<RTColor>& pixels, int width, int height)
    {
        
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                RTColor color = camera_->getPixel(i, j);
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
    void setCamera(const Camera *camera) { camera_ = camera; }

    CameraWindow(int width, int height, Widget * parent=nullptr): Widget(width, height, parent) {}
};


int main() {
    UIManager application(MAIN_WINDOW_SIZE.first, MAIN_WINDOW_SIZE.second);
    Container *mainWindow = new Container(MAIN_WINDOW_SIZE.first - 2 * APP_BORDER_SIZE, MAIN_WINDOW_SIZE.second - 2 * APP_BORDER_SIZE);
    application.setMainWidget(APP_BORDER_SIZE, APP_BORDER_SIZE, mainWindow);

    CameraWindow *cameraWindow = new CameraWindow(SCREEN_RESOLUTION.first, SCREEN_RESOLUTION.second, mainWindow);
    mainWindow->addWidget((mainWindow->rect().w - cameraWindow->rect().w) / 2, (mainWindow->rect().h - cameraWindow->rect().h) / 2, cameraWindow);

    SceneManager sceneManager;

    Camera camera({0, 0, 0}, {0, 0, 1}, SCREEN_RESOLUTION);
    sceneManager.render(camera);

    cameraWindow->setCamera(&camera);

    application.addUserEvent([&sceneManager, &camera](int deltaMS) {sceneManager.render(camera); });
    application.run();

}
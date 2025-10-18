#include <iostream>

#include "MyGUI.h"
#include "RayTracer.h"
#include "Camera.h"

const std::pair<int, int> MAIN_WINDOW_SIZE = {900, 700};
const int APP_BORDER_SIZE = 20;

const std::pair<int, int> SCREEN_RESOLUTION = {600, 600};

inline SDL_Color convertRTPixelColor(const RTPixelColor color) { return {color.r, color.g, color.b, color.a}; }

class CameraWindow : public Widget {
    const Camera *camera_ = nullptr;

    bool updateSelfAction() override {
        // setRerenderFlag();

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
    RTMaterial *groundMaterial = new RTLambertian({0.8, 0.8, 0.0});

    

    RTMaterial *midSphereMaterial = new RTLambertian({0.1, 0.2, 0.5});
    RTMaterial *rightSphereMaterial = new RTMetal({0.8, 0.8, 0.8}, 0.3);

    RTMaterial *lightSrcMaterial = new RTEmissive({10.0, 10.0, 10.0});


  
    PlaneObject *ground = new PlaneObject({0, 0, 0}, {0, 0, 1}, groundMaterial, &sceneManager);


    SphereObject *light = new SphereObject(1, lightSrcMaterial, &sceneManager);


    SphereObject *midSphere = new SphereObject(1, midSphereMaterial, &sceneManager);
    SphereObject *rightSphere = new SphereObject(1, rightSphereMaterial, &sceneManager);
   

    sceneManager.addObject({0, 0, -100}, ground);










    RTMaterial *leftSphereMaterial = new RTDielectric(1.50);
    RTMaterial *leftSphereBubbleMaterial = new RTDielectric(1.00 / 1.50);

    SphereObject *leftSphere = new SphereObject(0.8, leftSphereBubbleMaterial, &sceneManager);
    SphereObject *leftBubbleSphere = new SphereObject(1, leftSphereBubbleMaterial, &sceneManager);


    
    sceneManager.addObject({-2, 0, 1}, leftSphere);
    sceneManager.addObject({-2, 0, 1}, leftBubbleSphere);







    sceneManager.addObject({0, 0, 1}, midSphere);
    sceneManager.addObject({2, 0, 1}, rightSphere);

    sceneManager.addObject({0, 0, 4}, light);

    Camera camera(/*center*/{0, -6, 1}, /*direction*/{0, 3, 0}, SCREEN_RESOLUTION);
    camera.setSamplesPerPixel(1);
    camera.setMaxRayDepth(10);

    cameraWindow->setCamera(&camera);


    // !!!!! WARNING  
    sceneManager.render(camera);
    // application.addUserEvent([&sceneManager, &camera](int deltaMS) { std::cout << "updateSmth!!!\n";});
    application.run();

}

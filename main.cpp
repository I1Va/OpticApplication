#include <iostream>
#include <chrono>
#include <string>

#include "RayTracer.h"
#include "SceneWidgets.hpp"


const char FONT_PATH[] = "fonts/Roboto/RobotoFont.ttf";
const std::pair<int, int> MAIN_WINDOW_SIZE = {1000, 600};
const int APP_BORDER_SIZE = 20;

class ScreenShotWindow : public Container {
    public:
        ScreenShotWindow(int w, int h) : Container(w, h, /*parent=*/nullptr) {
            SDL_SetTextureAlphaMod(texture_, 0);
        }
    
    private:
        void renderSelfAction(SDL_Renderer* renderer) override {
            assert(renderer);


            int border = 3; 
            // SDL_SetRenderDrawColor(renderer, 2, 250, 247, 255);
            // SDL_Rect full =  {0, 0, rect_.w, rect_.h};
            // SDL_RenderFillRect(renderer, &full);
            
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128); // black
            SDL_Rect full = {border, border, rect_.w - 2 * border, rect_.h - 2 * border};
            SDL_RenderFillRect(renderer, &full);

            
        }

        bool updateSelfAction() override {
            setRerenderFlag();

            return true;
        }

        bool onMouseWheelSelfAction(const MouseWheelEvent &event) override {
            return PROPAGATE;
        }
        bool onMouseDownSelfAction(const MouseButtonEvent &event) override {
            return PROPAGATE;
        }
        bool onMouseUpSelfAction(const MouseButtonEvent &event) override {
            return PROPAGATE;
        }
        bool onMouseMoveSelfAction(const MouseMotionEvent &event) override {
            return PROPAGATE;
        }
        bool onKeyDownSelfAction(const KeyEvent &event) override {
            return PROPAGATE;
        }
        bool onKeyUpSelfAction(const KeyEvent &event) override {
            return PROPAGATE;
        }
};

SceneWidget *createSceneWidget(Container *mainWindow) {
    assert(mainWindow);

    RTMaterial *groundMaterial = new RTLambertian({0.8, 0.8, 0.0});    
    RTMaterial *midSphereMaterial = new RTLambertian({0.1, 0.2, 0.5});
    RTMaterial *rightSphereMaterial = new RTMetal({0.8, 0.8, 0.8}, 0.3);
    RTMaterial *glassMaterial = new RTDielectric({1.0, 1.0, 1.0}, 1.50);
    RTMaterial *sunMaterial = new RTEmissive(gm::IVec3f(1.0, 0.95, 0.9) * 10);

    SceneWidget *sceneWindow = new SceneWidget(RENDER_SCREEN_RESOLUTION.first, RENDER_SCREEN_RESOLUTION.second, mainWindow);

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

    return sceneWindow;
}

ObjectListComponent *createObjectsPanel(Container *mainWindow, SceneWidget *sceneWidget, TTF_Font* font) {
    assert(mainWindow);
    assert(sceneWidget);
    assert(font);

    ObjectPropertiesComponent *objectPropertiesComponent = new ObjectPropertiesComponent(300, 300, font, mainWindow);
    mainWindow->addWidget(650, 200, objectPropertiesComponent);
    
    ObjectListComponent *objectListComponent = new ObjectListComponent(200, 150, font, 
        [objectPropertiesComponent](Primitives *selected) { objectPropertiesComponent->selectObject(selected); },
        [objectPropertiesComponent](Primitives *selected) { objectPropertiesComponent->selectObject(nullptr); },
        mainWindow);
    
    

    objectListComponent->setObjects(sceneWidget->sceneManager().primitives());

    return objectListComponent;
}

void test() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow("Transparency Demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

    int quit = 0;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = 1;
        }

        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_RenderClear(ren);

        // Opaque blue rectangle
        SDL_Rect r1 = {100, 100, 200, 200};
        SDL_SetRenderDrawColor(ren, 0, 0, 255, 255);
        SDL_RenderFillRect(ren, &r1);

        // Semi-transparent red rectangle overlapping
        SDL_Rect r2 = {180, 180, 200, 200};
        SDL_SetRenderDrawColor(ren, 255, 0, 0, 128);
        SDL_RenderFillRect(ren, &r2);

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}


int main() {

    UIManager application(MAIN_WINDOW_SIZE.first, MAIN_WINDOW_SIZE.second, 10);
    TTF_Font* font = application.createFont(FONT_PATH, 22);

    Container *mainWindow = new Container(MAIN_WINDOW_SIZE.first - 2 * APP_BORDER_SIZE, MAIN_WINDOW_SIZE.second - 2 * APP_BORDER_SIZE);
    application.setMainWidget(APP_BORDER_SIZE, APP_BORDER_SIZE, mainWindow);

    Widget *wgt = new Widget(100, 100);
    mainWindow->addWidget(100, 100, wgt);

    SceneWidget *sceneWidget = createSceneWidget(mainWindow);
    ObjectListComponent *objectsPanel = createObjectsPanel(mainWindow, sceneWidget, font);

    mainWindow->addWidget(0, 0, sceneWidget);
    mainWindow->addWidget(650, 10, objectsPanel);


    // ScreenShotWindow *screenShotWindow = new ScreenShotWindow(400, 431);


    // application.pushModalWidget(100, 100, screenShotWindow);

    std::cout << "renderTime : " << sceneWidget->measureRenderTime() << "\n"; 
    application.run();

    TTF_CloseFont(font);
}
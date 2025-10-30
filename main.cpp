#include <iostream>
#include <chrono>
#include <string>

#include "MyGUI.h"
#include "RayTracer.h"
#include "SceneWidgets.hpp"

const char FONT_PATH[] = "fonts/Roboto/RobotoFont.ttf";
const std::pair<int, int> MAIN_WINDOW_SIZE = {1000, 600};
const int APP_BORDER_SIZE = 20;

int main() {
    UIManager application(MAIN_WINDOW_SIZE.first, MAIN_WINDOW_SIZE.second, 10);


    RTMaterial *groundMaterial = new RTLambertian({0.8, 0.8, 0.0});    
    RTMaterial *midSphereMaterial = new RTLambertian({0.1, 0.2, 0.5});
    RTMaterial *rightSphereMaterial = new RTMetal({0.8, 0.8, 0.8}, 0.3);
    RTMaterial *glassMaterial = new RTDielectric({1.0, 1.0, 1.0}, 1.50);
    RTMaterial *sunMaterial = new RTEmissive(gm::IVec3f(1.0, 0.95, 0.9) * 10);



    TTF_Font* font = application.createFont(FONT_PATH, 22);

    Container *mainWindow = new Container(MAIN_WINDOW_SIZE.first - 2 * APP_BORDER_SIZE, MAIN_WINDOW_SIZE.second - 2 * APP_BORDER_SIZE);
    application.setMainWidget(APP_BORDER_SIZE, APP_BORDER_SIZE, mainWindow);

    SceneWidget *sceneWindow = new SceneWidget(RENDER_SCREEN_RESOLUTION.first, RENDER_SCREEN_RESOLUTION.second, mainWindow);
    mainWindow->addWidget(0, 0, sceneWindow);

    ObjectPropertiesComponent *objectPropertiesComponent = new ObjectPropertiesComponent(300, 300, font, mainWindow);
    mainWindow->addWidget(650, 200, objectPropertiesComponent);
    
    ObjectListComponent *objectListComponent = new ObjectListComponent(200, 150, font, 
        [objectPropertiesComponent](Primitives *selected) { objectPropertiesComponent->selectObject(selected); },
        [objectPropertiesComponent](Primitives *selected) { objectPropertiesComponent->selectObject(nullptr); },
        mainWindow);
    
    mainWindow->addWidget(650, 10, objectListComponent);

        

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

    objectListComponent->setObjects(sceneWindow->sceneManager().primitives());


    std::cout << "renderTime : " << sceneWindow->measureRenderTime() << "\n"; 
    application.run();
}
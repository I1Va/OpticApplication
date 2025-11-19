#include <iostream>
#include <dlfcn.h>
#include <cassert>
#include "misc/dr4_ifc.hpp"
#include "hui/widget.hpp"
#include "hui/state.hpp"

const char PLAGIN_PATH[] = "external/libAIPlugin.so";
const char FONT_PATH[] = "assets/RobotoFont.ttf";

typedef dr4::DR4Backend* (*DR4BackendFunction)();

void test(dr4::Window *window) {
 dr4::Rectangle *rect = window->CreateRectangle();
    rect->SetBorderColor({0, 255, 0, 200});
    rect->SetFillColor({200, 0, 0, 100});
    rect->SetPos({30, 30});
    rect->SetSize({100, 200});
    rect->SetBorderThickness(10);

    dr4::Circle *circle = window->CreateCircle();
    circle->SetBorderColor({0, 0, 255, 255});
    circle->SetBorderThickness(10);
    circle->SetCenter({100, 100});
    circle->SetFillColor({0, 0, 0, 0});
    circle->SetRadius(30);


    dr4::Text *text = window->CreateText();
    dr4::Font *font = window->CreateFont();
    font->LoadFromFile(FONT_PATH);
    text->SetFont(font);
    text->SetColor({0, 255, 0, 150});
    text->SetVAlign(dr4::Text::VAlign::MIDDLE);
    text->SetFontSize(30);
    text->SetText("Ia obyzatelno vizhivu!");

    dr4::Texture *texture = window->CreateTexture();
    texture->SetSize({500, 500});
    texture->SetPos({50, 50});
    texture->Clear({255, 255, 255, 255});
    texture->SetZero({0, 0});

    
    rect->DrawOn(*texture);
    circle->DrawOn(*texture);

    text->SetPos(100, 100);
    text->DrawOn(*texture);
    
    while (true) {
        window->Clear({0, 0, 0, 255});
        window->Draw(*texture);
        window->Display();

        std::optional<dr4::Event> event = window->PollEvent();
        if (event.has_value() && event.value().type == dr4::Event::Type::QUIT) {
            break;
        }
    }
   
    

    delete circle;
    delete texture;
    delete rect;
    delete text;
    delete font;
}

int main() {
    void *backendLib = dlopen(PLAGIN_PATH, RTLD_LAZY);
    DR4BackendFunction func = (DR4BackendFunction) dlsym(backendLib, dr4::DR4BackendFunctionName); assert(func);
    dr4::DR4Backend *backend = func(); assert(backend);
    dr4::Window *window = backend->CreateWindow(); assert(window);
    assert(window);

    window->Open();
    window->SetSize({800, 600});


    hui::State state(window);
    hui::Widget *wgt = new hui::Widget(&state);
    assert(wgt);

    dr4::Texture *texture = window->CreateTexture();
    assert(texture);

    texture->SetSize(600, 400);
    texture->SetPos(100, 100);
    texture->Clear({255, 255, 255, 255});

    wgt->SetPos(30, 30);
    wgt->SetSize(100, 100);
    wgt->DrawOn(*texture);
    
    
    while (true) {
        window->Clear({0, 0, 0, 255});
        window->Draw(*texture);
        window->Display();

        std::optional<dr4::Event> event = window->PollEvent();
        if (event.has_value() && event.value().type == dr4::Event::Type::QUIT) {
            break;
        }
    }




    delete window;
    delete backend;
    delete texture;
    delete wgt;
}

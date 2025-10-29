#pragma once

#include "MyGUI.h"


class Button : public Widget {
protected:
    bool pressed_ = false;
    std::function<void()> onDownFunction_= nullptr;
    std::function<void()> onUpFunction_= nullptr;
public:
    Button
    (
        int width, int height,
        std::function<void()> onDownFunction=nullptr,
        std::function<void()> onUpFunction=nullptr,
        Widget *parent=nullptr
    ) : Widget(width, height, parent),  
        onDownFunction_(onDownFunction),
        onUpFunction_(onUpFunction)
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
        
            if (pressed_) {
                if (onDownFunction_) onDownFunction_();
            } else {
                if (onUpFunction_) onUpFunction_();
            }

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


#pragma once

#include <functional>
#include "Widget.h"


struct ButtonTexturePath {
    const char *unpressed;
    const char *pressed;
};

class Button : public Widget {
protected:
    bool pressed_ = false;
    std::function<void()> onDownFunction_;
    std::function<void()> onUpFunction_;
    bool sticky_;
public:
    static constexpr bool STICKY = true;

    Button
    (
        int width, int height,
        bool sticky=STICKY,
        std::function<void()> onDownFunction=nullptr,
        std::function<void()> onUpFunction=nullptr,
        Widget *parent=nullptr
    ) : Widget(width, height, parent),  
        onDownFunction_(onDownFunction),
        onUpFunction_(onUpFunction),
        sticky_(sticky)
    {}

    bool onMouseUpSelfAction(const MouseButtonEvent &event) {
        if (sticky_) return PROPAGATE;

        if (event.button == SDL_BUTTON_LEFT) {
            pressed_ = false;
            setRerenderFlag();
            return CONSUME;
        }
        return PROPAGATE;
    }

    bool onMouseDownSelfAction(const MouseButtonEvent &event) {
        if (event.button == SDL_BUTTON_LEFT) {
            pressed_ = !pressed_;
            setRerenderFlag();
        
            if (pressed_) {
                if (onDownFunction_) onDownFunction_();
            } else {
                if (onUpFunction_) onUpFunction_();
            }

            return CONSUME;
        }
        return PROPAGATE;
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


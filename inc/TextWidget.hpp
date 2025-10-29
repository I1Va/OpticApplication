#pragma once
#include "MyGUI.h"
#include "Button.hpp" 
#include <algorithm>
 

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
        textRect.w = std::clamp(textRect.w, 0, rect_.w);
        textRect.h = std::clamp(textRect.h, 0, rect_.h);

        SDL_Texture* textTexture = createFontTexture(font_, text_.c_str(), textColor_, renderer);
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
    }

    void setText(const std::string &str)     { text_ = str; setRerenderFlag();        }
    void setBackColor(const SDL_Color color) { backColor_ = color; setRerenderFlag(); }
    void setTextColor(const SDL_Color color) { textColor_ = color; setRerenderFlag(); }
    
    const std::string &text() const { return text_; } 
};

class TextInputWidget : public TextWidget {
    std::function<void(const std::string&)> onEnter_;
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
        std::function<void()> onDownFunction,
        std::function<void()> onUpFunction,
        Widget *parent=nullptr
    ): 
        Button(width, height, onDownFunction, onUpFunction, parent),
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

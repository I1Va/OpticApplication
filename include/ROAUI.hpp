#pragma once
#include <cassert>
#include <exception>
#include <iostream>

#include "hui/ui.hpp"

namespace roa
{
class UI : public hui::UI {
    dr4::Font *defaultFont = nullptr;

public:
    UI(dr4::Window *window, const std::string &defaultFontPath): hui::UI(window) {
        assert(window);

        try {
            defaultFont = window->CreateFont();        
            if (defaultFont)
                defaultFont->LoadFromFile(defaultFontPath);
        } catch (std::runtime_error &e) {
            delete defaultFont;
            std::cerr << "roa::UI create defaultFont failed : " << e.what() << "\n";
            throw;
        }
    }
    ~UI() { if (defaultFont) delete defaultFont; }

    dr4::Font *GetDefaultFont() { return defaultFont; }
};

} // namespace roa




#pragma once
#include <cassert>
#include <exception>
#include <iostream>
#include <vector>
#include <functional>

#include "dr4/event.hpp"

namespace roa
{

class UI : public hui::UI {
    dr4::Font *defaultFont = nullptr;
    std::vector<std::pair<dr4::Event::KeyEvent, std::function<void()>>> hotkeyTable;

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


    void AddHotkey(dr4::Event::KeyEvent keyEvent, std::function<void()> onHotkey) {
        assert(onHotkey);
        hotkeyTable.push_back({keyEvent, onHotkey});
    }

    void Run(double frameDelaySecs= 0.032) {
        while (GetWindow()->IsOpen()) {
            while (true) {
                auto evt = GetWindow()->PollEvent();
                if (!evt.has_value()) break; 

                if (evt->type == dr4::Event::Type::QUIT ||
                    (evt->type == dr4::Event::Type::KEY_DOWN && evt->key.sym == dr4::KeyCode::KEYCODE_ESCAPE)) {
                        GetWindow()->Close();
                        break;
                    }
                
                if (evt->type == dr4::Event::Type::KEY_DOWN) {
                    auto hotkeyFunction = findHotkeyFunction(evt->key);
                    if (hotkeyFunction) {
                        hotkeyFunction();
                        continue;
                    }
                }

                ProcessEvent(evt.value());
            }

            double frameStartSecs = GetWindow()->GetTime();
            hui::IdleEvent idleEvent;
            idleEvent.absTime = frameStartSecs;
            idleEvent.deltaTime = frameDelaySecs;
            OnIdle(idleEvent);

            GetWindow()->Clear({50,50,50,255});
            if (GetTexture()) GetWindow()->Draw(*GetTexture());
            GetWindow()->Display();

            GetWindow()->Sleep(frameDelaySecs);
        }
    }

    dr4::Font *GetDefaultFont() { return defaultFont; }

private:
    std::function<void()> findHotkeyFunction(dr4::Event::KeyEvent hotkey) {
        for (auto hk : hotkeyTable) {
            if (hotkey.sym == hk.first.sym && (hotkey.mods & hk.first.mods)) {
                return hk.second;
            }
        }
        return nullptr;
    }

};

} // namespace roa




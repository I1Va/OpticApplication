#pragma once
#include <functional>
#include <optional>

#include "hui/widget.hpp"
#include "Containers.hpp"
#include "RayTracer.h"
#include "ScrollBar.hpp"
#include "TextWidgets.hpp"
#include "Window.hpp"

namespace roa
{

// inline SDL_Color convertRTPixelColor(const RTPixelColor color) { return {color.r, color.g, color.b, color.a}; }

template<typename T>
concept IsPointer = std::is_pointer_v<T>;

template <IsPointer T>
class Outliner final : public LinContainer<hui::Widget> {
    static constexpr float RECORD_HEIGHT = 20;
    static constexpr float SCROLL_BAR_WIDTH = 6;
    static constexpr float SCROLL_BAR_RIGHT_PADDING = 3;
    static constexpr float SCROLL_BAR_HEIGHT_SHARE = 0.9;

    VerticalScrollBar            *scrollBar; 
    std::vector<ObjectButton *>   records;

public:
    Outliner(hui::UI *ui): 
        LinContainer(ui),
        scrollBar(new VerticalScrollBar(ui))
    {
        assert(ui);

        AddWidget(scrollBar);
        scrollBar->SetOnScrollAction([this](double) { relayoutRecords(); });
    }

    ~Outliner() = default;

    void AddObject
    (
        T object,
        const std::string &name,
        std::function<void()> onSelect,
        std::function<void()> onUnSelect
    ) {
        ObjectButton *record = new ObjectButton(GetUI()); assert(record);
    
        // record->SetText(name);
        record->SetMode(Button::Mode::STICK_MODE);

        // record->SetOnPressAction([this, name, object, onSelect, onUnSelect]()
        //     {
        //         if (onSelect) onSelect();
        //         if (!currentSelected.has_value() || currentSelected.value().second != object) {
        //             currentSelected = std::pair<std::string, T>(name, object);
        //             if (onSelectChangedAction) onSelectChangedAction();
        //         }
        //     }
        // );

        // record->SetOnUnpressAction([this, object, onUnSelect]()
        //     {
        //         if (onUnSelect) onUnSelect();
        //         if (currentSelected.has_value() && currentSelected.value().second == object) {
        //             currentSelected.reset();
        //             if (onSelectChangedAction) onSelectChangedAction();
        //         }
        //     }
        // );

        records.push_back(record);
        AddWidget(record);

        relayout();
    }

protected:
    void OnSizeChanged() override { relayout(); }

    void relayoutRecords() {
        float recordHeight = RECORD_HEIGHT;
        float recordWidth  = this->GetSize().x;
        dr4::Vec2f curRecordPos = dr4::Vec2f(0, 0);

        double scrollPerccentage = scrollBar->GetPercentage();
        float hBias = std::fmax(0, scrollPerccentage * (records.size() * recordHeight - this->GetSize().y));

        for (auto &record : records) {
            record->SetPos(curRecordPos - dr4::Vec2f(0, hBias));
            record->SetSize({recordWidth, recordHeight});
            record->ForceRedraw();
            curRecordPos += dr4::Vec2f(0, recordHeight);
        }

        this->ForceRedraw();
    }

    void relayoutScrollBar() {
        float recordsDummaryHeight = records.size() * RECORD_HEIGHT;

        scrollBar->Show();
        scrollBar->SetSize(SCROLL_BAR_WIDTH, GetSize().y * SCROLL_BAR_HEIGHT_SHARE);
        
        float posY = (this->GetSize().y - scrollBar->GetSize().y) / 2;
        scrollBar->SetPos({this->GetSize().x - SCROLL_BAR_WIDTH - SCROLL_BAR_RIGHT_PADDING, posY});

        double thumbLayoutShare = 1;
        if (recordsDummaryHeight > GetSize().y) {
            thumbLayoutShare = GetSize().y / recordsDummaryHeight;
        }
        scrollBar->SetThumbLayoutShare(thumbLayoutShare);
    }

    void relayout() {
        relayoutRecords();
        relayoutScrollBar();
    }

    void Redraw() const override {
        this->GetTexture().Clear(FULL_TRANSPARENT);
    
        for (const auto &record : records) {
            record->DrawOn(this->GetTexture());
        }
    
        scrollBar->DrawOn(this->GetTexture());
    }

    
    
private:
};

template <IsPointer T>
class OutlinerWindow final : public Window {
    static constexpr float TITLE_BAR_HEIGHT = 20;
    Outliner<T> *outliner;

public:
    OutlinerWindow(hui::UI *ui): Window(ui), outliner(new Outliner<T>(ui)) {
        assert(ui);
        AddWidget(outliner);
    }

    ~OutlinerWindow() = default;  

    void AddObject
    (
        T object,
        const std::string &name,
        std::function<void()> onSelect,
        std::function<void()> onUnSelect
    ) { outliner->AddObject(object, name, onSelect, onUnSelect); }

protected:
    void OnSizeChanged() override { layout(); }

private:
    void layout() {
        outliner->SetPos({0, TITLE_BAR_HEIGHT});
        outliner->SetSize(GetSize() - outliner->GetPos());
    }
};


} // namespace roa

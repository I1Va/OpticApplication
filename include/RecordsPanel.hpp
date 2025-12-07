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

template <WidgetDerived T>
class RecordsPanel : public Window {
    static constexpr float RECORD_HEIGHT = 20;
    static constexpr float SCROLL_BAR_WIDTH = 6;
    static constexpr float SCROLL_BAR_RIGHT_PADDING = 3;
    static constexpr float THUMB_AREA_VERTICAL_LAYOUT_SHARE = 0.9; 

protected:
    VerticalScrollBar  *scrollBar; 
    std::vector<T*>     records;

public:
    RecordsPanel(hui::UI *ui): 
        Window(ui), 
        scrollBar(new VerticalScrollBar(ui)) 
    { 
        assert(ui);
        
        SetSize({100, 100});

        AddWidget(scrollBar);

        scrollBar->SetOnScrollAction([this](double) { relayoutRecords(); });
    }

    virtual ~RecordsPanel() = default;

    void ClearRecords() {
        records.clear();
        this->ForceRedraw();
    }

protected:
    void OnSizeChanged() override { relayout(); }

    void relayoutRecords() {
        float recordHeight = RECORD_HEIGHT;
        float recordWidth = this->GetSize().x - scrollBar->GetSize().x;
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
        scrollBar->SetSize({SCROLL_BAR_WIDTH,  THUMB_AREA_VERTICAL_LAYOUT_SHARE * this->GetSize().y});
        float posY = (this->GetSize().y - scrollBar->GetSize().y) / 2;
        scrollBar->SetPos({this->GetSize().x - SCROLL_BAR_WIDTH - SCROLL_BAR_RIGHT_PADDING, posY});
    
        scrollBar->ForceRedraw();
    }

    void relayout() {
        relayoutRecords();
        relayoutScrollBar();
    }

    // void Redraw() const override {
    //     this->GetTexture().Clear({255, 132, 0, 255});
    
    //     for (const auto &record : records) {
    //         record->DrawOn(this->GetTexture());
    //     }
    //     scrollBar->DrawOn(this->GetTexture());
    //     title->DrawOn(this->GetTexture());
    // }

};

template <IsPointer T>
class ObjectsPanel final : public RecordsPanel<ObjectButton> {
    std::optional<std::pair<std::string, T>> currentSelected;
    std::function<void()> onSelectChangedAction = nullptr;
public:
    using RecordsPanel::RecordsPanel;
    ~ObjectsPanel() = default;

    std::optional<std::pair<std::string, T>> GetSelected() { return currentSelected; }

    void SetOnSelectChangedAction(std::function<void()> action) { onSelectChangedAction = action; }

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

        record->SetOnPressAction([this, name, object, onSelect, onUnSelect]()
            {
                if (onSelect) onSelect();
                if (!currentSelected.has_value() || currentSelected.value().second != object) {
                    currentSelected = std::pair<std::string, T>(name, object);
                    if (onSelectChangedAction) onSelectChangedAction();
                }
            }
        );

        record->SetOnUnpressAction([this, object, onUnSelect]()
            {
                if (onUnSelect) onUnSelect();
                if (currentSelected.has_value() && currentSelected.value().second == object) {
                    currentSelected.reset();
                    if (onSelectChangedAction) onSelectChangedAction();
                }
            }
        );

        records.push_back(record);
        AddWidget(record);

        relayout();
    }

};

} // namespace roa

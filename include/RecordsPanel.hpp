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
    static constexpr double RECORD_HEIGHT = 20;
    static constexpr double SCROLLBAR_LAYOUT_SHARE = 0.2;
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
        float scrollBarHeight = this->GetSize().y - 0;
        float scrollBarWidth = this->GetSize().x * SCROLLBAR_LAYOUT_SHARE;

        scrollBar->SetSize({scrollBarWidth, scrollBarHeight});
        scrollBar->SetPos({this->GetSize().x - scrollBarWidth, 0});
    
        scrollBar->ForceRedraw();
    }

    void relayout() {
        relayoutRecords();
        relayoutScrollBar();
    }
};

template <IsPointer T>
class ObjectsPanel final : public RecordsPanel<TextButton> {
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
        TextButton *record = new TextButton(GetUI()); assert(record);
        record->SetText(name);
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

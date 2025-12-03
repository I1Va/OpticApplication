#pragma once
#include <functional>

#include "hui/widget.hpp"
#include "Containers.hpp"
#include "RayTracer.h"
#include "ScrollBar.hpp"
#include "TextWidgets.hpp"

namespace roa
{

// inline SDL_Color convertRTPixelColor(const RTPixelColor color) { return {color.r, color.g, color.b, color.a}; }




template<typename T>
concept IsPointer = std::is_pointer_v<T>;

template <WidgetDerived T>
class RecordsPanel : public ZContainer<T> {
protected:
    static constexpr double RECORD_HEIGHT = 20;
    static constexpr double TITLE_HEIGHT = 20;
    static constexpr double SCROLLBAR_LAYOUT_SHARE = 0.2;

    std::unique_ptr<TextWidget> title;
    std::unique_ptr<VerticalScrollBar> scrollBar; 
    std::vector<std::unique_ptr<T>> records;

public:
    RecordsPanel(hui::UI *ui): 
        ZContainer<T>(ui), 
        title(new TextWidget(ui)),
        scrollBar(new VerticalScrollBar(ui)) 
    { 
        assert(ui);
        this->SetSize({100, 100});
        this->BecomeParentOf(scrollBar.get());

        scrollBar->SetOnScrollAction([this](double) { relayoutRecords(); });
        relayout();
    }

    virtual ~RecordsPanel() = default;

    void BringToFront(T *) override {}

    void SetTitle(const std::string &titleContent) {
        title->SetText(titleContent);
    }

    void ClearRecords() {
        records.clear();   
    }

protected:
    hui::EventResult PropagateToChildren(hui::Event &event) override {
        for (auto it = records.rbegin(); it != records.rend(); it++) {
            if (event.Apply(**it) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        }
        if (event.Apply(*scrollBar) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        return hui::EventResult::UNHANDLED;
    }
    
    void Redraw() const override {
        this->GetTexture().Clear({255, 132, 0, 255});
        for (const auto &record : records) {
            record->DrawOn(this->GetTexture());
        }
        scrollBar->DrawOn(this->GetTexture());
        title->DrawOn(this->GetTexture());
    }

    void OnSizeChanged() override { relayout(); }

    void relayoutRecords() {
        float recordHeight = RECORD_HEIGHT;
        float recordWidth = this->GetSize().x - scrollBar->GetSize().x;
        dr4::Vec2f curRecordPos = dr4::Vec2f(0, TITLE_HEIGHT);

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
        float scrollBarHeight = this->GetSize().y - TITLE_HEIGHT;
        float scrollBarWidth = this->GetSize().x * SCROLLBAR_LAYOUT_SHARE;

        scrollBar->SetSize({scrollBarWidth, scrollBarHeight});
        scrollBar->SetPos({this->GetSize().x - scrollBarWidth, TITLE_HEIGHT});
    
        this->ForceRedraw();
    }

    void relayoutTitle() {
        title->SetSize({this->GetSize().x, TITLE_HEIGHT});
    }

    void relayout() {
        relayoutRecords();
        relayoutScrollBar();
        relayoutTitle();
    }
};

template <IsPointer T>
class ObjectsPanel final : public RecordsPanel<TextButton> {
    T currentSelected = nullptr;
    std::function<void()> onSelectedAction = nullptr;
    std::function<void()> currentSelectedOnUnSelect = nullptr;
public:
    using RecordsPanel::RecordsPanel;
    ~ObjectsPanel() = default;

    T GetSelected() { return currentSelected; }

    void SetOnSelectedAction(std::function<void()> action) { onSelectedAction = action; }

    void AddObject
    (
        T object,
        const std::string &name,
        std::function<void()> onSelect,
        std::function<void()> onUnSelect
    ) {
        auto record = std::make_unique<TextButton>(GetUI());
        record->SetText(name);
        record->SetFocusedMode();

        record->SetOnClickAction([this, object, onSelect, onUnSelect]()
            {
                if (onSelect) onSelect();
                if (currentSelected != object && currentSelectedOnUnSelect) {
                    currentSelectedOnUnSelect();
                    if (onSelectedAction) onSelectedAction();
                }
                    
                currentSelected = object;
                currentSelectedOnUnSelect = onUnSelect;
            }
        );

        record->SetOnUnpressAction([this, object, onUnSelect]()
            {
                if (onUnSelect) onUnSelect();
                if (currentSelected == object) currentSelected = nullptr;
            }
        );

        TextButton* ptr = record.get();
        records.emplace_back(std::move(record));
        BecomeParentOf(ptr);

        relayout();
    }

};

class PropertiesPanel final : public RecordsPanel<TextInputField> {
public:
    using RecordsPanel::RecordsPanel;
    ~PropertiesPanel() = default;

    void AddProperty
    (
        const std::string &label, const std::string &value,
        std::function<void(const std::string&)> setPropertyVal
    ) {
        auto record = std::make_unique<TextInputField>(GetUI());
        record->SetLabel(label);
        record->SetText(value);
        record->SetOnEnterAction(setPropertyVal);

        TextInputField* ptr = record.get();
        records.emplace_back(std::move(record));
        BecomeParentOf(ptr);
        relayout();
    }

    hui::EventResult OnIdle(hui::IdleEvent &event) {
        PropagateToChildren(event);


        return hui::EventResult::UNHANDLED;
    }

};



} // namespace roa

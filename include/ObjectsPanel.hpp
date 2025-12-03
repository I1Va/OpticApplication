#pragma once
#include <functional>

#include "Containers.hpp"
#include "RayTracer.h"
#include "ScrollBar.hpp"

namespace roa
{

// class IPanelObject {
// public:
//     virtual ~IPanelObject() {};
//     virtual const std::string &GetName() = 0;
//     virtual void OnSelect() = 0;
//     virtual void OnUnSelect() = 0;
// };

// template<typename P>
// concept PointerToPanelObject =
//     std::is_pointer_v<P> &&
//     std::derived_from<std::remove_pointer_t<P>, IPanelObject>;

// class RTPrimPanelObject : public IPanelObject {
//     Primitives *object;
// public:
//     RTPrimPanelObject(Primitives *obj): object(obj) { assert(obj); }
//     ~RTPrimPanelObject() = default;
//     const std::string &GetName() override { return object->typeString(); }
//     void OnSelect() override { object->setSelectFlag(true); }
//     void OnUnSelect() override { object->setSelectFlag(false); }
// };

template<typename T>
concept IsPointer = std::is_pointer_v<T>;

template <IsPointer T>
class ObjectsPanel : public ZContainer<TextButton *> {
    static constexpr double RECORD_HEIGHT = 20;
    static constexpr double SCROLLBAR_LAYOUT_SHARE = 0.2;

    T currentSelected = nullptr;
    std::function<void()> currentSelectedOnUnSelect = nullptr;

    std::vector<std::unique_ptr<TextButton>> records;

    std::unique_ptr<VerticalScrollBar> scrollBar; 

public:
    ObjectsPanel(hui::UI *ui): 
        ZContainer(ui), 
        scrollBar(new VerticalScrollBar(ui)) 
    { 
        assert(ui);
        SetSize({100, 100});
        BecomeParentOf(scrollBar.get());

        scrollBar->SetOnScrollAction([this](double) { relayoutRecords(); });
        relayout();
    }

    ~ObjectsPanel() = default;

    void AddObject
    (
        T object,
        const std::string &name,
        std::function<void()> onSelect,
        std::function<void()> onUnSelect
    ) {
        TextButton *record = new TextButton(GetUI());
        record->SetText(name);
        record->SetFocusedMode();

        record->SetOnClickAction([this, object, onSelect, onUnSelect]()
            {
                if (onSelect) onSelect();
                if (currentSelected != object && currentSelected)
                    currentSelectedOnUnSelect();
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

        records.emplace_back(record);
        BecomeParentOf(record);

        relayout();
    }

    T GetSelected() { return currentSelected; }

    void BringToFront(TextButton *) override {}


protected:
    hui::EventResult PropagateToChildren(hui::Event &event) override {
        for (auto it = records.rbegin(); it != records.rend(); it++) {
            if (event.Apply(**it) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        }
        if (event.Apply(*scrollBar) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;
        return hui::EventResult::UNHANDLED;
    }
    
    void Redraw() const override {
        GetTexture().Clear({255, 132, 0, 255});
        for (const auto &record : records) {
            record->DrawOn(GetTexture());
        }
        scrollBar->DrawOn(GetTexture());
    }

    void OnSizeChanged() { relayout(); }

private:
    void relayoutRecords() {
        float recordHeight = RECORD_HEIGHT;
        float recordWidth = GetSize().x - scrollBar->GetSize().x;
        dr4::Vec2f curRecordPos = dr4::Vec2f(0, 0);

        double scrollPercentage = scrollBar->GetPercentage();
        float hBias = scrollPercentage * (records.size() * recordHeight - GetSize().y);
        for (auto &record : records) {
            record->SetPos(curRecordPos - dr4::Vec2f(0, hBias));
            record->SetSize({recordWidth, recordHeight});
            record->ForceRedraw();
            curRecordPos += dr4::Vec2f(0, recordHeight);
        }

        ForceRedraw();
    }

    void relayoutScrollBar() {
        float scrollBarHeight = GetSize().y;
        float scrollBarWidth = GetSize().x * SCROLLBAR_LAYOUT_SHARE;

        scrollBar->SetSize({scrollBarWidth, scrollBarHeight});
        scrollBar->SetPos({GetSize().x - scrollBarWidth, 0});
    
        ForceRedraw();
    }

    void relayout() {
        relayoutRecords();
        relayoutScrollBar();
    }
};

} // namespace roa

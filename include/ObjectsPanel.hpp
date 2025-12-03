#pragma once
#include <functional>

#include "Containers.hpp"
#include "RayTracer.h"

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
class ObjectsPanel : public LinContainer<TextButton *> {
    static constexpr double RECORD_HEIGHT = 20;

    T currentSelected = nullptr;
    std::function<void()> currentSelectedOnUnSelect = nullptr;

public:
    using LinContainer::LinContainer;

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

        addWidget(record);

        relayout();
    }

    T GetSelected() { return currentSelected; }

protected:
    void Redraw() const override {
        GetTexture().Clear({255, 132, 0, 255});
        for (TextButton *record : children) {
            record->DrawOn(GetTexture());
        }
    }

    void OnSizeChanged() { relayout(); }

private:
    void relayout() {
        float recordHeight = RECORD_HEIGHT;
        float recordWidth = GetSize().x;

        dr4::Vec2f curRecordPos = dr4::Vec2f(0, 0);
        for (TextButton *record : children) {
            record->SetPos(curRecordPos);
            record->SetSize({recordWidth, recordHeight});
            curRecordPos += dr4::Vec2f(0, recordHeight);
        }
    }
};

} // namespace roa

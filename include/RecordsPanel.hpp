#pragma once
#include <functional>
#include <optional>

#include "hui/widget.hpp"
#include "BasicWidgets/Containers.hpp"
#include "RayTracer.h"
#include "BasicWidgets/ScrollBar.hpp"
#include "BasicWidgets/TextWidgets.hpp"
#include "BasicWidgets/Window.hpp"

namespace roa
{

// inline SDL_Color convertRTPixelColor(const RTPixelColor color) { return {color.r, color.g, color.b, color.a}; }

template<typename T>
concept IsPointer = std::is_pointer_v<T>;



template <WidgetDerived T>
class RecordsPanel : public LinContainer<hui::Widget> {
    static constexpr float SCROLL_BAR_WIDTH = 6;
    static constexpr float SCROLL_BAR_RIGHT_PADDING = 3;
    static constexpr float SCROLL_BAR_HEIGHT_SHARE = 0.9;
    
    float recordsPadding = 0;

protected:
    VerticalScrollBar            *scrollBar; 
    std::vector<T *>              records;
public:
    RecordsPanel(hui::UI *ui): 
        LinContainer(ui),
        scrollBar(new VerticalScrollBar(ui))
    {
        assert(ui);

        AddWidget(scrollBar);
        scrollBar->SetOnScrollAction([this](double) { relayoutRecords(); });
    }
    ~RecordsPanel() = default;

    void SetRecordsPadding(const float padding) { recordsPadding = padding; }
protected:
    void OnSizeChanged() override { relayout(); }

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

    hui::EventResult OnMouseWheel(hui::MouseWheelEvent &evt) {
        if (GetRect().Contains(evt.pos)) {
            evt.pos -= GetPos();
            return evt.Apply(*scrollBar);
        }
        return hui::EventResult::UNHANDLED;
    }

private:
    float calculateSummaryRecordsHeight() {
        float result = 0;
        for (const auto &record : records) {
            result += record->GetSize().y;
        }
        return result;
    }

    void relayoutRecords() { // ADD PADDING
        dr4::Vec2f curRecordPos = dr4::Vec2f(0, 0);
        double scrollPerccentage = scrollBar->GetPercentage();
        float hBias = std::fmax(0, scrollPerccentage * (calculateSummaryRecordsHeight() - this->GetSize().y));

        for (auto &record : records) {
            record->SetPos(curRecordPos - dr4::Vec2f(0, hBias));
            record->ForceRedraw();
            curRecordPos += dr4::Vec2f(0, record->GetSize().y + recordsPadding);
        }

        this->ForceRedraw();
    }

    void relayoutScrollBar() {
        float recordsSummaryHeight = calculateSummaryRecordsHeight();

        scrollBar->Show();
        scrollBar->SetSize(SCROLL_BAR_WIDTH, GetSize().y * SCROLL_BAR_HEIGHT_SHARE);
        
        float posY = (this->GetSize().y - scrollBar->GetSize().y) / 2;
        scrollBar->SetPos({this->GetSize().x - SCROLL_BAR_WIDTH - SCROLL_BAR_RIGHT_PADDING, posY});

        double thumbLayoutShare = 1;
        if (recordsSummaryHeight > GetSize().y) {
            thumbLayoutShare = GetSize().y / recordsSummaryHeight;
        }
        scrollBar->SetThumbLayoutShare(thumbLayoutShare);
    }
};

template <IsPointer T>
class Outliner final : public RecordsPanel<ObjectButton> {
    static constexpr float RECORD_HEIGHT = 20;
    std::optional<std::pair<std::string, T>> currentSelected;
    std::function<void()> onSelectChangedAction = nullptr;
public:
    using RecordsPanel::RecordsPanel;
    ~Outliner() = default;

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
        record->SetSize({GetSize().x, RECORD_HEIGHT});
    
        record->SetLabelText(name);
        record->SetMode(Button::Mode::FOCUS_MODE);

        UI *ui = static_cast<UI *>(GetUI()); assert(ui);
        record->LoadSVGIcon(ui->GetIconsTexturePack().outlinerObMeshSvgPath);
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
        if (records.size() % 2) record->SetColorPack(GRAY_OBJECT_PACK);
        else                    record->SetColorPack(BLACK_OBJECT_PACK);
    
        records.push_back(record);
        AddWidget(record);

        relayout();
    }
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

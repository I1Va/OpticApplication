#pragma once
#include <functional>
#include <optional>
#include <vector>
#include <string>
#include <cassert>

#include "BasicWidgets/Containers.hpp"
#include "BasicWidgets/ScrollBar.hpp"

namespace roa {

template <WidgetDerived T>
class RecordsPanel : public Container {
protected:
    static constexpr float SCROLL_BAR_WIDTH = 6.0f;
    static constexpr float SCROLL_BAR_RIGHT_PADDING = 3.0f;
    static constexpr float SCROLL_BAR_HEIGHT_SHARE = 0.9f;
    const dr4::Vec2f DEFAULT_SIZE = dr4::Vec2f(100, 100);
    bool updatingRecords = false;

    float recordsPadding = 0;

    dr4::Vec2f recordsStartPos = {0,0};
    dr4::Color BGColor = {61, 61, 61};

    VerticalScrollBar* scrollBar = nullptr;
    std::vector<T*> records;

public:
    explicit RecordsPanel(hui::UI* ui) : Container(ui) {
        assert(ui);

        auto sb = std::make_unique<VerticalScrollBar>(ui);
        scrollBar = sb.get();
        AddWidget(std::move(sb));

        scrollBar->SetOnScrollAction([this](double){ relayoutRecords(); });
        SetSize(DEFAULT_SIZE);
    }

    virtual ~RecordsPanel() = default;
    
    void SetAllRecordsSize(const dr4::Vec2f size) {
        for (auto record : records) {
            record->SetSize(size);
        }
        relayout();
    }

    void SetRecordsPadding(float padding) { 
        recordsPadding = padding; 
        relayout();
    }
    void SetRecordsStartPos(const dr4::Vec2f pos) { 
        recordsStartPos = pos; 
        relayout();
    }
    void SetBGColor(const dr4::Color color) { 
        BGColor = color; 
        ForceRedraw();
    }
    size_t GetRecordCount() const { return records.size(); }

    float CalculateRecordsSumHeight() const {
        float sum = 0;
        for (auto r : records) sum += r->GetSize().y;
        return sum;
    }

    void AddRecord(std::unique_ptr<T> record) {
        records.push_back(record.get());        
        AddWidget(std::move(record));
        relayout();
    }
    void ClearRecords() {
        for (auto r : records) EraseWidget(r);
        records.clear();
        relayout();
    }

protected:
    void OnSizeChanged() override { relayout(); }

    void Redraw() const override {
        GetTexture().Clear(BGColor);
        for (auto r : records) r->DrawOn(GetTexture());
        if (!scrollBar->IsHidden()) scrollBar->DrawOn(GetTexture());
    }

    hui::EventResult OnMouseWheel(hui::MouseWheelEvent& evt) override {
        if (GetRect().Contains(evt.pos)) {
            evt.pos -= GetPos();
            return evt.Apply(*scrollBar);
        }
        return hui::EventResult::UNHANDLED;
    }

    void relayout() {
        relayoutRecords();
        relayoutScrollBar();
        ForceRedraw();
    }

private:
    void relayoutRecords() {
        dr4::Vec2f curPos = recordsStartPos;
        double scrollPct = scrollBar->GetPercentage();
        float scrollOffset = std::fmax(0.0f, scrollPct * (CalculateRecordsSumHeight() - GetSize().y));

        for (auto r : records) {
            r->SetPos(curPos - dr4::Vec2f(0, scrollOffset));
            r->ForceRedraw();
            curPos += dr4::Vec2f(0, r->GetSize().y + recordsPadding);
        }
    }

    void relayoutScrollBar() {
        float totalHeight = CalculateRecordsSumHeight();
        scrollBar->Show();
        scrollBar->SetSize(SCROLL_BAR_WIDTH, GetSize().y * SCROLL_BAR_HEIGHT_SHARE);
        float posY = (GetSize().y - scrollBar->GetSize().y) / 2;
        scrollBar->SetPos({GetSize().x - SCROLL_BAR_WIDTH - SCROLL_BAR_RIGHT_PADDING, posY});

        double thumbShare = 1.0;
        if (totalHeight > GetSize().y)
            thumbShare = GetSize().y / totalHeight;
        else
            scrollBar->Hide();

        scrollBar->SetThumbLayoutShare(thumbShare);
    }
};

} // namespace roa

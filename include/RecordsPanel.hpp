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
    dr4::Vec2f recordsStartPos = dr4::Vec2f(0, 0);
    dr4::Color BGColor = BLACK;

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

    void SetRecordsPadding(const float padding)   { recordsPadding = padding; }
    void SetRecordsStartPos(const dr4::Vec2f pos) { recordsStartPos = pos; }
    void SetBGColor(const dr4::Color color)       { BGColor = color; }
    std::vector<T *> &GetRecords()                { return records; }


    void AddRecord(T *record) {
        records.push_back(record);
        AddWidget(record);
        relayout();
    }
    
    void relayout() {
        relayoutRecords();
        relayoutScrollBar();
        ForceRedraw();
    }
    
    float calculateSummaryRecordsHeight() {
        float result = 0;
        for (const auto &record : records) {
            result += record->GetSize().y;
        }
        return result;
    }


protected:
    void OnSizeChanged() override { relayout(); }

    void Redraw() const override {
        this->GetTexture().Clear(BGColor);
    
        for (const auto &record : records) {
            record->DrawOn(this->GetTexture());
        }
        
        if (!scrollBar->IsHidden())
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
    void relayoutRecords() { // ADD PADDING
        dr4::Vec2f curRecordPos = recordsStartPos;
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
        } else {
            scrollBar->Hide();
        }
        scrollBar->SetThumbLayoutShare(thumbLayoutShare);
    }
};

   
struct ObjectButtonColorPack {
    dr4::Color focusedHovered;
    dr4::Color focused;
    dr4::Color hovered;
    dr4::Color nonActive;
};

static const inline ObjectButtonColorPack BLACK_OBJECT_PACK = 
{
    .focusedHovered = dr4::Color(78, 100, 145),
    .focused = dr4::Color(51, 77, 128),
    .hovered = dr4::Color(68, 68, 68),
    .nonActive = dr4::Color(40, 40, 40)
};

static const inline ObjectButtonColorPack GRAY_OBJECT_PACK = 
{
    .focusedHovered = dr4::Color(78, 100, 145),
    .focused = dr4::Color(51, 77, 128),
    .hovered = dr4::Color(71, 71, 71),
    .nonActive = dr4::Color(43, 43, 43)
};

class ObjectButton final : public Button {
    const dr4::Color labelNonFocusedColor = dr4::Color(174, 174, 174, 255); 
    const dr4::Color labelFocusedColor    = dr4::Color(232, 165, 55, 255); 

    ObjectButtonColorPack colorPack = BLACK_OBJECT_PACK;
    dr4::Text *label;
    dr4::Image *mainIcon;

public:
    ObjectButton(hui::UI *ui): 
        Button(ui), 
        label(ui->GetWindow()->CreateText()),
        mainIcon(ui->GetWindow()->CreateImage()) 
    {
        assert(ui);
        assert(label);    
        
        // FIXME: ADD LAYOUT
        label->SetPos({40, 3}); 
        label->SetFontSize(11);
        label->SetText("Figure.000");
        label->DrawOn(GetTexture());

        mainIcon->SetPos({20, 2});
        mainIcon->SetSize({16, 16});
    }

    ~ObjectButton() = default;
    
    void SetColorPack(const ObjectButtonColorPack pack) {
        colorPack = pack;
    }

    void SetLabel(const std::string &text) { label->SetText(text); }
    void SetLabelFontSize(const int fontSize) { label->SetFontSize(fontSize); }
    void LoadSVGMainIcon(const std::string &path) {
        assert(mainIcon);
        ExtractSVG(path, mainIcon->GetSize(), [this](int x, int y, dr4::Color color){ mainIcon->SetPixel(x, y, color); });
    }

protected:
    void Redraw() const override final {
        GetTexture().Clear(FULL_TRANSPARENT);

        label->SetColor(labelNonFocusedColor);
        if (checkStateProperty(Button::StateProperty::FOCUSED) && checkStateProperty(Button::StateProperty::HOVERED)) {
            GetTexture().Clear(colorPack.focusedHovered);
            label->SetColor(labelFocusedColor);
        } else if (checkStateProperty(Button::StateProperty::FOCUSED)) {
            label->SetColor(labelFocusedColor);
            GetTexture().Clear(colorPack.focused);
        } else if (checkStateProperty(Button::StateProperty::HOVERED)) {
            GetTexture().Clear(colorPack.hovered);
        } else {
            GetTexture().Clear(colorPack.nonActive);
        }
        
        mainIcon->DrawOn(GetTexture());
        label->DrawOn(GetTexture());
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

    void AddRecord
    (
        T object,
        const std::string &name,
        std::function<void()> onSelect,
        std::function<void()> onUnSelect
    ) {
        ObjectButton *record = new ObjectButton(GetUI()); assert(record);
        record->SetSize({GetSize().x, RECORD_HEIGHT});
    
        record->SetLabel(name);
        record->SetMode(Button::Mode::FOCUS_MODE);

        UI *ui = static_cast<UI *>(GetUI()); assert(ui);
        record->LoadSVGMainIcon(ui->GetTexturePack().outlinerObMeshSvgPath);
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
        
        RecordsPanel<ObjectButton>::AddRecord(record);
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

    void AddRecord
    (
        T object,
        const std::string &name,
        std::function<void()> onSelect,
        std::function<void()> onUnSelect
    ) { outliner->AddRecord(object, name, onSelect, onUnSelect); }

protected:
    void OnSizeChanged() override { layout(); }



private:
    void layout() {
        outliner->SetPos({0, TITLE_BAR_HEIGHT});
        outliner->SetSize(GetSize() - outliner->GetPos());
    }
};


} // namespace roa

#pragma once
#include <functional>
#include <optional>
#include <vector>
#include <string>
#include <cassert>

#include "BasicWidgets/Containers.hpp"
#include "BasicWidgets/ScrollBar.hpp"
#include "BasicWidgets/Buttons.hpp"
#include "BasicWidgets/TextWidgets.hpp"
#include "BasicWidgets/Window.hpp"
#include "CompositeWidgets/DropDownMenu.hpp"

namespace roa {

template<typename T>
concept IsPointer = std::is_pointer_v<T>;

template <WidgetDerived T>
class RecordsPanel : public Container {
    static constexpr float SCROLL_BAR_WIDTH = 6.0f;
    static constexpr float SCROLL_BAR_RIGHT_PADDING = 3.0f;
    static constexpr float SCROLL_BAR_HEIGHT_SHARE = 0.9f;

    float recordsPadding = 0.0f;
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
    }

    virtual ~RecordsPanel() = default;

    void SetRecordsPadding(float padding) { recordsPadding = padding; }
    void SetRecordsStartPos(const dr4::Vec2f pos) { recordsStartPos = pos; }
    void SetBGColor(const dr4::Color color) { BGColor = color; }
    std::vector<T*>& GetRecords() { return records; }

    void AddRecord(std::unique_ptr<T> record) {
        records.push_back(record.get());
        AddWidget(std::move(record));
        relayout();
    }

    void ClearRecords() {
        for (auto r : records) EraseWidget(r);
        records.clear();
        ForceRedraw();
    }

    float calculateTotalHeight() const {
        float sum = 0;
        for (auto r : records) sum += r->GetSize().y;
        return sum;
    }

    void relayout() {
        relayoutRecords();
        relayoutScrollBar();
        ForceRedraw();
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



private:
    void relayoutRecords() {
        dr4::Vec2f curPos = recordsStartPos;
        double scrollPct = scrollBar->GetPercentage();
        float scrollOffset = std::fmax(0.0f, scrollPct * (calculateTotalHeight() - GetSize().y));

        for (auto r : records) {
            r->SetPos(curPos - dr4::Vec2f(0, scrollOffset));
            r->ForceRedraw();
            curPos += dr4::Vec2f(0, r->GetSize().y + recordsPadding);
        }
    }

    void relayoutScrollBar() {
        float totalHeight = calculateTotalHeight();
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

// ---------------- ObjectButton ----------------
struct ObjectButtonColorPack {
    dr4::Color focusedHovered;
    dr4::Color focused;
    dr4::Color hovered;
    dr4::Color nonActive;
};

inline const ObjectButtonColorPack BLACK_OBJECT_PACK = {
    {78,100,145}, {51,77,128}, {68,68,68}, {40,40,40}
};

inline const ObjectButtonColorPack GRAY_OBJECT_PACK = {
    {78,100,145}, {51,77,128}, {71,71,71}, {43,43,43}
};

class ObjectButton final : public Button {
    ObjectButtonColorPack colorPack = BLACK_OBJECT_PACK;
    std::unique_ptr<dr4::Text> label;
    std::unique_ptr<dr4::Image> mainIcon;

public:
    explicit ObjectButton(hui::UI* ui) : Button(ui),
        label(ui->GetWindow()->CreateText()),
        mainIcon(ui->GetWindow()->CreateImage())
    {
        label->SetPos({40,3});
        label->SetFontSize(11);
        label->SetText("Figure.000");

        mainIcon->SetPos({20,2});
        mainIcon->SetSize({16,16});
    }

    void SetLabel(const std::string& text) { label->SetText(text); }
    void SetLabelFontSize(int fontSize) { label->SetFontSize(fontSize); }
    void LoadSVGMainIcon(const std::string& path) {
        ExtractSVG(path, mainIcon->GetSize(),
            [this](int x,int y,dr4::Color c){ mainIcon->SetPixel(x,y,c); });
    }

    void SetColorPack(const ObjectButtonColorPack pack) { colorPack = pack; }

protected:
    void Redraw() const override {
        GetTexture().Clear(FULL_TRANSPARENT);

        label->SetColor(dr4::Color(174,174,174,255));
        if (pressed && checkStateProperty(StateProperty::HOVERED)) {
            GetTexture().Clear(colorPack.focusedHovered);
            label->SetColor(dr4::Color(232,165,55,255));
        } else if (pressed) {
            GetTexture().Clear(colorPack.focused);
            label->SetColor(dr4::Color(232,165,55,255));
        } else if (checkStateProperty(StateProperty::HOVERED)) {
            GetTexture().Clear(colorPack.hovered);
        } else {
            GetTexture().Clear(colorPack.nonActive);
        }

        mainIcon->DrawOn(GetTexture());
        label->DrawOn(GetTexture());
    }
};

// ---------------- Outliner ----------------
template <IsPointer T>
class Outliner final : public RecordsPanel<ObjectButton> {
    static constexpr float RECORD_HEIGHT = 20.0f;
    std::optional<std::pair<std::string,T>> currentSelected = std::nullopt;
    std::function<void()> onSelectChangedAction = nullptr;
    Button::Mode recordButtonMode = Button::Mode::STICK_MODE;

public:
    using RecordsPanel<ObjectButton>::RecordsPanel;

    void AddRecord(T object, const std::string& name,
                   std::function<void()> onSelect,
                   std::function<void()> onUnSelect,
                   const std::string& iconPath="")
    {
        auto record = std::make_unique<ObjectButton>(GetUI());
        record->SetSize({GetSize().x, RECORD_HEIGHT});
        record->SetLabel(name);
        record->SetMode(recordButtonMode);

        if (iconPath.empty())
            record->LoadSVGMainIcon(static_cast<UI*>(GetUI())->GetTexturePack().outlinerObMeshSvgPath);
        else
            record->LoadSVGMainIcon(iconPath);

        record->SetOnPressAction([this,name,object,onSelect,onUnSelect]{
            if (onSelect) onSelect();
            if (!currentSelected.has_value() || currentSelected.value().second != object) {
                currentSelected = {name,object};
                if (onSelectChangedAction) onSelectChangedAction();
            }
        });

        record->SetOnUnpressAction([this,object,onUnSelect]{
            if (onUnSelect) onUnSelect();
            if (currentSelected.has_value() && currentSelected.value().second == object) {
                currentSelected.reset();
                if (onSelectChangedAction) onSelectChangedAction();
            }
        });

        if (this->GetRecords().size() % 2) record->SetColorPack(GRAY_OBJECT_PACK);
        else                               record->SetColorPack(BLACK_OBJECT_PACK);

        RecordsPanel<ObjectButton>::AddRecord(std::move(record));
    }

    void SetRecordButtonMode(Button::Mode mode) {
        recordButtonMode = mode;
        for (auto record : this->GetRecords()) record->SetMode(mode);
        ForceRedraw();
    }

    std::optional<std::pair<std::string,T>> GetSelected() { return currentSelected; }
    void SetOnSelectChangedAction(std::function<void()> action) { onSelectChangedAction = action; }
};

// ---------------- OutlinerWindow ----------------
template <IsPointer T>
class OutlinerWindow final : public Window {
    Outliner<T>* outliner = nullptr;

public:
    explicit OutlinerWindow(hui::UI* ui) : Window(ui) {
        auto outlinerUnique = std::make_unique<Outliner<T>>(ui);
        outliner = outlinerUnique.get();
        AddWidget(std::move(outlinerUnique));
    }

    void AddRecord(T object, const std::string& name,
                   std::function<void()> onSelect,
                   std::function<void()> onUnSelect,
                   const std::string& iconPath="")
    {
        outliner->AddRecord(object,name,onSelect,onUnSelect,iconPath);
    }

    std::optional<std::pair<std::string,T>> GetSelected() { return outliner->GetSelected(); }
    void SetOnSelectChangedAction(std::function<void()> action) { outliner->SetOnSelectChangedAction(action); }
    void SetRecordButtonMode(Button::Mode mode) { outliner->SetRecordButtonMode(mode); }

protected:
    void OnSizeChanged() override {
        outliner->SetPos({0, Window::TOOL_BAR_HEIGHT});
        outliner->SetSize(GetSize() - outliner->GetPos());
    }
};

} // namespace roa

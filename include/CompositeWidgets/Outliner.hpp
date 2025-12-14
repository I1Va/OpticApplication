#pragma once
#include "BasicWidgets/Buttons.hpp"
#include "BasicWidgets/TextWidgets.hpp"
#include "BasicWidgets/Window.hpp"
#include "CompositeWidgets/DropDownMenu.hpp"
#include "CompositeWidgets/RecordsPanel.hpp"

namespace roa
{

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
    Outliner(hui::UI *ui) : RecordsPanel<ObjectButton>(ui) {
        assert(ui);
    }

    void AddRecord(T object, const std::string& name,
                   std::function<void()> onSelect,
                   std::function<void()> onUnSelect,
                   const std::string& iconPath="")
    {
        auto record = std::make_unique<ObjectButton>(GetUI());
        record->SetLabel(name);
        record->SetMode(recordButtonMode);
        record->SetSize(GetSize().x, RECORD_HEIGHT);

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

        if (GetRecordCount() % 2) record->SetColorPack(GRAY_OBJECT_PACK);
        else                      record->SetColorPack(BLACK_OBJECT_PACK);

        RecordsPanel<ObjectButton>::AddRecord(std::move(record));
    }

    void SetRecordButtonMode(Button::Mode mode) {
        recordButtonMode = mode;
        for (auto record : records) record->SetMode(mode);
        ForceRedraw();
    }

    std::optional<std::pair<std::string,T>> GetSelected() { return currentSelected; }
    void SetOnSelectChangedAction(std::function<void()> action) { onSelectChangedAction = action; }

protected:
    void OnSizeChanged() override {
        relayout();
    }

    void relayout() {
        for (auto record : records) {
            record->SetSize(GetSize().x, RECORD_HEIGHT);
        }
        RecordsPanel<ObjectButton>::relayout();
    }
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
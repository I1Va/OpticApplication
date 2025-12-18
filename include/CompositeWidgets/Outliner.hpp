#pragma once
#include "BasicWidgets/Buttons.hpp"
#include "BasicWidgets/TextWidgets.hpp"
#include "BasicWidgets/Window.hpp"
#include "CompositeWidgets/DropDownMenu.hpp"
#include "CompositeWidgets/RecordsPanel.hpp"

namespace roa
{

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
    static constexpr float PADDING = 4;
    static constexpr double CARET_BLINK_DELTA_SECS = 0.5;

    ObjectButtonColorPack colorPack = BLACK_OBJECT_PACK;

    dr4::Vec2f iconStartPos = {20, 3};
    dr4::Vec2f iconSize = {16, 16};
    std::unique_ptr<dr4::Text> label;
    std::unique_ptr<dr4::Image> mainIcon;

    bool isRenaming = false;
    std::unique_ptr<TextInputWidget> renameInput;
    std::string originalLabel;
    bool renameRequested = false;

    std::function<void()> onDeleteAction = nullptr;
    std::function<void(const std::string&)> onRenameAction = nullptr;

public:
    explicit ObjectButton(hui::UI* ui)
        : Button(ui),
          label(ui->GetWindow()->CreateText()),
          mainIcon(ui->GetWindow()->CreateImage())
    {
        layout();
        label->SetText("Figure.000");
        SetMode(Button::Mode::STICK_MODE);
    }

    void SetIconStartPos(const dr4::Vec2f pos) {
        iconStartPos = pos;
        layout();
    }

    void SetIconSize(const dr4::Vec2f size) {
        iconSize = size;
        layout();
    }

    void SetOnDeleteAction(std::function<void()> action) { onDeleteAction = action; }
    void SetOnRenameAction(std::function<void(const std::string&)> action) { onRenameAction = action; }

    void SetLabel(const std::string& text) {
        label->SetText(text);
        ForceRedraw();
    }

    void SetLabelFontSize(int fontSize) {
        label->SetFontSize(fontSize);
        ForceRedraw();
    }

    void LoadSVGMainIcon(const std::string& path) {
        ExtractSVG(path, mainIcon->GetSize(),
            [this](int x, int y, dr4::Color c){ mainIcon->SetPixel(x, y, c); });
        ForceRedraw();
    }

    void SetColorPack(const ObjectButtonColorPack pack) {
        colorPack = pack;
        ForceRedraw();
    }

    std::string GetLabel() const { return label->GetText(); }
    bool IsRenaming() const { return isRenaming; }

    void StartRenaming() {
        if (isRenaming) return;

        isRenaming = true;
        originalLabel = label->GetText();

        renameInput = std::make_unique<TextInputWidget>(GetUI());
        renameInput->EnableCaret();
        renameInput->SetSize({GetSize().x - iconStartPos.x - iconSize.x - PADDING, GetSize().y - 6});
        renameInput->SetPos({iconStartPos.x + iconSize.x + PADDING, 3});
        renameInput->SetText(originalLabel);
        renameInput->SetBGColor(FULL_TRANSPARENT);
        renameInput->SetColor(dr4::Color(232,165,55,255));
        renameInput->SetFontSize(label->GetFontSize());
        renameInput->SetOnEnterAction([this](const std::string&) { FinishRenaming(true); });

        renameRequested = true;
        ForceRedraw();
    }

    void FinishRenaming(bool apply) {
        if (!isRenaming) return;

        renameInput->DisableCaret();
        if (apply && renameInput && !renameInput->GetText().empty()) {
            const std::string newName = renameInput->GetText();
            SetLabel(newName);
            if (onRenameAction) onRenameAction(newName);
        } else {
            SetLabel(originalLabel);
        }

        isRenaming = false;
        renameInput.reset();
        renameRequested = false;

        GetUI()->ReportFocus(this);
        ForceRedraw();
    }

protected:
    void Redraw() const override {
        GetTexture().Clear(FULL_TRANSPARENT);

        if (isRenaming && renameInput) {
            GetTexture().Clear(colorPack.focusedHovered);
            mainIcon->DrawOn(GetTexture());
            renameInput->DrawOn(GetTexture());
        } else {
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
    }

    hui::EventResult OnKeyDown(hui::KeyEvent &event) override {
        if (isRenaming && renameInput) {
            if (event.key == dr4::KeyCode::KEYCODE_ESCAPE) {
                FinishRenaming(false);
                return hui::EventResult::HANDLED;
            }
            if (event.key == dr4::KeyCode::KEYCODE_ENTER) {
                FinishRenaming(true);
                return hui::EventResult::HANDLED;
            }

            auto result = event.Apply(*renameInput);
            if (result == hui::EventResult::HANDLED) ForceRedraw();
            return result;
        }

        if (GetUI()->GetFocused() == this) {
            if (event.key == dr4::KeyCode::KEYCODE_F2 ||
                event.key == dr4::KeyCode::KEYCODE_ENTER) {
                StartRenaming();
                return hui::EventResult::HANDLED;
            }
            if (event.key == dr4::KeyCode::KEYCODE_DELETE) {
                if (onDeleteAction) onDeleteAction();
                return hui::EventResult::HANDLED;
            }
        }

        return hui::EventResult::UNHANDLED;
    }

    hui::EventResult OnText(hui::TextEvent &event) override {
        if (isRenaming && renameInput) {
            auto result = event.Apply(*renameInput);
            if (result == hui::EventResult::HANDLED) ForceRedraw();
            return result;
        }
        return hui::EventResult::UNHANDLED;
    }

    hui::EventResult OnMouseDown(hui::MouseButtonEvent &event) override {
        if (isRenaming && renameInput) {
            if (!GetRect().Contains(event.pos)) {
                FinishRenaming(true);
                return hui::EventResult::HANDLED;
            }
            auto adjusted = event;
            adjusted.pos -= renameInput->GetPos();
            auto result = adjusted.Apply(*renameInput);
            if (result == hui::EventResult::HANDLED) ForceRedraw();
            return result;
        }
        return Button::OnMouseDown(event);
    }

    hui::EventResult OnMouseUp(hui::MouseButtonEvent &event) override {
        if (isRenaming && renameInput) {
            auto adjusted = event;
            adjusted.pos -= renameInput->GetPos();
            auto result = adjusted.Apply(*renameInput);
            if (result == hui::EventResult::HANDLED) ForceRedraw();
            return result;
        }
        return Button::OnMouseUp(event);
    }

    hui::EventResult OnMouseMove(hui::MouseMoveEvent &event) override {
        if (isRenaming && renameInput) {
            auto adjusted = event;
            adjusted.pos -= renameInput->GetPos();
            return adjusted.Apply(*renameInput);
        }
        return Button::OnMouseMove(event);
    }

    hui::EventResult OnIdle(hui::IdleEvent &event) override {
        if (renameRequested && renameInput) renameRequested = false;

        if (isRenaming && renameInput) {
            auto result = event.Apply(*renameInput);
            static double acc = 0;
            acc += event.deltaTime;
            if (acc > 0.05) {
                ForceRedraw();
                acc = 0;
            }
            return result;
        }
        return Button::OnIdle(event);
    }

    void OnFocusLost() override {
        if (isRenaming && !renameRequested) FinishRenaming(true);
        Button::OnFocusLost();
    }

    void OnSizeChanged() override {
        layout();
        if (isRenaming && renameInput) {
            renameInput->SetSize({GetSize().x - iconStartPos.x - iconSize.x - PADDING, GetSize().y - 6});
            renameInput->SetPos({iconStartPos.x + iconSize.x + PADDING, 3});
        }
    }

private:
    void layout() {
        mainIcon->SetPos(iconStartPos);
        mainIcon->SetSize(iconSize);

        label->SetPos({
            mainIcon->GetSize().x + mainIcon->GetPos().x + PADDING,
            (GetSize().y - label->GetBounds().y) / 2
        });

        if (!label->GetFont())
            label->SetFont(GetUI()->GetWindow()->GetDefaultFont());
        if (label->GetFontSize() == 0)
            label->SetFontSize(14);

        label->SetColor(dr4::Color(174,174,174,255));
        ForceRedraw();
    }
};

template <IsPointer T>
class Outliner final : public RecordsPanel<ObjectButton> {
    static constexpr float RECORD_HEIGHT = 20.0f;

    dr4::Vec2f recordIconStartPos = {20, 3};
    dr4::Vec2f recordIconSize = {16, 16};
    int recordLabelFontSize = 14;

    std::optional<std::pair<std::string, T>> currentSelected = std::nullopt;
    std::function<void()> onSelectChangedAction = nullptr;
    std::function<void(T)> onDeleteAction = nullptr;
    Button::Mode recordButtonMode = Button::Mode::STICK_MODE;

public:
    Outliner(hui::UI* ui) : RecordsPanel<ObjectButton>(ui) {}

    void SetOnDeleteAction(std::function<void(T)> action) {
        onDeleteAction = action;
    }

    void AddRecord(T object, const std::string& name,
                   std::function<void()> onSelect,
                   std::function<void()> onUnSelect,
                   const std::string& iconPath = "")
    {
        auto record = std::make_unique<ObjectButton>(GetUI());
        record->SetLabel(name);
        record->SetLabelFontSize(recordLabelFontSize);
        record->SetMode(recordButtonMode);
        record->SetIconSize(recordIconSize);
        record->SetIconStartPos(recordIconStartPos);
        record->SetSize(GetSize().x, RECORD_HEIGHT);

        if (iconPath.empty())
            record->LoadSVGMainIcon(static_cast<UI*>(GetUI())->GetTexturePack().outlinerObMeshSvgPath);
        else
            record->LoadSVGMainIcon(iconPath);

        record->SetOnPressAction([this, name, object, onSelect, onUnSelect]{
            if (onSelect) onSelect();
            if (!currentSelected || currentSelected->second != object) {
                currentSelected = {name, object};
                if (onSelectChangedAction) onSelectChangedAction();
            }
        });

        record->SetOnUnpressAction([this, object, onUnSelect]{
            if (onUnSelect) onUnSelect();
            if (currentSelected && currentSelected->second == object) {
                currentSelected.reset();
                if (onSelectChangedAction) onSelectChangedAction();
            }
        });

        ObjectButton* recordPtr = record.get();
        record->SetOnDeleteAction([this, object, recordPtr]{
            if (onDeleteAction) onDeleteAction(object);
            if (currentSelected && currentSelected->second == object) {
                currentSelected.reset();
                if (onSelectChangedAction) onSelectChangedAction();
            }
            ClearRecord(recordPtr);
        });

        if (GetRecordCount() % 2) record->SetColorPack(GRAY_OBJECT_PACK);
        else record->SetColorPack(BLACK_OBJECT_PACK);

        RecordsPanel<ObjectButton>::AddRecord(std::move(record));
    }

    void ClearRecords() {
        currentSelected.reset();
        if (onSelectChangedAction) onSelectChangedAction();
        RecordsPanel::ClearRecords();
    }

    void SetRecordButtonMode(Button::Mode mode) {
        recordButtonMode = mode;
        for (auto r : records) r->SetMode(mode);
        ForceRedraw();
    }

    void SetRecordIconStartPos(const dr4::Vec2f pos) {
        recordIconStartPos = pos;
        for (auto r : records) r->SetIconStartPos(pos);
        ForceRedraw();
    }

    void SetRecordLabelFontSize(int fontSize) {
        recordLabelFontSize = fontSize;
        for (auto r : records) r->SetLabelFontSize(fontSize);
        ForceRedraw();
    }

    void SetRecordIconSize(const dr4::Vec2f size) {
        recordIconSize = size;
        for (auto r : records) r->SetIconSize(size);
        ForceRedraw();
    }

    std::optional<std::pair<std::string, T>> GetSelected() { return currentSelected; }
    void SetOnSelectChangedAction(std::function<void()> action) { onSelectChangedAction = action; }

protected:
    void OnSizeChanged() override { relayout(); }

    void relayout() {
        for (auto r : records)
            r->SetSize(GetSize().x, RECORD_HEIGHT);
        RecordsPanel<ObjectButton>::relayout();
    }
};

template <IsPointer T>
class OutlinerWindow final : public Window {
    Outliner<T>* outliner = nullptr;

public:
    explicit OutlinerWindow(hui::UI* ui) : Window(ui) {
        auto o = std::make_unique<Outliner<T>>(ui);
        outliner = o.get();
        AddWidget(std::move(o));
    }

    void AddRecord(T object, const std::string& name,
                   std::function<void()> onSelect,
                   std::function<void()> onUnSelect,
                   const std::string& iconPath = "")
    {
        outliner->AddRecord(object, name, onSelect, onUnSelect, iconPath);
    }

    void SetOnDeleteAction(std::function<void(T)> action) { outliner->SetOnDeleteAction(action); }
    void ClearRecords() { outliner->ClearRecords(); }

    std::optional<std::pair<std::string, T>> GetSelected() { return outliner->GetSelected(); }
    void SetOnSelectChangedAction(std::function<void()> action) { outliner->SetOnSelectChangedAction(action); }
    void SetRecordButtonMode(Button::Mode mode) { outliner->SetRecordButtonMode(mode); }

protected:
    void OnSizeChanged() override {
        outliner->SetPos({0, Window::TOOL_BAR_HEIGHT});
        outliner->SetSize(GetSize() - outliner->GetPos());
    }
};

} // namespace roa

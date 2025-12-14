#pragma once

#include "CompositeWidgets/RecordsPanel.hpp"
#include "CompositeWidgets/DropDownMenu.hpp"
#include "BasicWidgets/TextWidgets.hpp"

namespace roa {

// ---------------- PropertyField ----------------
class PropertyField final : public Container {
    std::unique_ptr<dr4::Text> label;
    TextInputWidget* inputField = nullptr;
    dr4::Color BGColor = FULL_TRANSPARENT;

public:
    explicit PropertyField(hui::UI* ui): 
        Container(ui),
        label(ui->GetWindow()->CreateText())
    {   
        auto inputFieldUnique = std::make_unique<TextInputWidget>(ui);
        inputField = inputFieldUnique.get();
        AddWidget(std::move(inputFieldUnique));

        auto* uiRef = static_cast<UI*>(GetUI());
        label->SetColor(uiRef->GetTexturePack().whiteTextColor);
        label->SetFontSize(uiRef->GetTexturePack().fontSize);
        inputField->SetFontSize(uiRef->GetTexturePack().fontSize);
    }

    virtual ~PropertyField() = default;

    void SetLabel(const std::string& content) {
        label->SetText(content);
        ForceRedraw();
    }

    void SetContent(const std::string& content) {
        inputField->SetText(content);
        ForceRedraw();
    }

    void SetBGColor(dr4::Color color) {
        BGColor = color;
        inputField->SetBGColor(BGColor);
        ForceRedraw();
    }

    void SetOnEnterAction(std::function<void(const std::string&)> action) {
        inputField->SetOnEnterAction(action);
    }

protected:
    void OnSizeChanged() override { relayout(); }

    void relayout() {
        float halfWidth = GetSize().x / 2.0f;
        inputField->SetSize({halfWidth, GetSize().y});
        inputField->SetPos({halfWidth, 0});
        inputField->SetBGColor(BGColor);
        inputField->ForceRedraw();
        ForceRedraw();
    }

    void Redraw() const override {
        GetTexture().Clear(BGColor);
        label->DrawOn(GetTexture());
        inputField->DrawOn(GetTexture());
    }
};

// ---------------- Property ----------------
class Property final : public DropDownMenu {
    static constexpr float RECORDS_PADDING = 1.0f;
    static constexpr float PANEL_PADDING = 30.0f;
    const dr4::Color BGColor = {61, 61, 61, 255};
    const dr4::Vec2f recordsStartPos = {16, 0};

    RecordsPanel<PropertyField>* fieldsPanel = nullptr;

public:
    explicit Property(hui::UI* ui) : DropDownMenu(ui)
    {
        auto fieldsPanelUnique = std::make_unique<RecordsPanel<PropertyField>>(ui);
        fieldsPanelUnique->SetRecordsPadding(RECORDS_PADDING);
        fieldsPanelUnique->SetRecordsStartPos(recordsStartPos);
        fieldsPanelUnique->SetBGColor(BGColor);
        fieldsPanelUnique->SetSize({30,200});

        fieldsPanel = fieldsPanelUnique.get();
        dropDown = fieldsPanel;
        AddWidget(std::move(fieldsPanelUnique));

        SetSize(300,100);
    }

    virtual ~Property() = default;

    void AddPropertyField(const std::string& label, const std::string& value,
                          std::function<void(const std::string&)> onEnterAction)
    {
        auto field = std::make_unique<PropertyField>(GetUI());
        field->SetSize(GetSize().x - 2 * recordsStartPos.x, 25);
        field->SetLabel(label);
        field->SetContent(value);
        field->SetBGColor(BGColor);
        field->SetOnEnterAction(onEnterAction);

        fieldsPanel->AddRecord(std::move(field));
    }

protected:
    void OnSizeChanged() override {
        DropDownMenu::OnSizeChanged();
        layout();
    }

private:
    void layout() {
        float height = fieldsPanel->CalculateRecordsSumHeight() + PANEL_PADDING;
        fieldsPanel->SetSize(GetSize().x, height);

        fieldsPanel->SetAllRecordsSize({GetSize().x - 2 * recordsStartPos.x, 25});
        ForceRedraw();
    }
};

// ---------------- PropertiesPanel ----------------
class PropertiesPanel final : public RecordsPanel<DropDownMenu> {
    const dr4::Vec2f RECORDS_START_POS = {8,16};

public:
    explicit PropertiesPanel(hui::UI* ui) : RecordsPanel(ui) {
        SetRecordsPadding(2);
        SetRecordsStartPos(RECORDS_START_POS);
        SetBGColor(static_cast<UI*>(ui)->GetTexturePack().propertiesPanelBGColor);
    }

    virtual ~PropertiesPanel() = default;

    void AddProperty(std::unique_ptr<Property> property) {
        property->SetOnSizeChangedAction([this](){ relayout(); });
        property->SetSize(GetSize().x - 2 * RECORDS_START_POS.x, 25);
        AddRecord(std::move(property));
    }
};

// ---------------- PropertiesWindow ----------------
class PropertiesWindow final : public Window {
    static constexpr float TITLE_BAR_HEIGHT = 20.0f;
    PropertiesPanel* panel = nullptr;

public:
    explicit PropertiesWindow(hui::UI* ui) : Window(ui)
    {
        auto panelUnique = std::make_unique<PropertiesPanel>(ui);
        panel = panelUnique.get();
        AddWidget(std::move(panelUnique));
    }

    virtual ~PropertiesWindow() = default;

    void AddProperty(std::unique_ptr<Property> property) { panel->AddProperty(std::move(property)); }
    void ClearRecords() { panel->ClearRecords(); }

protected:
    void OnSizeChanged() override { layout(); }

private:
    void layout() {
        panel->SetPos({0, TITLE_BAR_HEIGHT});
        panel->SetSize(GetSize() - panel->GetPos());
    }
};

} // namespace roa

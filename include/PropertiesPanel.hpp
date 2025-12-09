#pragma once

#include "RecordsPanel.hpp"
#include "DropDownMenu.hpp"
#include "BasicWidgets/TextWidgets.hpp"

namespace roa
{

class PropertyField final : public LinContainer<hui::Widget> {
    dr4::Text       *label;
    TextInputWidget *inputField;

public:
    PropertyField(hui::UI *ui) : LinContainer(ui), label(ui->GetWindow()->CreateText()), inputField(new TextInputWidget(ui)) { 
        assert(ui); 
        AddWidget(inputField);

        label->SetColor(static_cast<UI *>(GetUI())->GetTexturePack().whiteTextColor);
        label->SetFontSize(static_cast<UI *>(GetUI())->GetTexturePack().fontSize);
    }

    ~PropertyField() = default;

    void SetLabel(const std::string &content) {
        label->SetText(content);
        ForceRedraw();
    }
    void SetContent(const std::string &content) {
        inputField->SetText(content);
        ForceRedraw();
    }

    void SetOnEnterAction(std::function<void(const std::string &)> action) { inputField->SetOnEnterAction(action); }

protected:
    void OnSizeChanged() override {
        relayout();
    }

    void relayout() {
        float labelWIdth = GetSize().x / 2;
        float labelHeight = GetSize().y;
    
        float inputFieldWidth = labelWIdth;
        float inputFieldWidthHeight = GetSize().y;

        inputField->SetSize({inputFieldWidth, inputFieldWidthHeight}); 
        inputField->SetPos({labelWIdth, 0});

        inputField->ForceRedraw();
        ForceRedraw();

    }
    
    void Redraw() const override {
        GetTexture().Clear(FULL_TRANSPARENT);
        
        label->DrawOn(GetTexture());
        inputField->DrawOn(GetTexture());
    }
};

class Property final : public DropDownMenu {
    static constexpr float RECORDS_PADDING = 1;
    static constexpr float PROPERTY_FIELDS_PANEL_PADDING = 30;

    const dr4::Vec2f recordsStartPos = dr4::Vec2f(16, 0);

    RecordsPanel<PropertyField> *propertyFieldsPanel;

public:
    Property(hui::UI *ui): 
        DropDownMenu(ui),
        propertyFieldsPanel(new RecordsPanel<PropertyField>(ui))
    {
        assert(ui);
        
        propertyFieldsPanel->SetRecordsPadding(RECORDS_PADDING);
        propertyFieldsPanel->SetRecordsStartPos(recordsStartPos);
        propertyFieldsPanel->SetBGColor(static_cast<UI *>(ui)->GetTexturePack().propertiesPanelBGColor);
        
        propertyFieldsPanel->SetSize({30, 200});

        propertyFieldsPanel->SetBGColor({61, 61, 61});
        dropDown = propertyFieldsPanel;
        AddWidget(propertyFieldsPanel);
        
        SetSize(300, 100);
    }
    ~Property() = default;
    
    void AddPropertyField
    (
        const std::string &label, const std::string &value,
        std::function<void(const std::string&)> setPropertyVal
    ) {
        PropertyField *properyField = new PropertyField(GetUI());
        properyField->SetSize(GetSize().x - 2 * recordsStartPos.x, 25);
    
        properyField->SetLabel(label);
        properyField->SetContent(value);
        properyField->SetOnEnterAction(setPropertyVal);

        propertyFieldsPanel->AddRecord(properyField);
    }

protected:
    void OnSizeChanged() override { 
        DropDownMenu::OnSizeChanged();
        layout();
    }

    void layout() {
        propertyFieldsPanel->SetSize(GetSize().x, propertyFieldsPanel->calculateSummaryRecordsHeight() + PROPERTY_FIELDS_PANEL_PADDING);
        for (auto propertyField : propertyFieldsPanel->GetRecords()) {
           propertyField->SetSize(GetSize().x - 2 * recordsStartPos.x, 25);
        }

        propertyFieldsPanel->relayout();
        ForceRedraw();
    }
};

class PropertiesPanel final : public RecordsPanel<DropDownMenu> {
    const dr4::Vec2f RECORDS_START_POS = {8, 16};
public:
    
    PropertiesPanel(hui::UI *ui) : RecordsPanel(ui) {
        assert(ui);
    
        SetRecordsPadding(2);
        SetRecordsStartPos(RECORDS_START_POS);
        SetBGColor(static_cast<UI *>(ui)->GetTexturePack().propertiesPanelBGColor);
    }

    ~PropertiesPanel() = default;
    
    void AddProperty(Property *property) {
        assert(property);
        property->SetOnSizeChangedAction([this](){ relayout(); });
        property->SetSize(GetSize().x - 2 * RECORDS_START_POS.x, 25);
        
        records.push_back(property);
        AddWidget(property);
        relayout();
    }
};

class PropertiesWindow final : public Window {
    static constexpr float TITLE_BAR_HEIGHT = 20;
    PropertiesPanel *propertiesPanel;

public:
    PropertiesWindow(hui::UI *ui): Window(ui), propertiesPanel(new PropertiesPanel(ui)) {
        assert(ui);
        AddWidget(propertiesPanel);
    }

    ~PropertiesWindow() = default;  

    void AddProperty(Property *property) { 
        assert(property);
        propertiesPanel->AddProperty(property); 
    }

protected:
    void OnSizeChanged() override { layout(); }

private:
    void layout() {
        propertiesPanel->SetPos({0, TITLE_BAR_HEIGHT});
        propertiesPanel->SetSize(GetSize() - propertiesPanel->GetPos());
    }
};



} // namespace roa



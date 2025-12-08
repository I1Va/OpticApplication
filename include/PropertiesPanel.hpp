#pragma once

#include "RecordsPanel.hpp"
#include "DropDownMenu.hpp"



// #include "BasicWidgets/TextWidgets.hpp"
// #include "RecordsPanel.hpp"

namespace roa
{
   
class PropertiesPanel final : public RecordsPanel<DropDownMenu> {
public:
    using RecordsPanel::RecordsPanel;
    ~PropertiesPanel() = default;

    void AddProperty
    (
        const std::string &label, const std::string &value,
        std::function<void(const std::string&)> setPropertyVal
    ) {
        DropDownMenu *record = new DropDownMenu(GetUI());
        record->SetLabelText(label);
        record->SetSize(GetSize().x, 25);

        // record->SetLabel(label);
        // record->SetText(value);
        // record->SetOnEnterAction(setPropertyVal);

        // TextInputField* ptr = record.get();
        // records.emplace_back(std::move(record));
        // BecomeParentOf(ptr);
        // relayout();

        // ptr->ForceRedraw();

        records.push_back(record);
        AddWidget(record);
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

     void AddProperty
    (
        const std::string &label, const std::string &value,
        std::function<void(const std::string&)> setPropertyVal
    ) { propertiesPanel->AddProperty(label, value, setPropertyVal); }

protected:
    void OnSizeChanged() override { layout(); }

private:
    void layout() {
        propertiesPanel->SetPos({0, TITLE_BAR_HEIGHT});
        propertiesPanel->SetSize(GetSize() - propertiesPanel->GetPos());
        propertiesPanel->SetRecordsPadding(2);
    }
};



} // namespace roa



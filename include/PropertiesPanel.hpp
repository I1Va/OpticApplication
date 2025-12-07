#pragma once
#include "TextWidgets.hpp"
#include "RecordsPanel.hpp"

namespace roa
{
   
class PropertiesPanel final : public RecordsPanel<TextInputField> {
public:
    using RecordsPanel::RecordsPanel;
    ~PropertiesPanel() = default;

    void AddProperty
    (
        const std::string &label, const std::string &value,
        std::function<void(const std::string&)> setPropertyVal
    ) {
        // auto record = std::make_unique<TextInputField>(GetUI());
        // record->SetLabel(label);
        // record->SetText(value);
        // record->SetOnEnterAction(setPropertyVal);

        // TextInputField* ptr = record.get();
        // records.emplace_back(std::move(record));
        // BecomeParentOf(ptr);
        // relayout();

        // ptr->ForceRedraw();
    }
};

} // namespace roa



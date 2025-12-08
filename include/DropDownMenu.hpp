#pragma once
#include "Buttons.hpp"
#include "Containers.hpp"

namespace roa
{

class DropDownMenu : public LinContainer<hui::Widget> {
    ObjectButton *topButton;
    hui::Widget *dropDown;

public:
    DropDownMenu(hui::UI *ui): LinContainer(ui), topButton(new ObjectButton(ui))
    {
        AddWidget(topButton);
    }
};


} // namespace roa



// class ObjectButton final : public Button {
//     const dr4::Color labelNonFocusedColor = dr4::Color(174, 174, 174, 255); 
//     const dr4::Color labelFocusedColor    = dr4::Color(232, 165, 55, 255); 

//     ObjectButtonColorPack colorPack = BLACK_OBJECT_PACK;
//     // icon texture
//     // label
//     dr4::Text *label;

// public:
//     ObjectButton(hui::UI *ui): Button(ui), label(ui->GetWindow()->CreateText()) 
//     {
//         assert(ui);
//         assert(label);    
    
//         label->SetPos({40, 3}); // FIXME
//         label->SetFontSize(11);
//         label->SetText("Figure.000");
//         label->DrawOn(GetTexture());
//     }

//     ~ObjectButton() = default;
    
//     void SetColorPack(const ObjectButtonColorPack pack) {
//         colorPack = pack;
//     }

//     void SetLabelText(const std::string &text) { label->SetText(text); }
//     void SetLabelFontSize(const int fontSize) { label->SetFontSize(fontSize); }

// protected:
//     void Redraw() const override final {
//         GetTexture().Clear(FULL_TRANSPARENT);

//         label->SetColor(labelNonFocusedColor);
//         if (checkStateProperty(Button::StateProperty::FOCUSED) && checkStateProperty(Button::StateProperty::HOVERED)) {
//             GetTexture().Clear(colorPack.focusedHovered);
//             label->SetColor(labelFocusedColor);
//         } else if (checkStateProperty(Button::StateProperty::FOCUSED)) {
//             label->SetColor(labelFocusedColor);
//             GetTexture().Clear(colorPack.focused);
//         } else if (checkStateProperty(Button::StateProperty::HOVERED)) {
//             GetTexture().Clear(colorPack.hovered);
//         } else {
//             GetTexture().Clear(colorPack.nonActive);
//         }
    
//         label->DrawOn(GetTexture());
//     }
// };
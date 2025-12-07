#pragma once
#include <functional>
#include <algorithm>
#include <cmath>
#include <cassert>

#include "Containers.hpp"
#include "ROAGUIRender.hpp"
#include "ROACommon.hpp"

namespace roa
{

class ThumbButton : public RoundedBlenderButton  {
    dr4::Rect2f movingArea = {};

    dr4::Vec2f accumulatedRel = {};
    bool replaced = false;

    std::function<void()> onReplaceAction = nullptr;

public:
    using RoundedBlenderButton::RoundedBlenderButton;
    ~ThumbButton() = default;

    void SetMovingArea(const dr4::Rect2f rect) { 
        movingArea = rect;
    }

    void SetOnReplaceAction(std::function<void()> action) {
        onReplaceAction = action;
    }

protected:
    void OnIdleSelfAction(hui::IdleEvent &) override {
        if (replaced) {
            SetPos(GetPos() + accumulatedRel);
            clampPos();
            accumulatedRel = {0, 0};
            replaced = false;
            if (onReplaceAction) onReplaceAction();
            ForceRedraw();
        }
    }
    
    hui::EventResult OnMouseMove(hui::MouseMoveEvent &event) override {
        if (!GetRect().Contains(event.pos) && (GetUI()->GetCaptured() != this)) return hui::EventResult::UNHANDLED;

        GetUI()->ReportHover(this);

        if (GetUI()->GetFocused() == this && pressed) {
            accumulatedRel += event.rel;
            replaced = true;
            return hui::EventResult::HANDLED;
        }

        return hui::EventResult::UNHANDLED;
    }

    void OnPosChanged() {
        replaced = true;
    }

private:
    void clampPos() {
        dr4::Rect2f realMovingArea = movingArea;
        realMovingArea.size.x = std::fmax(0, realMovingArea.size.x - GetSize().x);
        realMovingArea.size.y = std::fmax(0, realMovingArea.size.y - GetSize().y);
        
        dr4::Vec2f pos = getClampedDotInRect(GetPos(), realMovingArea);
        SetPos(pos);
    }
};

class VerticalScrollBar : public ZContainer<hui::Widget> {
    static constexpr float MOUSEWHEEL_THUMB_MOVE_COEF = 0.1;

    std::function<void(double)> onScrollAction=nullptr;

    std::unique_ptr<ThumbButton> thumbButton  = nullptr;
    float thumbLayoutShare = 0.5;

    bool hiden = false;

public:
    VerticalScrollBar(hui::UI *ui): ZContainer(ui) {
        thumbButton = std::make_unique<ThumbButton>(ui);
        BecomeParentOf(thumbButton.get());
        thumbButton->SetOnReplaceAction([this] { percantageChanged(); });
    }

    ~VerticalScrollBar() = default;

    void BringToFront(hui::Widget *) override {}

    void SetOnScrollAction(std::function<void(double)> action) {
        onScrollAction = action;
    }

    void SetThumbLayoutShare(float share) {
        thumbLayoutShare = share;
        relayout();
    }

    double GetPercentage() { return calculateThumbPercentage(); }

    void Hide() { 
        hiden = true; 
        ForceRedraw();
    }

    void Show() { 
        hiden = false;
        ForceRedraw();
    }

protected:
    hui::EventResult PropagateToChildren(hui::Event &event) override {
        assert(thumbButton);

        if (event.Apply(*thumbButton) == hui::EventResult::HANDLED) return hui::EventResult::HANDLED;

        return hui::EventResult::UNHANDLED;
    }

    void OnSizeChanged() override { relayout(); }

protected:
    void Redraw() const override {
        GetTexture().Clear(RED);
        if (!hiden) {
            thumbButton->DrawOn(GetTexture());
        }
    }

    hui::EventResult OnMouseWheel(hui::MouseWheelEvent &event) override {
        if (GetUI()->GetHovered() == GetParent()        || 
            GetUI()->GetHovered() == this               ||
            GetUI()->GetHovered() == thumbButton.get()) {
                moveThumb(-event.delta.y * MOUSEWHEEL_THUMB_MOVE_COEF);
                return hui::EventResult::HANDLED; 
        } else {
             return hui::EventResult::UNHANDLED;
        }
    }

    hui::EventResult OnMouseDown(hui::MouseButtonEvent &event) override {
        if (GetRect().Contains(event.pos)) {
            event.pos -= GetPos();
            if (PropagateToChildren(event) == hui::EventResult::HANDLED) {
                event.pos += GetPos();
                return hui::EventResult::HANDLED;
            }

            if (calculateThumbMovingArea().Contains(event.pos)) {
                thumbButton->SetPos(event.pos);
                ForceRedraw();
            }

            event.pos += GetPos();
            GetUI()->ReportFocus(this);
            return hui::EventResult::HANDLED;
        }
        return hui::EventResult::UNHANDLED;
    }

private:
    dr4::Rect2f calculateThumbMovingArea() {
        dr4::Rect2f thumbMovingArea;
    
        thumbMovingArea.pos = dr4::Vec2f(0, 0);
        thumbMovingArea.size = dr4::Vec2f(GetSize());

        return thumbMovingArea;
    }

    dr4::Vec2f calculateThumbPos(double percentage) {
        percentage = std::clamp(percentage, 0.0, 1.0);
    
        dr4::Rect2f thumbMovingArea = calculateThumbMovingArea();
        double len = thumbMovingArea.size.y;
        double y = thumbMovingArea.pos.y + (len - thumbButton->GetSize().y) * percentage;

        return dr4::Vec2f(0, y);
    }

    double calculateThumbPercentage() {
         dr4::Rect2f thumbMovingArea = calculateThumbMovingArea();

        double y = thumbButton->GetPos().y - thumbMovingArea.pos.y;
        double len = calculateThumbMovingArea().size.y - thumbButton->GetSize().y;

        return y / len;
    }

    void initLayout() {
        assert(thumbButton);

        dr4::Rect2f thumbMovingArea = calculateThumbMovingArea();
        thumbButton->SetSize({thumbMovingArea.size.x, thumbMovingArea.size.y * thumbLayoutShare});
        thumbButton->SetPos(thumbMovingArea.pos);
        thumbButton->SetMovingArea(thumbMovingArea);
    }

    void relayout() {
        initLayout();
        
        double percentage = calculateThumbPercentage();
        dr4::Rect2f thumbMovingArea = calculateThumbMovingArea();

        thumbButton->SetPos({0, static_cast<float>(thumbMovingArea.pos.y + thumbMovingArea.size.y * percentage)});
        ForceRedraw();
    }

    void moveThumb(double deltaPercent) {
        double percentage = calculateThumbPercentage();
        double newPercentage = std::clamp(percentage + deltaPercent, 0.0, 1.0);

        if (onScrollAction) onScrollAction(newPercentage);

        dr4::Vec2f newThumbPos = calculateThumbPos(newPercentage);
        thumbButton->SetPos(newThumbPos);
        percentage = newPercentage;
    }

    void percantageChanged() {
        if (onScrollAction) onScrollAction(calculateThumbPercentage());
    }

};

} // namespace roa

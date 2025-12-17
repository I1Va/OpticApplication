#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <cassert>

#include "hui/widget.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/math/color.hpp"
#include "dr4/texture.hpp"
#include "dr4/event.hpp"
#include "pp/canvas.hpp"
#include "Utilities/ROACommon.hpp"
#include "BasicWidgets/Window.hpp"

namespace roa
{

class ColorPicker : public hui::Widget {
public:
    std::function<void(dr4::Color)> onColorChanged;

    ColorPicker(hui::UI *ui, const pp::ControlsTheme &theme = DefaultTheme())
        : hui::Widget(ui)
    {
        theme_ = theme;
        margin_ = 8.0f;
        gap_ = 8.0f;
        sliderWidth_ = 18.0f;
        knobRadius_ = 7.0f;
        hueCursorY_ = 0.0f;
        hueDegrees_ = 0.0f;
        sat_ = 1.0f;
        val_ = 1.0f;
        draggingSelector_ = false;
        draggingSlider_ = false;
        SetSize({150.0f, 150.0f});
        ComputeLayout();
        hueCursorY_ = 0.0f;
    }

    ~ColorPicker() override = default;

    void SetColor(const dr4::Color &c)
    {
        float h, s, v;
        RGBtoHSV(c, h, s, v);
        hueDegrees_ = h;
        sat_ = s;
        val_ = v;
        UpdateControlsFromHSV();
        ForceRedraw();
        if (onColorChanged) onColorChanged(GetColor());
    }

    dr4::Color GetColor() const
    {
        return HSVtoRGB(hueDegrees_, sat_, val_);
    }

protected:
    void OnSizeChanged() override { ComputeLayout(); }

    hui::EventResult OnMouseDown(hui::MouseButtonEvent &ev) override
    {
        if (!GetRect().Contains(ev.pos)) return hui::EventResult::UNHANDLED;
        dr4::Vec2f local = dr4::Vec2f(ev.pos.x, ev.pos.y) - GetPos();
        if (ev.button == dr4::MouseButtonType::LEFT) {
            if (SelectorHit(local)) {
                draggingSelector_ = true;
                GetUI()->ReportFocus(this);
                GetUI()->SetCaptured(this);
                return hui::EventResult::HANDLED;
            }
            if (SliderHit(local)) {
                draggingSlider_ = true;
                float y = local.y - sliderOrigin_.y;
                hueCursorY_ = std::clamp(y, 0.0f, sliderSize_.y - 1.0f);
                UpdateHSVFromControls();
                ForceRedraw();
                if (onColorChanged) onColorChanged(GetColor());
                GetUI()->ReportFocus(this);
                GetUI()->SetCaptured(this);
                return hui::EventResult::HANDLED;
            }
            if (PaletteHit(local)) {
                dr4::Vec2f p = local;
                p.x = std::clamp(p.x, paletteOrigin_.x, paletteOrigin_.x + paletteSize_.x - 1.0f);
                p.y = std::clamp(p.y, paletteOrigin_.y, paletteOrigin_.y + paletteSize_.y - 1.0f);
                selectorCenter_ = p;
                UpdateHSVFromControls();
                ForceRedraw();
                if (onColorChanged) onColorChanged(GetColor());
                GetUI()->ReportFocus(this);
                GetUI()->SetCaptured(this);
                return hui::EventResult::HANDLED;
            }
        }
        return hui::EventResult::UNHANDLED;
    }

    hui::EventResult OnMouseUp(hui::MouseButtonEvent &ev) override
    {
        if (ev.button == dr4::MouseButtonType::LEFT) {
            if (draggingSelector_) { draggingSelector_ = false; GetUI()->SetCaptured(nullptr); return hui::EventResult::HANDLED; }
            if (draggingSlider_)  { draggingSlider_  = false; GetUI()->SetCaptured(nullptr); return hui::EventResult::HANDLED; }
        }
        return hui::EventResult::UNHANDLED;
    }

    hui::EventResult OnMouseMove(hui::MouseMoveEvent &ev) override
    {
        if (!(GetUI()->GetCaptured() == this || GetRect().Contains(ev.pos))) return hui::EventResult::UNHANDLED;
        dr4::Vec2f rel(ev.rel.x, ev.rel.y);
        if (draggingSelector_) {
            selectorCenter_ = (selectorCenter_ + rel).Clamped(paletteOrigin_, paletteOrigin_ + paletteSize_ - dr4::Vec2f{1.0f,1.0f});
            UpdateHSVFromControls();
            ForceRedraw();
            if (onColorChanged) onColorChanged(GetColor());
            return hui::EventResult::HANDLED;
        }
        if (draggingSlider_) {
            float y = hueCursorY_ + rel.y;
            hueCursorY_ = std::clamp(y, 0.0f, sliderSize_.y - 1.0f);
            UpdateHSVFromControls();
            ForceRedraw();
            if (onColorChanged) onColorChanged(GetColor());
            return hui::EventResult::HANDLED;
        }
        return hui::EventResult::UNHANDLED;
    }

    void Redraw() const override
    {
        dr4::Texture &tex = GetTexture();
        tex.Clear(FULL_TRANSPARENT);
        dr4::Image *img = tex.GetImage();
        if (!img) return;
        int W = img->GetWidth();
        int H = img->GetHeight();
        for (int y = 0; y < H; ++y)
            for (int x = 0; x < W; ++x)
                img->SetPixel(x, y, dr4::Color(48,48,48,255));
        PaintPalette(*img);
        PaintSlider(*img);
        DrawOutline(*img, paletteOrigin_, paletteSize_);
        DrawOutline(*img, sliderOrigin_, sliderSize_);
        PaintKnob(*img, selectorCenter_, knobRadius_, theme_.handleColor, dr4::Color(255,255,255,220));
        dr4::Vec2f lineStart = sliderOrigin_ + dr4::Vec2f{0.0f, hueCursorY_};
        FillRect(*img, lineStart, dr4::Vec2f{sliderSize_.x, 3.0f}, theme_.handleColor);
        dr4::Vec2f previewPos = dr4::Vec2f{margin_, paletteOrigin_.y + paletteSize_.y + margin_};
        dr4::Vec2f previewSize = dr4::Vec2f{48.0f, 24.0f};
        FillRect(*img, previewPos, previewSize, GetColor());
        DrawOutline(*img, previewPos, previewSize);
        img->DrawOn(tex);
    }

private:
    pp::ControlsTheme theme_;
    dr4::Vec2f paletteOrigin_;
    dr4::Vec2f paletteSize_;
    float margin_;
    float gap_;
    float sliderWidth_;
    dr4::Vec2f sliderOrigin_;
    dr4::Vec2f sliderSize_;
    float knobRadius_;

    dr4::Vec2f selectorCenter_;
    float hueCursorY_;
    float hueDegrees_;
    float sat_;
    float val_;
    bool draggingSelector_;
    bool draggingSlider_;

    static pp::ControlsTheme DefaultTheme()
    {
        pp::ControlsTheme t;
        t.shapeFillColor = dr4::Color(200,200,200,255);
        t.shapeBorderColor = dr4::Color(80,80,80,255);
        t.textColor = dr4::Color(0,0,0,255);
        t.baseFontSize = 14;
        t.handleColor = dr4::Color(255,105,180,255);
        t.handleHoverColor = t.handleColor;
        t.handleActiveColor = t.handleColor;
        return t;
    }

    void ComputeLayout()
    {
        dr4::Vec2f s = GetSize();
        paletteOrigin_ = dr4::Vec2f{margin_, margin_};
        paletteSize_ = dr4::Vec2f{ s.x - margin_ * 2 - gap_ - sliderWidth_, s.y - margin_ * 3 - 40.0f };
        if (paletteSize_.x < 10.0f) paletteSize_.x = 10.0f;
        if (paletteSize_.y < 10.0f) paletteSize_.y = 10.0f;
        sliderOrigin_ = paletteOrigin_ + dr4::Vec2f{ paletteSize_.x + gap_, 0.0f };
        sliderSize_ = dr4::Vec2f{ sliderWidth_, paletteSize_.y };
        UpdateControlsFromHSV();
    }

    void PaintPalette(dr4::Image &img) const
    {
        int w = static_cast<int>(paletteSize_.x);
        int h = static_cast<int>(paletteSize_.y);
        int ox = static_cast<int>(paletteOrigin_.x);
        int oy = static_cast<int>(paletteOrigin_.y);
        if (w <= 0 || h <= 0) return;
        for (int yy = 0; yy < h; ++yy) {
            float v = 1.0f - (static_cast<float>(yy) / std::max(1, h - 1));
            for (int xx = 0; xx < w; ++xx) {
                float s = static_cast<float>(xx) / std::max(1, w - 1);
                img.SetPixel(ox + xx, oy + yy, HSVtoRGB(hueDegrees_, s, v));
            }
        }
    }

    void PaintSlider(dr4::Image &img) const
    {
        int w = static_cast<int>(sliderSize_.x);
        int h = static_cast<int>(sliderSize_.y);
        int ox = static_cast<int>(sliderOrigin_.x);
        int oy = static_cast<int>(sliderOrigin_.y);
        if (w <= 0 || h <= 0) return;
        for (int yy = 0; yy < h; ++yy) {
            float hh = (static_cast<float>(yy) / std::max(1, h - 1)) * 360.0f;
            dr4::Color c = HSVtoRGB(hh, 1.0f, 1.0f);
            for (int xx = 0; xx < w; ++xx) img.SetPixel(ox + xx, oy + yy, c);
        }
    }

    void DrawOutline(dr4::Image &img, dr4::Vec2f pos, dr4::Vec2f size) const
    {
        int x0 = static_cast<int>(pos.x);
        int y0 = static_cast<int>(pos.y);
        int x1 = static_cast<int>(pos.x + size.x) - 1;
        int y1 = static_cast<int>(pos.y + size.y) - 1;
        dr4::Color c = theme_.shapeBorderColor;
        for (int x = x0; x <= x1; ++x) { if (y0 >= 0 && y0 < img.GetHeight()) img.SetPixel(x, y0, c); if (y1 >= 0 && y1 < img.GetHeight()) img.SetPixel(x, y1, c); }
        for (int y = y0; y <= y1; ++y) { if (x0 >= 0 && x0 < img.GetWidth()) img.SetPixel(x0, y, c); if (x1 >= 0 && x1 < img.GetWidth()) img.SetPixel(x1, y, c); }
    }

    void FillRect(dr4::Image &img, dr4::Vec2f pos, dr4::Vec2f size, const dr4::Color &c) const
    {
        int ox = static_cast<int>(pos.x);
        int oy = static_cast<int>(pos.y);
        int w = static_cast<int>(size.x);
        int h = static_cast<int>(size.y);
        for (int yy = 0; yy < h; ++yy) for (int xx = 0; xx < w; ++xx) img.SetPixel(ox + xx, oy + yy, c);
    }

    void PaintKnob(dr4::Image &img, dr4::Vec2f center, float radius, const dr4::Color &fill, const dr4::Color &border) const
    {
        int cx = static_cast<int>(center.x);
        int cy = static_cast<int>(center.y);
        int r = static_cast<int>(std::ceil(radius));
        for (int y = cy - r; y <= cy + r; ++y) {
            for (int x = cx - r; x <= cx + r; ++x) {
                float dx = static_cast<float>(x) - center.x;
                float dy = static_cast<float>(y) - center.y;
                float d2 = dx*dx + dy*dy;
                if (d2 <= radius*radius) img.SetPixel(x, y, fill);
                else if (d2 <= (radius + 1.5f)*(radius + 1.5f)) img.SetPixel(x, y, border);
            }
        }
        int inner = std::max(1, r/2);
        dr4::Color innerColor = dr4::Color(0,0,0,120);
        for (int y = cy - inner; y <= cy + inner; ++y) for (int x = cx - inner; x <= cx + inner; ++x) img.SetPixel(x, y, innerColor);
        DrawOutline(img, dr4::Vec2f{float(cx - r), float(cy - r)}, dr4::Vec2f{float(r*2+1), float(r*2+1)});
    }

    bool PaletteHit(const dr4::Vec2f &p) const
    {
        return p.x >= paletteOrigin_.x && p.x < paletteOrigin_.x + paletteSize_.x && p.y >= paletteOrigin_.y && p.y < paletteOrigin_.y + paletteSize_.y;
    }

    bool SelectorHit(const dr4::Vec2f &p) const
    {
        dr4::Vec2f c = selectorCenter_;
        float dx = p.x - c.x;
        float dy = p.y - c.y;
        float r = knobRadius_;
        return (dx*dx + dy*dy) <= (r * r);
    }

    bool SliderHit(const dr4::Vec2f &p) const
    {
        dr4::Vec2f linePos = sliderOrigin_ + dr4::Vec2f{0.0f, hueCursorY_};
        return p.x >= sliderOrigin_.x && p.x < sliderOrigin_.x + sliderSize_.x && p.y >= linePos.y - 6.0f && p.y <= linePos.y + 6.0f;
    }

    void UpdateHSVFromControls()
    {
        float sx = (selectorCenter_.x - paletteOrigin_.x) / std::max(1.0f, paletteSize_.x - 1.0f);
        float vy = (selectorCenter_.y - paletteOrigin_.y) / std::max(1.0f, paletteSize_.y - 1.0f);
        sat_ = std::clamp(sx, 0.0f, 1.0f);
        val_ = std::clamp(1.0f - vy, 0.0f, 1.0f);
        float hr = sliderSize_.y > 1.0f ? (hueCursorY_ / (sliderSize_.y - 1.0f)) : 0.0f;
        hueDegrees_ = std::clamp(hr, 0.0f, 1.0f) * 360.0f;
    }

    void UpdateControlsFromHSV()
    {
        selectorCenter_.x = paletteOrigin_.x + sat_ * std::max(1.0f, paletteSize_.x - 1.0f);
        selectorCenter_.y = paletteOrigin_.y + (1.0f - val_) * std::max(1.0f, paletteSize_.y - 1.0f);
        float hr = hueDegrees_ / 360.0f;
        hueCursorY_ = hr * std::max(1.0f, sliderSize_.y - 1.0f);
    }

    static dr4::Color HSVtoRGB(float h, float s, float v)
    {
        if (s <= 1e-6f) {
            unsigned char vv = static_cast<unsigned char>(std::clamp(v, 0.0f, 1.0f) * 255.0f);
            return dr4::Color(vv, vv, vv, 255);
        }
        h = std::fmod(h, 360.0f);
        if (h < 0.0f) h += 360.0f;
        float hf = h / 60.0f;
        int i = static_cast<int>(std::floor(hf)) % 6;
        float f = hf - std::floor(hf);
        float p = v * (1.0f - s);
        float q = v * (1.0f - s * f);
        float t = v * (1.0f - s * (1.0f - f));
        float r=0,g=0,b=0;
        switch (i) {
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            default: r = v; g = p; b = q; break;
        }
        auto to8 = [](float c){ return static_cast<unsigned char>(std::clamp(c, 0.0f, 1.0f) * 255.0f); };
        return dr4::Color(to8(r), to8(g), to8(b), 255);
    }

    static void RGBtoHSV(const dr4::Color &c, float &h, float &s, float &v)
    {
        float rr = c.r / 255.0f;
        float gg = c.g / 255.0f;
        float bb = c.b / 255.0f;
        float mx = std::max({rr,gg,bb});
        float mn = std::min({rr,gg,bb});
        float d = mx - mn;
        v = mx;
        s = (mx <= 1e-6f) ? 0.0f : (d / mx);
        if (d <= 1e-6f) { h = 0.0f; return; }
        if (mx == rr) h = 60.0f * (std::fmod(((gg - bb) / d), 6.0f));
        else if (mx == gg) h = 60.0f * (((bb - rr) / d) + 2.0f);
        else h = 60.0f * (((rr - gg) / d) + 4.0f);
        if (h < 0.0f) h += 360.0f;
    }
};

class ColorPickerWindow final : public Window {
    static constexpr float TOOL_AREA = 0;
    ColorPicker *picker;
public:
    ColorPickerWindow(hui::UI *ui, const pp::ControlsTheme &theme = pp::ControlsTheme{})
        : Window(ui)
    {
        auto pickerUnique = std::make_unique<ColorPicker>(ui, theme);
        picker = pickerUnique.get();
        AddWidget(std::move(pickerUnique));
    }
    ~ColorPickerWindow() override = default;

    void OnSizeChanged() override { Arrange(); }

    void SetOnColorChangedAction(std::function<void(dr4::Color)> action) {
        picker->onColorChanged = action;
    }

private:
    void Arrange()
    {
        picker->SetPos({0.0f, TOOL_AREA});
        picker->SetSize(GetSize() - picker->GetPos());
    }
};

} // namespace roa

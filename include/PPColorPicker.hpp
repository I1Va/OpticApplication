#pragma once
// roa/ColorPicker.hpp
// Header-only ColorPicker widget for ROA GUI system (hui::Widget)

#include <algorithm>
#include <cmath>
#include <functional>
#include <cassert>

#include "hui/widget.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/math/color.hpp"
#include "dr4/texture.hpp"
#include "dr4/event.hpp"
#include "pp/canvas.hpp" // for ControlsTheme (used for handle color)
#include "Utilities/ROACommon.hpp"

namespace roa {

class ColorPicker : public hui::Widget {
public:
    // Callback called whenever color changes (after interaction).
    // Receives dr4::Color (RGBA).
    std::function<void(dr4::Color)> onColorChanged;

    // ctor: ui must be valid. theme supplies handle color / accent.
    ColorPicker(hui::UI *ui, const pp::ControlsTheme &theme = DefaultTheme())
        : hui::Widget(ui),
          theme_(theme),
          colorRectPos_{8, 8},
          hueRectGap_{8},
          hueRectWidth_{18},
          cornerPadding_{8},
          handleRadius_{7.0f},
          hueLineY_{0.0f},
          hue_{0.0f},
          saturation_{1.0f},
          value_{1.0f},
          isPointDragged_{false},
          isLineDragged_{false}
    {
        // sensible default size
        SetSize({260.0f, 160.0f});
        ComputeLayout();
        // init hueLineY for initial hue of 0
        hueLineY_ = 0.0f;
    }

    ~ColorPicker() override = default;

    // Set/get color from outside (color in dr4::Color, 0..255 channels)
    void SetColor(const dr4::Color &c) {
        float h, s, v;
        RGB2HSV(c, h, s, v);
        hue_ = h;
        saturation_ = s;
        value_ = v;
        // update handles accordingly and request redraw
        UpdateHandlesFromHSV();
        ForceRedraw();
        if (onColorChanged) onColorChanged(GetColor());
    }

    dr4::Color GetColor() const {
        return HSV2RGB(hue_, saturation_, value_);
    }

protected:
    // Widget overrides
    void OnSizeChanged() override {
        ComputeLayout();
    }

    hui::EventResult OnMouseDown(hui::MouseButtonEvent &evt) override {
        if (!GetRect().Contains(evt.pos)) return hui::EventResult::UNHANDLED;
    
        // evt.pos is local to this widget already (PropagateToChildren handles transformations)
        const dr4::Vec2f pos = dr4::Vec2f(evt.pos.x, evt.pos.y) - GetPos();

        if (evt.button == dr4::MouseButtonType::LEFT) {
            // point handle
            if (PointHitTest(pos)) {
                isPointDragged_ = true;
                return hui::EventResult::HANDLED;
            }
            // hue line
            if (HueLineHitTest(pos)) {
                isLineDragged_ = true;
                // move immediately to clicked position
                float localY = pos.y - hueRectPos_.y;
                hueLineY_ = std::clamp(localY, 0.0f, hueRectSize_.y - 1.0f);
                UpdateHSVFromHandles();
                ForceRedraw();
                if (onColorChanged) onColorChanged(GetColor());
                return hui::EventResult::HANDLED;
            }
            // clicking color rect moves point there
            if (ColorRectHitTest(pos)) {
                dr4::Vec2f clamped = pos;
                clamped.x = std::clamp(clamped.x, colorRectPos_.x, colorRectPos_.x + colorRectSize_.x - 1.0f);
                clamped.y = std::clamp(clamped.y, colorRectPos_.y, colorRectPos_.y + colorRectSize_.y - 1.0f);
                colorPointCenter_ = clamped;
                UpdateHSVFromHandles();
                ForceRedraw();
                if (onColorChanged) onColorChanged(GetColor());
                return hui::EventResult::HANDLED;
            }
        }
        return hui::EventResult::UNHANDLED;
    }

    hui::EventResult OnMouseUp(hui::MouseButtonEvent &evt) override {
        if (evt.button == dr4::MouseButtonType::LEFT) {
            if (isPointDragged_) { isPointDragged_ = false; return hui::EventResult::HANDLED; }
            if (isLineDragged_)  { isLineDragged_ = false;  return hui::EventResult::HANDLED; }
        }
        return hui::EventResult::UNHANDLED;
    }

    hui::EventResult OnMouseMove(hui::MouseMoveEvent &evt) override {
        // evt.pos is local; evt.rel is relative movement
        dr4::Vec2f pos(evt.pos.x, evt.pos.y);
        dr4::Vec2f rel(evt.rel.x, evt.rel.y);

        if (isPointDragged_) {
            colorPointCenter_ = (colorPointCenter_ + rel).Clamped(colorRectPos_, colorRectPos_ + colorRectSize_ - dr4::Vec2f{1.0f,1.0f});
            UpdateHSVFromHandles();
            ForceRedraw();
            if (onColorChanged) onColorChanged(GetColor());
            return hui::EventResult::HANDLED;
        }

        if (isLineDragged_) {
            float localY = hueLineY_ + rel.y;
            hueLineY_ = std::clamp(localY, 0.0f, hueRectSize_.y - 1.0f);
            UpdateHSVFromHandles();
            ForceRedraw();
            if (onColorChanged) onColorChanged(GetColor());
            return hui::EventResult::HANDLED;
        }

        return hui::EventResult::UNHANDLED;
    }

    void Redraw() const override {
        // draw background and controls into our widget texture
        dr4::Texture &tex = GetTexture();
        tex.Clear(FULL_TRANSPARENT);

        dr4::Image *img = tex.GetImage();
        if (!img) return;

        const int W = img->GetWidth();
        const int H = img->GetHeight();

        // background fill
        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x) {
                img->SetPixel(x, y, dr4::Color(48, 48, 48, 255));
            }
        }

        // draw color rectangle (left)
        DrawColorRect(*img);

        // draw hue rectangle (right)
        DrawHueRect(*img);

        // draw border outlines
        DrawRectOutline(*img, colorRectPos_, colorRectSize_);
        DrawRectOutline(*img, hueRectPos_, hueRectSize_);

        // draw color point handle (circle with border)
        DrawHandleCircle(*img, colorPointCenter_, handleRadius_, theme_.handleColor, dr4::Color(255,255,255,200));

        // draw hue line indicator (thin horizontal band across hueRect)
        dr4::Vec2f hlStart = hueRectPos_ + dr4::Vec2f{0.0f, hueLineY_};
        DrawRectFilled(*img, hlStart, dr4::Vec2f{hueRectSize_.x, 3.0f}, theme_.handleColor);

        // small preview square for current color
        dr4::Vec2f pvPos = dr4::Vec2f{cornerPadding_, colorRectPos_.y + colorRectSize_.y + cornerPadding_};
        dr4::Vec2f pvSize = dr4::Vec2f{48.0f, 24.0f};
        DrawRectFilled(*img, pvPos, pvSize, GetColor());
        DrawRectOutline(*img, pvPos, pvSize);

        img->DrawOn(tex);
    }

private:
    // Layout / config
    pp::ControlsTheme theme_;
    dr4::Vec2f colorRectPos_;
    dr4::Vec2f colorRectSize_;
    float hueRectGap_;
    float hueRectWidth_;
    dr4::Vec2f hueRectPos_;
    dr4::Vec2f hueRectSize_;
    float cornerPadding_;
    float handleRadius_;

    // Interaction state
    dr4::Vec2f colorPointCenter_; // absolute (local widget coords)
    float hueLineY_; // relative to hueRect (0..hueRectSize_.y)
    float hue_;       // 0..360
    float saturation_; // 0..1
    float value_;      // 0..1

    bool isPointDragged_;
    bool isLineDragged_;

private:
    // Default theme fallback
    static pp::ControlsTheme DefaultTheme() {
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

    // compute layout rectangles based on widget size
    void ComputeLayout() {
        dr4::Vec2f sz = GetSize();
        // leave padding around
        colorRectPos_ = dr4::Vec2f{cornerPadding_, cornerPadding_};
        colorRectSize_ = dr4::Vec2f{ sz.x - cornerPadding_ * 2 - hueRectGap_ - hueRectWidth_, sz.y - cornerPadding_ * 3 - 40.0f };
        if (colorRectSize_.x < 10.0f) colorRectSize_.x = 10.0f;
        if (colorRectSize_.y < 10.0f) colorRectSize_.y = 10.0f;

        hueRectPos_ = colorRectPos_ + dr4::Vec2f{ colorRectSize_.x + hueRectGap_, 0.0f };
        hueRectSize_ = dr4::Vec2f{ hueRectWidth_, colorRectSize_.y };

        // place color point according to current HSV
        UpdateHandlesFromHSV();
    }

    // drawing helpers
    void DrawColorRect(dr4::Image &img) const {
        int w = static_cast<int>(colorRectSize_.x);
        int h = static_cast<int>(colorRectSize_.y);
        int ox = static_cast<int>(colorRectPos_.x);
        int oy = static_cast<int>(colorRectPos_.y);

        if (w <= 0 || h <= 0) return;

        // For each pixel compute saturation (x: 0..1) and value (y: top -> 1, bottom -> 0)
        for (int yy = 0; yy < h; ++yy) {
            float v = 1.0f - (static_cast<float>(yy) / std::max(1, h - 1));
            for (int xx = 0; xx < w; ++xx) {
                float s = static_cast<float>(xx) / std::max(1, w - 1);
                dr4::Color c = HSV2RGB(hue_, s, v);
                img.SetPixel(ox + xx, oy + yy, c);
            }
        }
    }

    void DrawHueRect(dr4::Image &img) const {
        int w = static_cast<int>(hueRectSize_.x);
        int h = static_cast<int>(hueRectSize_.y);
        int ox = static_cast<int>(hueRectPos_.x);
        int oy = static_cast<int>(hueRectPos_.y);

        if (w <= 0 || h <= 0) return;

        // vertical hue: top -> hue=0, bottom-> hue=360
        for (int yy = 0; yy < h; ++yy) {
            float hue = (static_cast<float>(yy) / std::max(1, h - 1)) * 360.0f;
            dr4::Color col = HSV2RGB(hue, 1.0f, 1.0f);
            for (int xx = 0; xx < w; ++xx) {
                img.SetPixel(ox + xx, oy + yy, col);
            }
        }
    }

    void DrawRectOutline(dr4::Image &img, dr4::Vec2f pos, dr4::Vec2f size) const {
        int x0 = static_cast<int>(pos.x);
        int y0 = static_cast<int>(pos.y);
        int x1 = static_cast<int>(pos.x + size.x) - 1;
        int y1 = static_cast<int>(pos.y + size.y) - 1;
        dr4::Color c = theme_.shapeBorderColor;
        for (int x = x0; x <= x1; ++x) { if (y0 >= 0 && y0 < img.GetHeight()) img.SetPixel(x, y0, c); if (y1 >= 0 && y1 < img.GetHeight()) img.SetPixel(x, y1, c); }
        for (int y = y0; y <= y1; ++y) { if (x0 >= 0 && x0 < img.GetWidth()) img.SetPixel(x0, y, c); if (x1 >= 0 && x1 < img.GetWidth()) img.SetPixel(x1, y, c); }
    }

    void DrawRectFilled(dr4::Image &img, dr4::Vec2f pos, dr4::Vec2f size, const dr4::Color &c) const {
        int ox = static_cast<int>(pos.x);
        int oy = static_cast<int>(pos.y);
        int w = static_cast<int>(size.x);
        int h = static_cast<int>(size.y);
        for (int yy = 0; yy < h; ++yy) {
            for (int xx = 0; xx < w; ++xx) {
                img.SetPixel(ox + xx, oy + yy, c);
            }
        }
    }

    void DrawHandleCircle(dr4::Image &img, dr4::Vec2f center, float radius, const dr4::Color &fill, const dr4::Color &border) const {
        int cx = static_cast<int>(center.x);
        int cy = static_cast<int>(center.y);
        int r = static_cast<int>(std::ceil(radius));
        int x0 = cx - r;
        int x1 = cx + r;
        int y0 = cy - r;
        int y1 = cy + r;
        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                float dx = static_cast<float>(x) - center.x;
                float dy = static_cast<float>(y) - center.y;
                float d2 = dx*dx + dy*dy;
                if (d2 <= radius*radius) {
                    img.SetPixel(x, y, fill);
                } else if (d2 <= (radius+1.5f)*(radius+1.5f)) {
                    img.SetPixel(x, y, border);
                }
            }
        }
    }

    // Hit tests
    bool ColorRectHitTest(const dr4::Vec2f &p) const {
        return p.x >= colorRectPos_.x && p.x < colorRectPos_.x + colorRectSize_.x &&
               p.y >= colorRectPos_.y && p.y < colorRectPos_.y + colorRectSize_.y;
    }

    bool PointHitTest(const dr4::Vec2f &p) const {
        dr4::Vec2f c = colorPointCenter_;
        float dx = p.x - c.x;
        float dy = p.y - c.y;
        float r = handleRadius_;
        return (dx*dx + dy*dy) <= (r * r);
    }

    bool HueLineHitTest(const dr4::Vec2f &p) const {
        // a slightly larger hit band around the visible hue line
        dr4::Vec2f hlPos = hueRectPos_ + dr4::Vec2f{0.0f, hueLineY_};
        return p.x >= hueRectPos_.x && p.x < hueRectPos_.x + hueRectSize_.x &&
               p.y >= hlPos.y - 6.0f && p.y <= hlPos.y + 6.0f;
    }

    // Convert handle positions -> HSV, and vice versa
    void UpdateHSVFromHandles() {
        // saturation: left->0, right->1
        float sx = (colorPointCenter_.x - colorRectPos_.x) / std::max(1.0f, colorRectSize_.x - 1.0f);
        float vy = (colorPointCenter_.y - colorRectPos_.y) / std::max(1.0f, colorRectSize_.y - 1.0f);
        // invert Y: top = value 1
        saturation_ = std::clamp(sx, 0.0f, 1.0f);
        value_ = std::clamp(1.0f - vy, 0.0f, 1.0f);

        // hue: hueLineY_ from 0..hueRectSize_.y maps to 0..360
        float hr = hueRectSize_.y > 1.0f ? (hueLineY_ / (hueRectSize_.y - 1.0f)) : 0.0f;
        hue_ = std::clamp(hr, 0.0f, 1.0f) * 360.0f;
    }

    void UpdateHandlesFromHSV() {
        // color point: x from saturation, y from value (inverted)
        colorPointCenter_.x = colorRectPos_.x + saturation_ * std::max(1.0f, colorRectSize_.x - 1.0f);
        colorPointCenter_.y = colorRectPos_.y + (1.0f - value_) * std::max(1.0f, colorRectSize_.y - 1.0f);

        // hue line y from hue
        float hr = hue_ / 360.0f;
        hueLineY_ = hr * std::max(1.0f, hueRectSize_.y - 1.0f);
    }

    // Color conversions
    static dr4::Color HSV2RGB(float hue, float saturation, float value) {
        if (saturation <= 1e-6f) {
            unsigned char v = static_cast<unsigned char>(std::clamp(value, 0.0f, 1.0f) * 255.0f);
            return dr4::Color(v, v, v, 255);
        }
        // normalize
        hue = std::fmod(hue, 360.0f);
        if (hue < 0.0f) hue += 360.0f;

        float h = hue / 60.0f;
        int sector = static_cast<int>(std::floor(h)) % 6;
        float f = h - std::floor(h);

        float p = value * (1.0f - saturation);
        float q = value * (1.0f - saturation * f);
        float t = value * (1.0f - saturation * (1.0f - f));

        float r=0,g=0,b=0;
        switch (sector) {
            case 0: r = value; g = t;     b = p;     break;
            case 1: r = q;     g = value; b = p;     break;
            case 2: r = p;     g = value; b = t;     break;
            case 3: r = p;     g = q;     b = value; break;
            case 4: r = t;     g = p;     b = value; break;
            case 5: r = value; g = p;     b = q;     break;
        }
        auto to8 = [](float c){ return static_cast<unsigned char>(std::clamp(c, 0.0f, 1.0f) * 255.0f); };
        return dr4::Color(to8(r), to8(g), to8(b), 255);
    }

    // Convert 0..255 RGB to HSV (h in degrees 0..360)
    static void RGB2HSV(const dr4::Color &c, float &outH, float &outS, float &outV) {
        float r = c.r / 255.0f;
        float g = c.g / 255.0f;
        float b = c.b / 255.0f;
        float mx = std::max({r,g,b});
        float mn = std::min({r,g,b});
        float d = mx - mn;
        outV = mx;
        outS = (mx <= 1e-6f) ? 0.0f : (d / mx);

        if (d <= 1e-6f) {
            outH = 0.0f;
            return;
        }
        if (mx == r) outH = 60.0f * (std::fmod(((g - b) / d), 6.0f));
        else if (mx == g) outH = 60.0f * (((b - r) / d) + 2.0f);
        else outH = 60.0f * (((r - g) / d) + 4.0f);

        if (outH < 0.0f) outH += 360.0f;
    }
};

} // namespace roa
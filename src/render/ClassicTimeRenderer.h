#pragma once

#include <Arduino.h>
#include <M5EPD.h>

#include "SegmentRenderer.h"
#include "render/RenderPrimitives.h"

namespace render {

constexpr int16_t kClassicTimeDefaultX = 100;
constexpr int16_t kClassicTimeDefaultY = 72;
constexpr int16_t kClassicTimeCanvasW = 760;
constexpr int16_t kClassicTimeCanvasH = 300;
constexpr int16_t kClassicMinuteCanvasW = 400;
constexpr int16_t kClassicMinuteCanvasH = 292;

enum class ClassicTimeRenderTarget : uint8_t {
    None,
    Full,
    Minute,
};

struct ClassicTimeRenderState {
    String last_time_digits;
    uint8_t partial_count = 0;
};

struct ClassicTimeRenderInput {
    String time_digits;
    int16_t x = kClassicTimeDefaultX;
    int16_t y = kClassicTimeDefaultY;
    bool full_refresh = false;
    uint8_t partial_clean_interval = 1;
};

struct ClassicTimeRenderResult {
    bool rendered = false;
    ClassicTimeRenderTarget target = ClassicTimeRenderTarget::None;
    int16_t push_x = 0;
    int16_t push_y = 0;
    bool partial_update = false;
    DirtyRect dirty;
    m5epd_update_mode_t update_mode = UPDATE_MODE_GL16;
};

ClassicTimeRenderResult RenderClassicTime(
    const SegmentRenderer& renderer,
    M5EPD_Canvas& time_canvas,
    M5EPD_Canvas& minute_canvas,
    const ClassicTimeRenderInput& input,
    ClassicTimeRenderState& state);

}  // namespace render

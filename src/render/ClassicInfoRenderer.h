#pragma once

#include <Arduino.h>
#include <M5EPD.h>

#include "SegmentRenderer.h"
#include "render/RenderPrimitives.h"

namespace render {

constexpr int16_t kClassicInfoDefaultX = 100;
constexpr int16_t kClassicInfoDefaultY = 378;
constexpr int16_t kClassicInfoCanvasW = 760;
constexpr int16_t kClassicInfoCanvasH = 120;
constexpr int16_t kClassicHumidityCanvasW = 224;
constexpr int16_t kClassicHumidityCanvasH = kClassicInfoCanvasH;
constexpr int16_t kClassicTemperatureCanvasW = 268;
constexpr int16_t kClassicTemperatureCanvasH = kClassicInfoCanvasH;
constexpr int16_t kClassicComfortCanvasW = 240;
constexpr int16_t kClassicComfortCanvasH = kClassicInfoCanvasH;

enum class ClassicInfoRenderTarget : uint8_t {
    Humidity,
    Temperature,
    Comfort,
};

struct ClassicInfoRenderState {
    String last_humidity_text;
    String last_temperature_text;
    String last_comfort_face;
    uint8_t humidity_partial_count = 0;
    uint8_t temperature_partial_count = 0;
    uint8_t comfort_partial_count = 0;
};

struct ClassicInfoRenderInput {
    String humidity_text;
    String temperature_text;
    String comfort_face;
    int16_t x = kClassicInfoDefaultX;
    int16_t y = kClassicInfoDefaultY;
    bool full_refresh = false;
    bool use_cjk_font = false;
    uint8_t partial_clean_interval = 1;
};

struct ClassicInfoPartialUpdate {
    ClassicInfoRenderTarget target = ClassicInfoRenderTarget::Humidity;
    int16_t push_x = 0;
    int16_t push_y = 0;
    int16_t width = 0;
    int16_t height = 0;
    m5epd_update_mode_t update_mode = UPDATE_MODE_GL16;
};

struct ClassicInfoRenderResult {
    bool rendered = false;
    bool full_render = false;
    uint8_t partial_update_count = 0;
    ClassicInfoPartialUpdate partial_updates[3];
};

ClassicInfoRenderResult RenderClassicInfo(
    const SegmentRenderer& renderer,
    M5EPD_Canvas& info_canvas,
    M5EPD_Canvas& humidity_canvas,
    M5EPD_Canvas& temperature_canvas,
    M5EPD_Canvas& comfort_canvas,
    const ClassicInfoRenderInput& input,
    ClassicInfoRenderState& state);

}  // namespace render

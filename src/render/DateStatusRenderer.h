#pragma once

#include <Arduino.h>
#include <M5EPD.h>

#include "render/RenderPrimitives.h"

namespace render {

struct DateStatusRenderState {
    String last_date_text_rendered;
    String last_holiday_display_rendered;
    uint8_t last_weekday_rendered = 255;
    uint8_t partial_count = 0;
};

struct DateStatusRenderInput {
    String date_text;
    String weekday_text;
    String holiday_text;
    String render_signature;
    String pairing_code;
    String date_layout = "inline";
    uint8_t weekday = 255;
    uint8_t date_text_size = 7;
    bool show_holiday = false;
    bool use_cjk_font = false;
    bool full_refresh = false;
    uint8_t partial_clean_interval = 1;
};

struct DateStatusRenderResult {
    bool rendered = false;
    bool partial_update = false;
    DirtyRect dirty;
    m5epd_update_mode_t update_mode = UPDATE_MODE_GL16;
};

constexpr int16_t kDateStatusW = 744;
constexpr int16_t kDateStatusH = 44;

DateStatusRenderResult RenderDateStatus(M5EPD_Canvas& canvas,
                                        const DateStatusRenderInput& input,
                                        DateStatusRenderState& state);

void RenderFastDateStatus(M5EPD_Canvas& canvas, const String& date_text);

}  // namespace render

#pragma once

#include <Arduino.h>
#include <M5EPD.h>

namespace render {

struct DirtyRect {
    int16_t x = 0;
    int16_t y = 0;
    int16_t w = 0;
    int16_t h = 0;
    bool valid = false;
};

struct PartialRegion {
    int16_t update_x = 0;
    int16_t update_y = 0;
    int16_t draw_x = 0;
    int16_t draw_y = 0;
    int16_t update_w = 0;
    int16_t update_h = 0;
};

uint16_t uiTextPixelSize(uint8_t legacy_size);
void setCanvasTextSize(M5EPD_Canvas& canvas, bool use_cjk_font,
                       uint8_t legacy_size);
void includeDirtyRect(DirtyRect& dirty, const DirtyRect& rect);
PartialRegion makePartialRegion(int16_t x, int16_t y, int16_t w, int16_t h);
void updateAlignedArea(int16_t x, int16_t y, int16_t w, int16_t h,
                       m5epd_update_mode_t mode);
void pushAndRefreshCanvas(M5EPD_Canvas& canvas, int16_t x, int16_t y,
                          m5epd_update_mode_t mode);
m5epd_update_mode_t nextPartialMode(uint8_t& count, uint8_t clean_interval);

}  // namespace render

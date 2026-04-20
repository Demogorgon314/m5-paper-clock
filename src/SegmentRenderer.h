#pragma once

#include <Arduino.h>
#include <M5EPD.h>

class SegmentRenderer {
public:
    void drawText(M5EPD_Canvas& canvas, int16_t x, int16_t y, const String& text,
                  int16_t digit_width, int16_t digit_height, int16_t gap,
                  uint8_t body_color, uint8_t edge_color) const;
    int16_t measureText(const String& text, int16_t digit_width,
                        int16_t gap) const;

private:
    void drawDigit(M5EPD_Canvas& canvas, int16_t x, int16_t y, char digit,
                   int16_t digit_width, int16_t digit_height, uint8_t body_color,
                   uint8_t edge_color) const;
    void drawColon(M5EPD_Canvas& canvas, int16_t x, int16_t y,
                   int16_t digit_width, int16_t digit_height,
                   uint8_t color) const;
    void drawDot(M5EPD_Canvas& canvas, int16_t x, int16_t y,
                 int16_t digit_width, int16_t digit_height,
                 uint8_t color) const;
    void drawHorizontalSegment(M5EPD_Canvas& canvas, int16_t x, int16_t y,
                               int16_t length, int16_t thickness,
                               int16_t slant, uint8_t color) const;
    void drawVerticalSegment(M5EPD_Canvas& canvas, int16_t x, int16_t y,
                             int16_t length, int16_t thickness,
                             int16_t slant, uint8_t color) const;
};

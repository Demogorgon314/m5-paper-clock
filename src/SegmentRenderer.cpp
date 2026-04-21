#include "SegmentRenderer.h"

#include "logic/SegmentLogic.h"

namespace {

int16_t charWidth(char ch, int16_t digit_width) {
    if (ch == ':') {
        return digit_width / 3;
    }
    if (ch == '.') {
        return digit_width / 5;
    }
    return digit_width;
}

}  // namespace

void SegmentRenderer::drawText(M5EPD_Canvas& canvas, int16_t x, int16_t y,
                               const String& text, int16_t digit_width,
                               int16_t digit_height, int16_t gap,
                               uint8_t body_color, uint8_t edge_color) const {
    int16_t cursor_x = x;
    for (size_t i = 0; i < text.length(); ++i) {
        const char ch = text.charAt(i);
        if (ch >= '0' && ch <= '9') {
            drawDigit(canvas, cursor_x, y, ch, digit_width, digit_height,
                      body_color, edge_color);
        } else if (ch == '-') {
            drawMinus(canvas, cursor_x, y, digit_width, digit_height,
                      body_color, edge_color);
        } else if (ch == ':') {
            drawColon(canvas, cursor_x, y, digit_width, digit_height,
                      body_color);
        } else if (ch == '.') {
            drawDot(canvas, cursor_x, y, digit_width, digit_height, body_color);
        }
        cursor_x += charWidth(ch, digit_width) + gap;
    }
}

int16_t SegmentRenderer::measureText(const String& text, int16_t digit_width,
                                     int16_t gap) const {
    if (text.isEmpty()) {
        return 0;
    }

    int16_t total = 0;
    for (size_t i = 0; i < text.length(); ++i) {
        total += charWidth(text.charAt(i), digit_width);
        if (i + 1 < text.length()) {
            total += gap;
        }
    }
    return total;
}

void SegmentRenderer::drawDigit(M5EPD_Canvas& canvas, int16_t x, int16_t y,
                                char digit, int16_t digit_width,
                                int16_t digit_height, uint8_t body_color,
                                uint8_t edge_color) const {
    const uint8_t mask = logic::SegmentMaskForDigit(digit);
    const int16_t thickness = max<int16_t>(6, digit_width / 7);
    const int16_t slant = max<int16_t>(2, thickness / 2);
    const int16_t inner_inset = max<int16_t>(2, thickness / 5);
    const int16_t horizontal_length = digit_width - thickness;
    const int16_t vertical_length = (digit_height / 2) - (thickness / 2);

    auto draw_horizontal = [&](int16_t sx, int16_t sy) {
        drawHorizontalSegment(canvas, sx, sy, horizontal_length, thickness,
                              slant, edge_color);
        drawHorizontalSegment(canvas, sx + inner_inset, sy + inner_inset,
                              horizontal_length - inner_inset * 2,
                              thickness - inner_inset * 2,
                              max<int16_t>(1, slant - inner_inset), body_color);
    };

    auto draw_vertical = [&](int16_t sx, int16_t sy) {
        drawVerticalSegment(canvas, sx, sy, vertical_length, thickness, slant,
                            edge_color);
        drawVerticalSegment(canvas, sx + inner_inset, sy + inner_inset,
                            vertical_length - inner_inset * 2,
                            thickness - inner_inset * 2,
                            max<int16_t>(1, slant - inner_inset), body_color);
    };

    const int16_t top_x = x + thickness / 2;
    const int16_t upper_y = y + thickness / 2;
    const int16_t middle_y = y + (digit_height - thickness) / 2;
    const int16_t lower_y = y + digit_height / 2;
    const int16_t right_x = x + digit_width - thickness;

    if (mask & logic::kSegmentTop) {
        draw_horizontal(top_x, y);
    }
    if (mask & logic::kSegmentUpperLeft) {
        draw_vertical(x, upper_y);
    }
    if (mask & logic::kSegmentUpperRight) {
        draw_vertical(right_x, upper_y);
    }
    if (mask & logic::kSegmentMiddle) {
        draw_horizontal(top_x, middle_y);
    }
    if (mask & logic::kSegmentLowerLeft) {
        draw_vertical(x, lower_y);
    }
    if (mask & logic::kSegmentLowerRight) {
        draw_vertical(right_x, lower_y);
    }
    if (mask & logic::kSegmentBottom) {
        draw_horizontal(top_x, y + digit_height - thickness);
    }
}

void SegmentRenderer::drawColon(M5EPD_Canvas& canvas, int16_t x, int16_t y,
                                int16_t digit_width, int16_t digit_height,
                                uint8_t color) const {
    const int16_t radius = max<int16_t>(3, digit_width / 10);
    const int16_t center_x = x + (digit_width / 6);
    const int16_t upper_y = y + digit_height / 3;
    const int16_t lower_y = y + (digit_height * 2) / 3;
    canvas.fillCircle(center_x, upper_y, radius, color);
    canvas.fillCircle(center_x, lower_y, radius, color);
}

void SegmentRenderer::drawDot(M5EPD_Canvas& canvas, int16_t x, int16_t y,
                              int16_t digit_width, int16_t digit_height,
                              uint8_t color) const {
    const int16_t radius = max<int16_t>(3, digit_width / 12);
    canvas.fillCircle(x + radius, y + digit_height - radius * 2, radius,
                      color);
}

void SegmentRenderer::drawMinus(M5EPD_Canvas& canvas, int16_t x, int16_t y,
                                int16_t digit_width, int16_t digit_height,
                                uint8_t body_color,
                                uint8_t edge_color) const {
    const int16_t thickness = max<int16_t>(6, digit_width / 7);
    const int16_t slant = max<int16_t>(2, thickness / 2);
    const int16_t inner_inset = max<int16_t>(2, thickness / 5);
    const int16_t horizontal_length = digit_width - thickness;
    const int16_t middle_y = y + (digit_height - thickness) / 2;
    const int16_t top_x = x + thickness / 2;

    drawHorizontalSegment(canvas, top_x, middle_y, horizontal_length, thickness,
                          slant, edge_color);
    drawHorizontalSegment(canvas, top_x + inner_inset, middle_y + inner_inset,
                          horizontal_length - inner_inset * 2,
                          thickness - inner_inset * 2,
                          max<int16_t>(1, slant - inner_inset), body_color);
}

void SegmentRenderer::drawHorizontalSegment(M5EPD_Canvas& canvas, int16_t x,
                                            int16_t y, int16_t length,
                                            int16_t thickness, int16_t slant,
                                            uint8_t color) const {
    if (length <= 0 || thickness <= 0) {
        return;
    }

    canvas.fillRect(x + slant, y, length - slant * 2, thickness, color);
    canvas.fillTriangle(x, y + thickness / 2, x + slant, y, x + slant,
                        y + thickness, color);
    canvas.fillTriangle(x + length, y + thickness / 2, x + length - slant, y,
                        x + length - slant, y + thickness, color);
}

void SegmentRenderer::drawVerticalSegment(M5EPD_Canvas& canvas, int16_t x,
                                          int16_t y, int16_t length,
                                          int16_t thickness, int16_t slant,
                                          uint8_t color) const {
    if (length <= 0 || thickness <= 0) {
        return;
    }

    canvas.fillRect(x, y + slant, thickness, length - slant * 2, color);
    canvas.fillTriangle(x + thickness / 2, y, x, y + slant, x + thickness,
                        y + slant, color);
    canvas.fillTriangle(x + thickness / 2, y + length, x, y + length - slant,
                        x + thickness, y + length - slant, color);
}

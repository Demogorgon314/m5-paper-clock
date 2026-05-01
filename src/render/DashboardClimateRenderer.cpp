#include "render/DashboardClimateRenderer.h"

#include <algorithm>

namespace render {
namespace {

constexpr uint8_t kWhite = 0;
constexpr uint8_t kBorder = 11;
constexpr uint8_t kText = 15;
constexpr int16_t kDashboardClimateHumidityDividerX = 126;
constexpr int16_t kDashboardClimateTemperatureDividerX = 286;
constexpr int16_t kDashboardClimateContentY = 8;
constexpr int16_t kDashboardClimateContentH = 70;
constexpr int16_t kDashboardClimateHumidityAreaX = 8;
constexpr int16_t kDashboardClimateHumidityAreaW = 112;
constexpr int16_t alignUpTo4(int16_t value) {
    return (value + 3) & ~0x03;
}
constexpr int16_t kDashboardClimateTemperatureAreaX =
    alignUpTo4(kDashboardClimateHumidityDividerX + 4);
constexpr int16_t kDashboardClimateTemperatureAreaW = 148;
constexpr int16_t kDashboardClimateComfortAreaX =
    alignUpTo4(kDashboardClimateTemperatureDividerX + 4);
constexpr int16_t kDashboardClimateComfortAreaW = 156;

void drawCompactComfortInfoAt(M5EPD_Canvas& canvas, int16_t center_x,
                              int16_t center_y, const String& face,
                              uint8_t color, bool use_cjk_font) {
    const auto drawWideLine = [&](int16_t x0, int16_t y0, int16_t x1,
                                  int16_t y1, int16_t thickness) {
        const int16_t radius = thickness / 2;
        for (int16_t offset = -radius; offset <= radius; ++offset) {
            canvas.drawLine(x0, y0 + offset, x1, y1 + offset, color);
            if (offset != 0) {
                canvas.drawLine(x0 + offset, y0, x1 + offset, y1, color);
            }
        }
    };
    const auto drawCaret = [&](int16_t x, int16_t bottom_y, int16_t half_w,
                               int16_t rise, int16_t thickness) {
        drawWideLine(x - half_w, bottom_y, x, bottom_y - rise, thickness);
        drawWideLine(x + half_w, bottom_y, x, bottom_y - rise, thickness);
    };
    const auto drawDash = [&](int16_t x, int16_t y, int16_t half_w,
                              int16_t thickness) {
        const int16_t radius = thickness / 2;
        for (int16_t offset = -radius; offset <= radius; ++offset) {
            canvas.drawFastHLine(x - half_w, y + offset, half_w * 2 + 1,
                                 color);
        }
    };

    canvas.setTextDatum(CC_DATUM);
    canvas.setTextColor(color);
    setCanvasTextSize(canvas, use_cjk_font, 4);
    canvas.drawString("(", center_x - 40, center_y);
    canvas.drawString(")", center_x + 40, center_y);

    if (face == "(^_^)") {
        drawCaret(center_x - 18, center_y - 2, 6, 7, 4);
        drawCaret(center_x + 18, center_y - 2, 6, 7, 4);
        drawDash(center_x, center_y + 10, 7, 4);
        return;
    }

    if (face == "(-^-)") {
        drawDash(center_x - 20, center_y - 5, 6, 2);
        drawDash(center_x + 20, center_y - 5, 6, 2);
        drawCaret(center_x, center_y + 9, 6, 7, 2);
        return;
    }

    drawDash(center_x - 20, center_y - 5, 6, 2);
    drawDash(center_x + 20, center_y - 5, 6, 2);
    drawDash(center_x, center_y + 10, 7, 2);
}

}  // namespace

DashboardClimateRenderResult RenderDashboardClimate(
    M5EPD_Canvas& canvas,
    const DashboardClimateRenderInput& input,
    DashboardClimateRenderState& state) {
    DashboardClimateRenderResult result;
    const bool humidity_changed =
        input.humidity_text != state.last_humidity_text;
    const bool temperature_changed =
        input.temperature_text != state.last_temperature_text;
    const bool comfort_changed = input.comfort_face != state.last_comfort_face;

    if (!input.full_refresh && !humidity_changed && !temperature_changed &&
        !comfort_changed) {
        return result;
    }

    canvas.fillCanvas(kWhite);
    canvas.drawRoundRect(0, 0, kDashboardClimateStatusW,
                         kDashboardClimateStatusH, 6, kBorder);
    canvas.drawFastVLine(kDashboardClimateHumidityDividerX, 18, 50, kBorder);
    canvas.drawFastVLine(kDashboardClimateTemperatureDividerX, 18, 50,
                         kBorder);

    const int16_t climate_value_y = kDashboardClimateStatusH / 2 + 2;
    const int16_t humidity_x = 18;
    const int16_t temperature_x = 134;

    canvas.setTextDatum(CL_DATUM);
    canvas.setTextColor(kText);
    setCanvasTextSize(canvas, input.use_cjk_font, 5);
    canvas.drawString(input.humidity_text, humidity_x, climate_value_y);
    const int16_t humidity_width = canvas.textWidth(input.humidity_text);
    setCanvasTextSize(canvas, input.use_cjk_font, 2);
    canvas.drawString("%", humidity_x + humidity_width + 6,
                      climate_value_y + 14);

    setCanvasTextSize(canvas, input.use_cjk_font, 5);
    canvas.drawString(input.temperature_text, temperature_x, climate_value_y);
    const int16_t temperature_width = canvas.textWidth(input.temperature_text);
    const int16_t unit_x = temperature_x + temperature_width + 4;
    setCanvasTextSize(canvas, input.use_cjk_font, 2);
    canvas.drawString("o", unit_x, climate_value_y - 8);
    canvas.drawString("C", unit_x + 14, climate_value_y + 14);

    const int16_t face_cell_center_x =
        kDashboardClimateTemperatureDividerX +
        ((kDashboardClimateStatusW - kDashboardClimateTemperatureDividerX) / 2);
    drawCompactComfortInfoAt(canvas, face_cell_center_x,
                             kDashboardClimateStatusH / 2 + 2,
                             input.comfort_face, kText, input.use_cjk_font);

    if (input.full_refresh) {
        state.partial_count = 0;
    } else {
        int16_t dirty_left = kDashboardClimateStatusW;
        int16_t dirty_right = 0;
        const auto includeDirtyArea = [&](int16_t x, int16_t w) {
            dirty_left = std::min<int16_t>(dirty_left, x);
            dirty_right =
                std::max<int16_t>(dirty_right, static_cast<int16_t>(x + w));
        };
        if (humidity_changed) {
            includeDirtyArea(kDashboardClimateHumidityAreaX,
                             kDashboardClimateHumidityAreaW);
        }
        if (temperature_changed) {
            includeDirtyArea(kDashboardClimateTemperatureAreaX,
                             kDashboardClimateTemperatureAreaW);
        }
        if (comfort_changed) {
            includeDirtyArea(kDashboardClimateComfortAreaX,
                             kDashboardClimateComfortAreaW);
        }
        result.dirty.x = dirty_left;
        result.dirty.y = kDashboardClimateContentY;
        result.dirty.w = dirty_right - dirty_left;
        result.dirty.h = kDashboardClimateContentH;
        result.dirty.valid = dirty_right > dirty_left;
        result.update_mode =
            nextPartialMode(state.partial_count, input.partial_clean_interval);
        result.partial_update = result.dirty.valid;
    }

    state.last_humidity_text = input.humidity_text;
    state.last_temperature_text = input.temperature_text;
    state.last_comfort_face = input.comfort_face;
    result.rendered = true;
    return result;
}

}  // namespace render

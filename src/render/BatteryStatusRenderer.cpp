#include "render/BatteryStatusRenderer.h"

#include <algorithm>

#include "resources/wifi_bitmaps.h"

namespace render {
namespace {

constexpr uint8_t kWhite = 0;
constexpr uint8_t kText = 15;

void drawWifiStatusIcon(M5EPD_Canvas& canvas, int16_t origin_x, int16_t origin_y,
                        bool connected, uint8_t signal_level) {
    const WifiBitmap& bitmap = wifiBitmapForLevel(signal_level);
    canvas.pushImage(origin_x, origin_y, bitmap.width, bitmap.height, bitmap.data);

    if (connected) {
        return;
    }

    canvas.drawLine(origin_x + 5, origin_y + 26, origin_x + 26, origin_y + 5,
                    kText);
    canvas.drawLine(origin_x + 6, origin_y + 26, origin_x + 27, origin_y + 5,
                    kText);
}

void drawThickLine(M5EPD_Canvas& canvas, int16_t x0, int16_t y0, int16_t x1,
                   int16_t y1, uint8_t color) {
    canvas.drawLine(x0, y0, x1, y1, color);
    canvas.drawLine(x0 + 1, y0, x1 + 1, y1, color);
}

void drawUsbStatusIcon(M5EPD_Canvas& canvas, int16_t origin_x, int16_t origin_y) {
    const int16_t cx = origin_x + 9;
    canvas.fillTriangle(cx, origin_y + 1, origin_x + 6, origin_y + 7,
                        origin_x + 12, origin_y + 7, kText);
    canvas.fillRect(cx, origin_y + 7, 2, 11, kText);

    canvas.drawLine(cx, origin_y + 16, origin_x + 5, origin_y + 13, kText);
    canvas.drawLine(origin_x + 5, origin_y + 13, origin_x + 5,
                    origin_y + 11, kText);
    canvas.fillCircle(origin_x + 5, origin_y + 9, 2, kText);

    canvas.drawLine(cx + 1, origin_y + 14, origin_x + 15, origin_y + 10,
                    kText);
    canvas.drawLine(origin_x + 15, origin_y + 10, origin_x + 15,
                    origin_y + 8, kText);
    canvas.fillRect(origin_x + 13, origin_y + 5, 4, 4, kText);

    canvas.fillCircle(cx + 1, origin_y + 20, 2, kText);
}

void drawBluetoothStatusIcon(M5EPD_Canvas& canvas, int16_t origin_x,
                             int16_t origin_y) {
    const int16_t spine_x = origin_x + 8;
    const int16_t top_y = origin_y + 2;
    const int16_t mid_y = origin_y + 12;
    const int16_t bottom_y = origin_y + 22;
    const int16_t arm_x = origin_x + 16;

    drawThickLine(canvas, spine_x, top_y, spine_x, bottom_y, kText);
    drawThickLine(canvas, spine_x, top_y, arm_x, origin_y + 8, kText);
    drawThickLine(canvas, arm_x, origin_y + 8, spine_x, mid_y, kText);
    drawThickLine(canvas, spine_x, mid_y, arm_x, origin_y + 16, kText);
    drawThickLine(canvas, arm_x, origin_y + 16, spine_x, bottom_y, kText);
    drawThickLine(canvas, origin_x + 3, origin_y + 6, spine_x, mid_y, kText);
    drawThickLine(canvas, origin_x + 3, origin_y + 18, spine_x, mid_y, kText);
}

}  // namespace

BatteryStatusRenderResult RenderBatteryStatus(
    M5EPD_Canvas& canvas,
    const BatteryStatusRenderInput& input,
    BatteryStatusRenderState& state) {
    BatteryStatusRenderResult result;
    const bool battery_changed =
        input.battery_percentage != state.last_battery_percentage;
    const bool wifi_changed =
        input.wifi_connected != state.last_wifi_connected ||
        input.wifi_signal_level != state.last_wifi_signal_level;
    const bool transport_changed =
        input.config_connection_icon != state.last_config_connection_icon;

    if (!input.full_refresh && !battery_changed && !wifi_changed &&
        !transport_changed) {
        return result;
    }

    char battery_label[8];
    snprintf(battery_label, sizeof(battery_label), "%3u%%",
             input.battery_percentage);

    const int16_t body_w = 38;
    const int16_t body_h = 20;
    const int16_t cap_w = 4;
    const int16_t cap_h = 10;
    const int16_t icon_right = kBatteryStatusW - 2;
    const int16_t cap_x = icon_right - cap_w;
    const int16_t body_x = cap_x - body_w;
    const int16_t body_y = 12;
    const int16_t cap_y = body_y + 5;
    const int16_t inner_x = body_x + 3;
    const int16_t inner_y = body_y + 3;
    const int16_t inner_w = body_w - 6;
    const int16_t inner_h = body_h - 6;
    const int16_t fill_w = (input.battery_percentage * inner_w) / 100;
    const int16_t label_right = body_x - 10;

    canvas.fillCanvas(kWhite);
    canvas.setTextColor(kText);
    setCanvasTextSize(canvas, input.use_cjk_font, 2);
    canvas.setTextDatum(CR_DATUM);
    const int16_t label_width = canvas.textWidth(String(battery_label));
    const int16_t wifi_gap = 8;
    const int16_t wifi_size = 32;
    const int16_t wifi_x = label_right - label_width - wifi_gap - wifi_size;
    const int16_t wifi_y = 6;
    const int16_t transport_gap = 10;
    const int16_t transport_size = 18;
    const int16_t transport_x = wifi_x - transport_gap - transport_size;
    const int16_t transport_y = 10;
    canvas.drawString(String(battery_label), label_right, kBatteryStatusH / 2);
    if (input.config_connection_icon == ConfigConnectionIcon::Serial) {
        drawUsbStatusIcon(canvas, transport_x, transport_y);
    } else if (input.config_connection_icon == ConfigConnectionIcon::Bluetooth) {
        drawBluetoothStatusIcon(canvas, transport_x, transport_y);
    }
    drawWifiStatusIcon(canvas, wifi_x, wifi_y, input.wifi_connected,
                       input.wifi_signal_level);
    canvas.drawRoundRect(body_x, body_y, body_w, body_h, 3, kText);
    canvas.fillRect(cap_x, cap_y, cap_w, cap_h, kText);
    if (fill_w > 0) {
        canvas.fillRect(inner_x, inner_y, fill_w, inner_h, kText);
    }

    if (input.full_refresh) {
        state.partial_count = 0;
    } else {
        if (battery_changed) {
            DirtyRect rect;
            rect.x = std::max<int16_t>(0, label_right - 72);
            rect.y = 0;
            rect.w = kBatteryStatusW - rect.x;
            rect.h = kBatteryStatusH;
            rect.valid = true;
            includeDirtyRect(result.dirty, rect);
        }
        if (wifi_changed) {
            DirtyRect rect;
            rect.x = std::max<int16_t>(0, wifi_x - 4);
            rect.y = 0;
            rect.w =
                std::min<int16_t>(kBatteryStatusW - rect.x, wifi_size + 8);
            rect.h = kBatteryStatusH;
            rect.valid = true;
            includeDirtyRect(result.dirty, rect);
        }
        if (transport_changed) {
            DirtyRect rect;
            rect.x = std::max<int16_t>(0, transport_x - 4);
            rect.y = 0;
            rect.w =
                std::min<int16_t>(kBatteryStatusW - rect.x, transport_size + 8);
            rect.h = kBatteryStatusH;
            rect.valid = true;
            includeDirtyRect(result.dirty, rect);
        }
        if (input.forced_dirty && !result.dirty.valid) {
            result.dirty.x = 0;
            result.dirty.y = 0;
            result.dirty.w = kBatteryStatusW;
            result.dirty.h = kBatteryStatusH;
            result.dirty.valid = true;
        }
        result.update_mode =
            nextPartialMode(state.partial_count, input.partial_clean_interval);
        result.partial_update = result.dirty.valid;
    }

    state.last_battery_percentage = input.battery_percentage;
    state.last_wifi_connected = input.wifi_connected;
    state.last_wifi_signal_level = input.wifi_signal_level;
    state.last_config_connection_icon = input.config_connection_icon;
    result.rendered = true;
    return result;
}

}  // namespace render

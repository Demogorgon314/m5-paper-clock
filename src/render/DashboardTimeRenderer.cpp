#include "render/DashboardTimeRenderer.h"

namespace render {
namespace {

constexpr uint8_t kWhite = 0;
constexpr uint8_t kText = 15;
constexpr uint8_t kTimeEdgeText = 8;
constexpr int16_t kDashboardTimeDigitW = 92;
constexpr int16_t kDashboardTimeDigitH = 210;
constexpr int16_t kDashboardTimeGap = 18;
constexpr int16_t kDashboardTimeColonW = kDashboardTimeDigitW / 3;
constexpr int16_t kDashboardTimeTextW =
    (kDashboardTimeDigitW * 4) + kDashboardTimeColonW +
    (kDashboardTimeGap * 4);
constexpr int16_t kDashboardTimeDrawX =
    kDashboardTimeStatusW > kDashboardTimeTextW
        ? ((kDashboardTimeStatusW - kDashboardTimeTextW) / 2)
        : 0;
constexpr int16_t kDashboardTimeDigitY =
    (kDashboardTimeStatusH - kDashboardTimeDigitH) / 2;
constexpr int16_t kDashboardMinuteLocalX =
    kDashboardTimeDrawX + ((kDashboardTimeDigitW + kDashboardTimeGap) * 2);
constexpr int16_t kDashboardMinuteDrawW =
    kDashboardTimeColonW + kDashboardTimeGap + kDashboardTimeDigitW +
    kDashboardTimeGap + kDashboardTimeDigitW;

void drawDashboardMinuteTime(const SegmentRenderer& renderer,
                             M5EPD_Canvas& canvas,
                             const String& time_digits,
                             int16_t draw_x) {
    canvas.fillCanvas(kWhite);
    const String minute_text =
        ":" + time_digits.substring(2, 3) + time_digits.substring(3, 4);
    renderer.drawText(canvas, draw_x, kDashboardTimeDigitY, minute_text,
                      kDashboardTimeDigitW, kDashboardTimeDigitH,
                      kDashboardTimeGap, kText, kTimeEdgeText);
}

}  // namespace

DashboardTimeRenderResult RenderDashboardTime(
    const SegmentRenderer& renderer,
    M5EPD_Canvas& time_canvas,
    M5EPD_Canvas& minute_canvas,
    const DashboardTimeRenderInput& input,
    DashboardTimeRenderState& state) {
    DashboardTimeRenderResult result;
    if (!input.full_refresh && input.time_digits == state.last_time_digits) {
        return result;
    }

    const bool can_refresh_minutes_only =
        !input.full_refresh && state.last_time_digits.length() == 4 &&
        input.time_digits.substring(0, 2) ==
            state.last_time_digits.substring(0, 2);

    if (can_refresh_minutes_only) {
        const PartialRegion region =
            makePartialRegion(input.x + kDashboardMinuteLocalX, input.y,
                              kDashboardMinuteDrawW, kDashboardMinuteStatusH);
        const int16_t minute_draw_x =
            (input.x + kDashboardMinuteLocalX) - region.update_x;
        drawDashboardMinuteTime(renderer, minute_canvas, input.time_digits,
                                minute_draw_x);
        result.target = DashboardTimeRenderTarget::Minute;
        result.push_x = region.update_x;
        result.push_y = input.y;
        result.update_mode =
            nextPartialMode(state.partial_count, input.partial_clean_interval);
        result.partial_update = true;
        result.dirty.x = 0;
        result.dirty.y = 0;
        result.dirty.w = minute_canvas.width();
        result.dirty.h = minute_canvas.height();
        result.dirty.valid = true;
        state.last_time_digits = input.time_digits;
        result.rendered = true;
        return result;
    }

    const String time_text =
        input.time_digits.substring(0, 2) + ":" + input.time_digits.substring(2);
    time_canvas.fillCanvas(kWhite);
    renderer.drawText(time_canvas, kDashboardTimeDrawX, kDashboardTimeDigitY,
                      time_text, kDashboardTimeDigitW, kDashboardTimeDigitH,
                      kDashboardTimeGap, kText, kTimeEdgeText);
    result.target = DashboardTimeRenderTarget::Full;
    result.push_x = input.x;
    result.push_y = input.y;
    if (input.full_refresh) {
        state.partial_count = 0;
    } else {
        result.update_mode =
            nextPartialMode(state.partial_count, input.partial_clean_interval);
        result.partial_update = true;
        result.dirty.x = 0;
        result.dirty.y = 0;
        result.dirty.w = kDashboardTimeStatusW;
        result.dirty.h = kDashboardTimeStatusH;
        result.dirty.valid = true;
    }
    state.last_time_digits = input.time_digits;
    result.rendered = true;
    return result;
}

}  // namespace render

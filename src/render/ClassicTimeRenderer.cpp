#include "render/ClassicTimeRenderer.h"

namespace render {
namespace {

constexpr uint8_t kWhite = 0;
constexpr uint8_t kText = 15;
constexpr uint8_t kTimeEdgeText = 8;
constexpr int16_t kTimeDigitWidth = 150;
constexpr int16_t kTimeDigitHeight = 280;
constexpr int16_t kTimeGap = 24;
constexpr int16_t kTimeDigitY = 10;
constexpr int16_t kTimeColonX = 355;
constexpr int16_t kTimeDigitXs[] = {7, 181, 429, 603};

void drawMinuteTime(const SegmentRenderer& renderer, M5EPD_Canvas& canvas,
                    const String& time_digits, int16_t draw_x) {
    if (time_digits.length() != 4) {
        return;
    }

    const String minute_text = ":" + time_digits.substring(2);
    canvas.fillCanvas(kWhite);
    canvas.setTextDatum(TL_DATUM);
    canvas.setTextColor(kText);
    renderer.drawText(canvas, draw_x, kTimeDigitY, minute_text,
                      kTimeDigitWidth, kTimeDigitHeight, kTimeGap, kText,
                      kTimeEdgeText);
}

}  // namespace

ClassicTimeRenderResult RenderClassicTime(
    const SegmentRenderer& renderer,
    M5EPD_Canvas& time_canvas,
    M5EPD_Canvas& minute_canvas,
    const ClassicTimeRenderInput& input,
    ClassicTimeRenderState& state) {
    ClassicTimeRenderResult result;
    if (!input.full_refresh && input.time_digits == state.last_time_digits) {
        return result;
    }

    const bool can_refresh_minutes_only =
        !input.full_refresh && state.last_time_digits.length() == 4 &&
        input.time_digits.substring(0, 2) ==
            state.last_time_digits.substring(0, 2);

    if (can_refresh_minutes_only) {
        const PartialRegion region =
            makePartialRegion(input.x + kTimeColonX, input.y,
                              (kTimeDigitXs[3] + kTimeDigitWidth) -
                                  kTimeColonX,
                              kClassicMinuteCanvasH);
        const int16_t minute_draw_x = (input.x + kTimeColonX) - region.update_x;
        drawMinuteTime(renderer, minute_canvas, input.time_digits,
                       minute_draw_x);
        result.target = ClassicTimeRenderTarget::Minute;
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
    renderer.drawText(time_canvas, kTimeDigitXs[0], kTimeDigitY, time_text,
                      kTimeDigitWidth, kTimeDigitHeight, kTimeGap, kText,
                      kTimeEdgeText);
    result.target = ClassicTimeRenderTarget::Full;
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
        result.dirty.w = kClassicTimeCanvasW;
        result.dirty.h = kClassicTimeCanvasH;
        result.dirty.valid = true;
    }
    state.last_time_digits = input.time_digits;
    result.rendered = true;
    return result;
}

}  // namespace render

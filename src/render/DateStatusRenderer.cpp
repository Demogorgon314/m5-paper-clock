#include "render/DateStatusRenderer.h"

#include <algorithm>

namespace render {
namespace {

constexpr uint8_t kWhite = 0;
constexpr uint8_t kText = 15;
constexpr uint8_t kMinimumDateTextSize = 2;
constexpr int16_t kDateCjkWeekdayX = 142;
constexpr int16_t kDateCjkHolidayX = 222;
constexpr int16_t alignDownTo4(int16_t value) {
    return value & ~0x03;
}
constexpr int16_t alignUpTo4(int16_t value) {
    return (value + 3) & ~0x03;
}
constexpr int16_t kDateDateAreaX = 0;
constexpr int16_t kDateDateAreaW = alignUpTo4(kDateCjkWeekdayX + 8);
constexpr int16_t kDateWeekdayAreaX = alignDownTo4(kDateCjkWeekdayX - 8);
constexpr int16_t kDateWeekdayAreaW =
    alignUpTo4((kDateCjkHolidayX + 8) - kDateWeekdayAreaX);

void drawFauxBoldString(M5EPD_Canvas& canvas, const String& text, int16_t x,
                        int16_t y, int16_t offset_x = 1) {
    canvas.drawString(text, x, y);
    if (offset_x != 0 && !text.isEmpty()) {
        canvas.drawString(text, x + offset_x, y);
    }
}

String joinDateTextParts(const String& first, const String& second,
                         const String& third) {
    String output;
    const String parts[] = {first, second, third};
    for (const String& part : parts) {
        if (part.isEmpty()) {
            continue;
        }
        if (!output.isEmpty()) {
            output += "  ";
        }
        output += part;
    }
    return output;
}

uint8_t fitDateTextSize(M5EPD_Canvas& canvas, const String& text,
                        uint8_t preferred_size, uint8_t minimum_size,
                        int16_t max_width) {
    constexpr uint8_t kDateTextSizeCandidates[] = {7, 3, 2};
    for (uint8_t size : kDateTextSizeCandidates) {
        if (size > preferred_size || size < minimum_size) {
            continue;
        }
        setCanvasTextSize(canvas, true, size);
        if (canvas.textWidth(text) + 1 <= max_width) {
            return size;
        }
    }
    setCanvasTextSize(canvas, true, minimum_size);
    return minimum_size;
}

void includeFullDateDirtyRect(DirtyRect& dirty) {
    DirtyRect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = kDateStatusW;
    rect.h = kDateStatusH;
    rect.valid = true;
    includeDirtyRect(dirty, rect);
}

}  // namespace

void RenderFastDateStatus(M5EPD_Canvas& canvas, const String& date_text) {
    canvas.fillCanvas(kWhite);
    canvas.setTextDatum(CL_DATUM);
    canvas.setTextColor(kText);
    setCanvasTextSize(canvas, false, 3);
    canvas.drawString(date_text, 0, kDateStatusH / 2);
}

DateStatusRenderResult RenderDateStatus(M5EPD_Canvas& canvas,
                                        const DateStatusRenderInput& input,
                                        DateStatusRenderState& state) {
    DateStatusRenderResult result;
    if (input.date_text == state.last_date_text_rendered &&
        input.weekday == state.last_weekday_rendered &&
        input.render_signature == state.last_holiday_display_rendered) {
        return result;
    }

    const bool date_changed =
        input.date_text != state.last_date_text_rendered;
    const bool weekday_changed = input.weekday != state.last_weekday_rendered;
    const bool signature_changed =
        input.render_signature != state.last_holiday_display_rendered;
    const bool show_date = input.date_layout != "holiday-only";
    const bool show_weekday =
        input.date_layout != "date-only" &&
        input.date_layout != "holiday-only";

    canvas.fillCanvas(kWhite);
    canvas.setTextDatum(CL_DATUM);
    canvas.setTextColor(kText);

    if (input.use_cjk_font) {
        if (input.date_layout == "two-line") {
            const uint8_t two_line_text_size =
                std::min<uint8_t>(input.date_text_size, 2);
            const String first_line = joinDateTextParts(
                show_date ? input.date_text : String(""),
                show_weekday ? input.weekday_text : String(""), String(""));
            fitDateTextSize(canvas, first_line, two_line_text_size,
                            kMinimumDateTextSize, kDateStatusW);
            if (!first_line.isEmpty()) {
                drawFauxBoldString(canvas, first_line, 0, 12);
            }
            if (!input.holiday_text.isEmpty()) {
                fitDateTextSize(canvas, input.holiday_text, two_line_text_size,
                                kMinimumDateTextSize, kDateStatusW);
                drawFauxBoldString(canvas, input.holiday_text, 0, 34);
            }
        } else {
            const String line = joinDateTextParts(
                show_date ? input.date_text : String(""),
                show_weekday ? input.weekday_text : String(""),
                input.holiday_text);
            fitDateTextSize(canvas, line, input.date_text_size,
                            kMinimumDateTextSize, kDateStatusW);
            if (!line.isEmpty()) {
                drawFauxBoldString(canvas, line, 0, kDateStatusH / 2);
            }
            if (!input.pairing_code.isEmpty()) {
                drawFauxBoldString(canvas, "蓝牙配对码 " + input.pairing_code,
                                   520, kDateStatusH / 2);
            }
        }
    } else {
        setCanvasTextSize(canvas, false, 3);
        if (show_date && !input.date_text.isEmpty()) {
            canvas.drawString(input.date_text, 0, kDateStatusH / 2);
        }
    }

    if (input.full_refresh) {
        state.partial_count = 0;
    } else {
        result.update_mode =
            nextPartialMode(state.partial_count, input.partial_clean_interval);
        if (input.use_cjk_font) {
            if (signature_changed) {
                includeFullDateDirtyRect(result.dirty);
            } else if (date_changed) {
                DirtyRect rect;
                rect.x = kDateDateAreaX;
                rect.y = 0;
                rect.w = kDateDateAreaW;
                rect.h = kDateStatusH;
                rect.valid = true;
                includeDirtyRect(result.dirty, rect);
            }
            if (weekday_changed) {
                DirtyRect rect;
                rect.x = kDateWeekdayAreaX;
                rect.y = 0;
                rect.w = kDateWeekdayAreaW;
                rect.h = kDateStatusH;
                rect.valid = true;
                includeDirtyRect(result.dirty, rect);
            }
        } else {
            includeFullDateDirtyRect(result.dirty);
        }
        result.partial_update = result.dirty.valid;
    }

    state.last_date_text_rendered = input.date_text;
    state.last_holiday_display_rendered = input.render_signature;
    state.last_weekday_rendered = input.weekday;
    result.rendered = true;
    return result;
}

}  // namespace render

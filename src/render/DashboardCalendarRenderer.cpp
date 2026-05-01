#include "render/DashboardCalendarRenderer.h"

#include "DisplayFormatting.h"

namespace render {
namespace {

constexpr uint8_t kWhite = 0;
constexpr uint8_t kBorder = 11;
constexpr uint8_t kText = 15;
constexpr uint8_t kCalendarPastFill = 4;
constexpr uint8_t kCalendarTodayFill = 15;
constexpr int16_t kDashboardCalendarCellW = 42;
constexpr int16_t kDashboardCalendarCellH = 24;
constexpr int16_t kDashboardCalendarGridW = kDashboardCalendarCellW * 7;
constexpr int16_t kDashboardCalendarStartX =
    (kDashboardCalendarStatusW - kDashboardCalendarGridW) / 2;
constexpr int16_t kDashboardCalendarHeaderY = 54;
constexpr int16_t kDashboardCalendarGridY = 86;

DirtyRect dashboardCalendarCellRect(const rtc_date_t& date) {
    const uint8_t month_days = displayfmt::DaysInMonth(date.year, date.mon);
    if (date.day < 1 || date.day > month_days) {
        return {};
    }

    rtc_date_t first_day = date;
    first_day.day = 1;
    const uint8_t first_weekday = displayfmt::CalculateWeekday(first_day);
    const int index = first_weekday + date.day - 1;
    const int row = index / 7;
    const int col = index % 7;
    DirtyRect rect;
    rect.x = static_cast<int16_t>(kDashboardCalendarStartX +
                                  col * kDashboardCalendarCellW);
    rect.y = static_cast<int16_t>(kDashboardCalendarGridY +
                                  row * kDashboardCalendarCellH);
    rect.w = kDashboardCalendarCellW - 2;
    rect.h = kDashboardCalendarCellH - 2;
    rect.valid = true;
    return rect;
}

void drawWeekdayHeader(M5EPD_Canvas& canvas, bool use_cjk_font) {
    for (int col = 0; col < 7; ++col) {
        const int16_t center_x =
            kDashboardCalendarStartX + col * kDashboardCalendarCellW +
            (kDashboardCalendarCellW - 2) / 2;
        canvas.setTextDatum(CC_DATUM);
        canvas.setTextColor(kText);
        if (use_cjk_font) {
            setCanvasTextSize(canvas, true, 2);
            canvas.drawString(
                displayfmt::WeekdayLabel(static_cast<uint8_t>(col), "narrow"),
                center_x, kDashboardCalendarHeaderY + 8);
        } else {
            static constexpr const char* kFallbackWeekdayLabels[] = {
                "S", "M", "T", "W", "T", "F", "S"};
            setCanvasTextSize(canvas, false, 2);
            canvas.drawString(kFallbackWeekdayLabels[col], center_x,
                              kDashboardCalendarHeaderY + 8);
        }
    }
}

}  // namespace

DashboardCalendarRenderResult RenderDashboardCalendar(
    M5EPD_Canvas& canvas,
    const DashboardCalendarRenderInput& input,
    DashboardCalendarRenderState& state) {
    DashboardCalendarRenderResult result;
    canvas.fillCanvas(kWhite);
    canvas.setTextColor(kText);
    canvas.setTextDatum(CC_DATUM);
    setCanvasTextSize(canvas, input.use_cjk_font, 5);
    canvas.drawString(displayfmt::DashboardMonth(input.current_date),
                      kDashboardCalendarStatusW / 2, 24);

    drawWeekdayHeader(canvas,
                      input.use_weekday_cjk_font && !input.fast_ascii_only);

    rtc_date_t first_day = input.current_date;
    first_day.day = 1;
    const uint8_t first_weekday = displayfmt::CalculateWeekday(first_day);
    const uint8_t month_days =
        displayfmt::DaysInMonth(input.current_date.year,
                                input.current_date.mon);

    setCanvasTextSize(canvas, input.use_cjk_font, 2);
    canvas.setTextDatum(CC_DATUM);
    for (int row = 0; row < 6; ++row) {
        for (int col = 0; col < 7; ++col) {
            const int day = row * 7 + col - first_weekday + 1;
            if (day < 1 || day > month_days) {
                continue;
            }

            const int16_t x =
                kDashboardCalendarStartX + col * kDashboardCalendarCellW;
            const int16_t y =
                kDashboardCalendarGridY + row * kDashboardCalendarCellH;
            const bool is_today = day == input.current_date.day;
            const bool is_past = day < input.current_date.day;

            if (is_today) {
                canvas.fillRect(x, y, kDashboardCalendarCellW - 2,
                                kDashboardCalendarCellH - 2,
                                kCalendarTodayFill);
            } else if (is_past) {
                canvas.fillRect(x, y, kDashboardCalendarCellW - 2,
                                kDashboardCalendarCellH - 2,
                                kCalendarPastFill);
            }
            canvas.drawRect(x, y, kDashboardCalendarCellW - 2,
                            kDashboardCalendarCellH - 2, kBorder);
            canvas.setTextColor(is_today ? kWhite : kText);
            canvas.drawString(String(day),
                              x + (kDashboardCalendarCellW - 2) / 2,
                              y + (kDashboardCalendarCellH - 2) / 2);
        }
    }

    if (input.full_refresh) {
        state.partial_count = 0;
    } else {
        const bool can_refresh_changed_cells =
            input.previous_date.year == input.current_date.year &&
            input.previous_date.mon == input.current_date.mon &&
            input.previous_date.day >= 1 &&
            input.previous_date.day != input.current_date.day;
        result.update_mode =
            nextPartialMode(state.partial_count, input.partial_clean_interval);
        if (can_refresh_changed_cells) {
            includeDirtyRect(result.dirty,
                             dashboardCalendarCellRect(input.previous_date));
            includeDirtyRect(result.dirty,
                             dashboardCalendarCellRect(input.current_date));
        } else {
            result.dirty.x = 0;
            result.dirty.y = 0;
            result.dirty.w = kDashboardCalendarStatusW;
            result.dirty.h = kDashboardCalendarStatusH;
            result.dirty.valid = true;
        }
        result.partial_update = result.dirty.valid;
    }

    result.rendered = true;
    return result;
}

}  // namespace render

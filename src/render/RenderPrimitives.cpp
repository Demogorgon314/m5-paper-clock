#include "render/RenderPrimitives.h"

#include <algorithm>

#include "logic/UiLogic.h"

namespace render {
namespace {

constexpr int16_t alignDownTo4(int16_t value) {
    return value & ~0x03;
}

constexpr int16_t alignUpTo4(int16_t value) {
    return (value + 3) & ~0x03;
}

}  // namespace

uint16_t uiTextPixelSize(uint8_t legacy_size) {
    switch (legacy_size) {
        case 2:
            return 20;
        case 3:
            return 30;
        case 4:
            return 38;
        case 5:
            return 48;
        case 6:
            return 60;
        case 7:
            return 24;
        default:
            break;
    }
    return static_cast<uint16_t>(legacy_size) * 10;
}

void setCanvasTextSize(M5EPD_Canvas& canvas, bool use_cjk_font,
                       uint8_t legacy_size) {
    canvas.useFreetypeFont(use_cjk_font);
    canvas.setTextSize(use_cjk_font ? uiTextPixelSize(legacy_size) : legacy_size);
}

void includeDirtyRect(DirtyRect& dirty, const DirtyRect& rect) {
    if (!rect.valid) {
        return;
    }
    if (!dirty.valid) {
        dirty = rect;
        return;
    }

    const int16_t left = std::min<int16_t>(dirty.x, rect.x);
    const int16_t top = std::min<int16_t>(dirty.y, rect.y);
    const int16_t right =
        std::max<int16_t>(static_cast<int16_t>(dirty.x + dirty.w),
                          static_cast<int16_t>(rect.x + rect.w));
    const int16_t bottom =
        std::max<int16_t>(static_cast<int16_t>(dirty.y + dirty.h),
                          static_cast<int16_t>(rect.y + rect.h));
    dirty.x = left;
    dirty.y = top;
    dirty.w = right - left;
    dirty.h = bottom - top;
    dirty.valid = true;
}

PartialRegion makePartialRegion(int16_t x, int16_t y, int16_t w, int16_t h) {
    const int16_t update_x = alignDownTo4(x);
    const int16_t update_y = alignDownTo4(y);
    const int16_t update_right = alignUpTo4(x + w);
    const int16_t update_bottom = y + h;
    PartialRegion region;
    region.update_x = update_x;
    region.update_y = update_y;
    region.draw_x = x - update_x;
    region.draw_y = y - update_y;
    region.update_w = update_right - update_x;
    region.update_h = update_bottom - update_y;
    return region;
}

void updateAlignedArea(int16_t x, int16_t y, int16_t w, int16_t h,
                       m5epd_update_mode_t mode) {
    const PartialRegion region = makePartialRegion(x, y, w, h);
    M5.EPD.UpdateArea(region.update_x, region.update_y, region.update_w,
                      region.update_h, mode);
}

void pushAndRefreshCanvas(M5EPD_Canvas& canvas, int16_t x, int16_t y,
                          m5epd_update_mode_t mode) {
    canvas.pushCanvas(x, y, UPDATE_MODE_NONE);
    updateAlignedArea(x, y, canvas.width(), canvas.height(), mode);
}

m5epd_update_mode_t nextPartialMode(uint8_t& count, uint8_t clean_interval) {
    const uint8_t interval = static_cast<uint8_t>(
        logic::ClampPartialCleanInterval(clean_interval));
    if (++count >= interval) {
        count = 0;
        return UPDATE_MODE_GC16;
    }
    return UPDATE_MODE_GL16;
}

}  // namespace render

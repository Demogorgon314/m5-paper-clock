#include "render/ClassicInfoRenderer.h"

namespace render {
namespace {

constexpr uint8_t kWhite = 0;
constexpr uint8_t kText = 15;
constexpr uint8_t kMutedText = 8;
constexpr int16_t kSmallDigitWidth = 54;
constexpr int16_t kSmallDigitHeight = 96;
constexpr int16_t kSmallGap = 8;
constexpr int16_t kInfoDigitY = 6;
constexpr int16_t kHumidityUnitX = 226;
constexpr int16_t kHumidityUnitY = 72;
constexpr int16_t kTemperatureDegreeX = 480;
constexpr int16_t kTemperatureDegreeY = 62;
constexpr int16_t kTemperatureUnitX = 495;
constexpr int16_t kTemperatureUnitY = 72;
constexpr int16_t kHumidityDigitXs[] = {40, 102, 164};
constexpr int16_t kTemperatureDigitXs[] = {280, 342, 422};
constexpr int16_t kHumidityLocalX = 40;
constexpr int16_t kHumidityUnitLocalX = 226;
constexpr int16_t kTemperatureLocalX = 0;
constexpr int16_t kTemperatureDegreeLocalX = 200;
constexpr int16_t kTemperatureUnitLocalX = 215;
constexpr int16_t kComfortLocalX = 520;
constexpr int16_t kComfortFaceCenterX = 640;
constexpr int16_t kComfortFaceCenterY = 58;

void drawComfortInfoAt(M5EPD_Canvas& canvas, int16_t center_x,
                       int16_t center_y, const String& face, uint8_t color,
                       bool use_cjk_font) {
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
    setCanvasTextSize(canvas, use_cjk_font, 6);
    canvas.drawString("(", center_x - 78, center_y);
    canvas.drawString(")", center_x + 78, center_y);

    if (face == "(^_^)") {
        drawCaret(center_x - 30, center_y - 2, 10, 12, 4);
        drawCaret(center_x + 30, center_y - 2, 10, 12, 4);
        drawDash(center_x, center_y + 18, 12, 4);
        return;
    }

    if (face == "(-^-)") {
        drawDash(center_x - 34, center_y - 8, 10, 2);
        drawDash(center_x + 34, center_y - 8, 10, 2);
        drawCaret(center_x, center_y + 14, 9, 10, 2);
        return;
    }

    drawDash(center_x - 34, center_y - 8, 10, 2);
    drawDash(center_x + 34, center_y - 8, 10, 2);
    drawDash(center_x, center_y + 18, 12, 2);
}

void drawHumidityInfo(const SegmentRenderer& renderer, M5EPD_Canvas& canvas,
                      const String& humidity_text, int16_t draw_x,
                      bool use_cjk_font) {
    canvas.fillCanvas(kWhite);
    canvas.setTextDatum(TL_DATUM);
    renderer.drawText(canvas, draw_x, kInfoDigitY, humidity_text,
                      kSmallDigitWidth, kSmallDigitHeight, kSmallGap, kText,
                      kMutedText);
    canvas.setTextColor(kText);
    setCanvasTextSize(canvas, use_cjk_font, 3);
    canvas.drawString("%", draw_x + (kHumidityUnitLocalX - kHumidityLocalX),
                      kHumidityUnitY);
}

void drawTemperatureInfo(const SegmentRenderer& renderer, M5EPD_Canvas& canvas,
                         const String& temperature_text, int16_t draw_x,
                         bool use_cjk_font) {
    canvas.fillCanvas(kWhite);
    canvas.setTextDatum(TL_DATUM);
    const String temperature_display =
        temperature_text.substring(0, 2) + "." + temperature_text.substring(2);
    renderer.drawText(canvas, draw_x, kInfoDigitY, temperature_display,
                      kSmallDigitWidth, kSmallDigitHeight, kSmallGap, kText,
                      kMutedText);
    canvas.setTextColor(kText);
    setCanvasTextSize(canvas, use_cjk_font, 3);
    canvas.drawString(" C",
                      draw_x + (kTemperatureUnitLocalX - kTemperatureLocalX),
                      kTemperatureUnitY);
    setCanvasTextSize(canvas, use_cjk_font, 2);
    canvas.drawString(
        "o", draw_x + (kTemperatureDegreeLocalX - kTemperatureLocalX),
        kTemperatureDegreeY);
}

void drawComfortInfo(M5EPD_Canvas& canvas, const String& face,
                     int16_t center_x, bool use_cjk_font) {
    canvas.fillCanvas(kWhite);
    drawComfortInfoAt(canvas, center_x, kComfortFaceCenterY, face, kText,
                      use_cjk_font);
}

void addPartialUpdate(ClassicInfoRenderResult& result,
                      ClassicInfoRenderTarget target, int16_t push_x,
                      int16_t push_y, int16_t width, int16_t height,
                      m5epd_update_mode_t update_mode) {
    if (result.partial_update_count >= 3) {
        return;
    }
    ClassicInfoPartialUpdate& update =
        result.partial_updates[result.partial_update_count++];
    update.target = target;
    update.push_x = push_x;
    update.push_y = push_y;
    update.width = width;
    update.height = height;
    update.update_mode = update_mode;
}

}  // namespace

ClassicInfoRenderResult RenderClassicInfo(
    const SegmentRenderer& renderer,
    M5EPD_Canvas& info_canvas,
    M5EPD_Canvas& humidity_canvas,
    M5EPD_Canvas& temperature_canvas,
    M5EPD_Canvas& comfort_canvas,
    const ClassicInfoRenderInput& input,
    ClassicInfoRenderState& state) {
    ClassicInfoRenderResult result;
    const bool humidity_changed =
        input.humidity_text != state.last_humidity_text;
    const bool temperature_changed =
        input.temperature_text != state.last_temperature_text;
    const bool comfort_changed = input.comfort_face != state.last_comfort_face;

    if (!input.full_refresh && !humidity_changed && !temperature_changed &&
        !comfort_changed) {
        return result;
    }

    if (input.full_refresh) {
        info_canvas.fillCanvas(kWhite);
        info_canvas.setTextDatum(TL_DATUM);
        renderer.drawText(info_canvas, kHumidityDigitXs[0], kInfoDigitY,
                          input.humidity_text, kSmallDigitWidth,
                          kSmallDigitHeight, kSmallGap, kText, kMutedText);
        info_canvas.setTextColor(kText);
        setCanvasTextSize(info_canvas, input.use_cjk_font, 3);
        info_canvas.drawString("%", kHumidityUnitX, kHumidityUnitY);

        const String temperature_display =
            input.temperature_text.substring(0, 2) + "." +
            input.temperature_text.substring(2);
        renderer.drawText(info_canvas, kTemperatureDigitXs[0], kInfoDigitY,
                          temperature_display, kSmallDigitWidth,
                          kSmallDigitHeight, kSmallGap, kText, kMutedText);
        info_canvas.setTextColor(kText);
        info_canvas.drawString(" C", kTemperatureUnitX, kTemperatureUnitY);
        setCanvasTextSize(info_canvas, input.use_cjk_font, 2);
        info_canvas.drawString("o", kTemperatureDegreeX, kTemperatureDegreeY);

        drawComfortInfoAt(info_canvas, kComfortFaceCenterX, kComfortFaceCenterY,
                          input.comfort_face, kText, input.use_cjk_font);
        state.humidity_partial_count = 0;
        state.temperature_partial_count = 0;
        state.comfort_partial_count = 0;
        result.full_render = true;
    } else {
        if (humidity_changed) {
            const PartialRegion region =
                makePartialRegion(input.x + kHumidityDigitXs[0], input.y,
                                  kClassicHumidityCanvasW,
                                  kClassicHumidityCanvasH);
            drawHumidityInfo(renderer, humidity_canvas, input.humidity_text,
                             (input.x + kHumidityDigitXs[0]) - region.update_x,
                             input.use_cjk_font);
            addPartialUpdate(
                result, ClassicInfoRenderTarget::Humidity, region.update_x,
                region.update_y,
                humidity_canvas.width(), humidity_canvas.height(),
                nextPartialMode(state.humidity_partial_count,
                                input.partial_clean_interval));
        }

        if (temperature_changed) {
            const PartialRegion region =
                makePartialRegion(input.x + kTemperatureDigitXs[0], input.y,
                                  kClassicTemperatureCanvasW,
                                  kClassicTemperatureCanvasH);
            drawTemperatureInfo(renderer, temperature_canvas,
                                input.temperature_text,
                                (input.x + kTemperatureDigitXs[0]) -
                                    region.update_x,
                                input.use_cjk_font);
            addPartialUpdate(
                result, ClassicInfoRenderTarget::Temperature, region.update_x,
                region.update_y,
                temperature_canvas.width(), temperature_canvas.height(),
                nextPartialMode(state.temperature_partial_count,
                                input.partial_clean_interval));
        }

        if (comfort_changed) {
            const PartialRegion region =
                makePartialRegion(input.x + kComfortLocalX, input.y,
                                  kClassicComfortCanvasW,
                                  kClassicComfortCanvasH);
            drawComfortInfo(comfort_canvas, input.comfort_face,
                            (input.x + kComfortFaceCenterX) - region.update_x,
                            input.use_cjk_font);
            addPartialUpdate(
                result, ClassicInfoRenderTarget::Comfort, region.update_x,
                region.update_y,
                comfort_canvas.width(), comfort_canvas.height(),
                nextPartialMode(state.comfort_partial_count,
                                input.partial_clean_interval));
        }
    }

    state.last_humidity_text = input.humidity_text;
    state.last_temperature_text = input.temperature_text;
    state.last_comfort_face = input.comfort_face;
    result.rendered = true;
    return result;
}

}  // namespace render

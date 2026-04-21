from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


FONT_PATH = "/System/Library/Fonts/STHeiti Medium.ttc"
FONT_INDEX = 1
FONT_SIZE = 30
PADDING_X = 4
PADDING_Y = 2
TEXTS = ["周日", "周一", "周二", "周三", "周四", "周五", "周六"]
OUTPUT = Path("include/resources/weekday_bitmaps.h")


def pack_4bpp(image: Image.Image) -> list[int]:
    image = image.convert("L")
    width, height = image.size
    packed: list[int] = []
    for y in range(height):
        row = []
        for x in range(width):
            pixel = image.getpixel((x, y))
            row.append(round((255 - pixel) * 15 / 255))
        if width % 2:
            row.append(0)
        for x in range(0, len(row), 2):
            packed.append((row[x] << 4) | row[x + 1])
    return packed


def render_text(font: ImageFont.FreeTypeFont, text: str, height: int) -> tuple[int, int, list[int]]:
    left, top, right, bottom = font.getbbox(text)
    width = (right - left) + PADDING_X * 2
    image = Image.new("L", (width, height), 255)
    draw = ImageDraw.Draw(image)
    text_x = PADDING_X - left
    text_y = PADDING_Y - top
    draw.text((text_x, text_y), text, font=font, fill=0)
    return width, height, pack_4bpp(image)


def main() -> None:
    font = ImageFont.truetype(FONT_PATH, FONT_SIZE, index=FONT_INDEX)

    heights = []
    for text in TEXTS:
        _, top, _, bottom = font.getbbox(text)
        heights.append((bottom - top) + PADDING_Y * 2)
    height = max(heights)

    lines = [
        "#pragma once",
        "",
        "#include <stdint.h>",
        "",
        "struct WeekdayBitmap {",
        "    uint16_t width;",
        "    uint16_t height;",
        "    const uint8_t* data;",
        "};",
        "",
    ]

    entry_lines = []
    for index, text in enumerate(TEXTS):
        width, glyph_height, packed = render_text(font, text, height)
        lines.append(f"static const uint8_t kWeekdayBitmap{index}[] = {{")
        chunk = []
        for value in packed:
            chunk.append(f"0x{value:02x}")
            if len(chunk) == 12:
                lines.append("    " + ", ".join(chunk) + ",")
                chunk = []
        if chunk:
            lines.append("    " + ", ".join(chunk) + ",")
        lines.append("};")
        lines.append("")
        entry_lines.append(
            f"    {{{width}, {glyph_height}, kWeekdayBitmap{index}}},  // {text}"
        )

    lines.append("static const WeekdayBitmap kWeekdayBitmaps[] = {")
    lines.extend(entry_lines)
    lines.append("};")
    lines.append("")

    OUTPUT.write_text("\n".join(lines), encoding="utf-8")


if __name__ == "__main__":
    main()

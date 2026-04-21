import { execFileSync } from "node:child_process";
import { mkdtempSync, readFileSync, rmSync, writeFileSync } from "node:fs";
import { tmpdir } from "node:os";
import path from "node:path";
import { fileURLToPath } from "node:url";

const scriptDir = path.dirname(fileURLToPath(import.meta.url));
const outPath = path.resolve(scriptDir, "../include/resources/holiday_countdown_bitmaps.h");
const workdir = mkdtempSync(path.join(tmpdir(), "holiday-bitmaps-"));
const fontPath = process.env.HOLIDAY_BITMAP_FONT ||
  "/System/Library/Fonts/STHeiti Medium.ttc";
const pointSize = process.env.HOLIDAY_BITMAP_POINT_SIZE || "30";

const countdownLabels = [
  { key: "YuanDan", text: "距 元旦 还有" },
  { key: "ChunJie", text: "距 春节 还有" },
  { key: "QingMing", text: "距 清明节 还有" },
  { key: "LaoDong", text: "距 劳动节 还有" },
  { key: "DuanWu", text: "距 端午节 还有" },
  { key: "ZhongQiu", text: "距 中秋节 还有" },
  { key: "GuoQing", text: "距 国庆节 还有" },
];

const activeLabels = [
  { key: "YuanDan", text: "元旦假期中" },
  { key: "ChunJie", text: "春节假期中" },
  { key: "QingMing", text: "清明节假期中" },
  { key: "LaoDong", text: "劳动节假期中" },
  { key: "DuanWu", text: "端午节假期中" },
  { key: "ZhongQiu", text: "中秋节假期中" },
  { key: "GuoQing", text: "国庆节假期中" },
];

const lastDayLabels = [
  { key: "YuanDan", text: "元旦最后一天" },
  { key: "ChunJie", text: "春节最后一天" },
  { key: "QingMing", text: "清明节最后一天" },
  { key: "LaoDong", text: "劳动节最后一天" },
  { key: "DuanWu", text: "端午节最后一天" },
  { key: "ZhongQiu", text: "中秋节最后一天" },
  { key: "GuoQing", text: "国庆节最后一天" },
];

const singleLabels = [
  { key: "DaySuffix", text: "天" },
];

function renderLabel(label) {
  const pngPath = path.join(workdir, `${label.key}.png`);
  const rawPath = path.join(workdir, `${label.key}.gray`);
  execFileSync("magick", [
    "-background", "white",
    "-fill", "black",
    "-font", fontPath,
    "-pointsize", pointSize,
    `label:${label.text}`,
    "-trim",
    "+repage",
    pngPath,
  ]);

  const geometry = execFileSync("magick", [
    "identify",
    "-format",
    "%w %h",
    pngPath,
  ], { encoding: "utf8" }).trim();
  const [width, height] = geometry.split(" ").map((value) => Number.parseInt(value, 10));

  execFileSync("magick", [
    pngPath,
    "-alpha", "off",
    "-colorspace", "Gray",
    "-depth", "8",
    `gray:${rawPath}`,
  ]);

  const raw = readFileSync(rawPath);
  const rowBytes = Math.ceil(width / 2);
  const packed = Buffer.alloc(rowBytes * height);
  for (let y = 0; y < height; y += 1) {
    for (let x = 0; x < width; x += 1) {
      const gray = raw[y * width + x];
      const pixel = Math.round((255 - gray) * 15 / 255);
      const offset = y * rowBytes + Math.floor(x / 2);
      if (x % 2 === 0) {
        packed[offset] |= pixel << 4;
      } else {
        packed[offset] |= pixel;
      }
    }
  }

  return { ...label, width, height, packed };
}

function bytesToLines(buffer) {
  const parts = [];
  for (const value of buffer.values()) {
    parts.push(`0x${value.toString(16).padStart(2, "0")}`);
  }

  const lines = [];
  for (let index = 0; index < parts.length; index += 12) {
    lines.push(`    ${parts.slice(index, index + 12).join(", ")},`);
  }
  return lines.join("\n");
}

function renderLabelArray(labels) {
  return labels.map(renderLabel);
}

function buildBitmapArrays(prefix, rendered) {
  return rendered.map((label) => `static const uint8_t ${prefix}${label.key}[] = {\n${bytesToLines(label.packed)}\n};`).join("\n\n");
}

function buildHolidayBitmapFunction(functionName, prefix, rendered) {
  return `inline const HolidayBannerBitmap& ${functionName}(logic::HolidayId id) {
    static const HolidayBannerBitmap kYuanDan = {${rendered[0].width}, ${rendered[0].height}, ${prefix}YuanDan};
    static const HolidayBannerBitmap kChunJie = {${rendered[1].width}, ${rendered[1].height}, ${prefix}ChunJie};
    static const HolidayBannerBitmap kQingMing = {${rendered[2].width}, ${rendered[2].height}, ${prefix}QingMing};
    static const HolidayBannerBitmap kLaoDong = {${rendered[3].width}, ${rendered[3].height}, ${prefix}LaoDong};
    static const HolidayBannerBitmap kDuanWu = {${rendered[4].width}, ${rendered[4].height}, ${prefix}DuanWu};
    static const HolidayBannerBitmap kZhongQiu = {${rendered[5].width}, ${rendered[5].height}, ${prefix}ZhongQiu};
    static const HolidayBannerBitmap kGuoQing = {${rendered[6].width}, ${rendered[6].height}, ${prefix}GuoQing};
    switch (id) {
        case logic::HolidayId::YuanDan:
            return kYuanDan;
        case logic::HolidayId::ChunJie:
            return kChunJie;
        case logic::HolidayId::QingMing:
            return kQingMing;
        case logic::HolidayId::LaoDong:
            return kLaoDong;
        case logic::HolidayId::DuanWu:
            return kDuanWu;
        case logic::HolidayId::ZhongQiu:
            return kZhongQiu;
        case logic::HolidayId::GuoQing:
        case logic::HolidayId::None:
        default:
            return kGuoQing;
    }
}`;
}

function buildSingleBitmapFunction(functionName, prefix, rendered) {
  return `inline const HolidayBannerBitmap& ${functionName}() {
    static const HolidayBannerBitmap kBitmap = {${rendered[0].width}, ${rendered[0].height}, ${prefix}${rendered[0].key}};
    return kBitmap;
}`;
}

const renderedCountdown = renderLabelArray(countdownLabels);
const renderedActive = renderLabelArray(activeLabels);
const renderedLastDay = renderLabelArray(lastDayLabels);
const renderedSingle = renderLabelArray(singleLabels);

const body = `#pragma once

#include <stdint.h>

#include "logic/HolidayLogic.h"

struct HolidayBannerBitmap {
    uint16_t width;
    uint16_t height;
    const uint8_t* data;
};

${buildBitmapArrays("kHolidayBannerBitmapCountdown", renderedCountdown)}

${buildBitmapArrays("kHolidayBannerBitmapActive", renderedActive)}

${buildBitmapArrays("kHolidayBannerBitmapLastDay", renderedLastDay)}

${buildBitmapArrays("kHolidayBannerBitmap", renderedSingle)}

${buildHolidayBitmapFunction("holidayCountdownLabelBitmap", "kHolidayBannerBitmapCountdown", renderedCountdown)}

${buildHolidayBitmapFunction("holidayActiveLabelBitmap", "kHolidayBannerBitmapActive", renderedActive)}

${buildHolidayBitmapFunction("holidayLastDayLabelBitmap", "kHolidayBannerBitmapLastDay", renderedLastDay)}

${buildSingleBitmapFunction("holidayCountdownDaySuffixBitmap", "kHolidayBannerBitmap", renderedSingle)}
`;

writeFileSync(outPath, body);
rmSync(workdir, { recursive: true, force: true });
console.log(`Wrote holiday countdown bitmaps to ${outPath}`);

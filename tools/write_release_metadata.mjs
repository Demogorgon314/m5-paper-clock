import { createHash } from "node:crypto";
import { readFileSync, statSync, writeFileSync } from "node:fs";
import { basename, join } from "node:path";

function requiredEnv(name) {
  const value = process.env[name];
  if (!value) {
    throw new Error(`Missing ${name}`);
  }
  return value;
}

function fileInfo(path, baseUrl) {
  const name = basename(path);
  const bytes = statSync(path).size;
  const hash = createHash("sha256");
  hash.update(readFileSync(path));
  return {
    name,
    path: name,
    url: baseUrl ? `${baseUrl.replace(/\/$/, "")}/${name}` : name,
    size: bytes,
    sha256: hash.digest("hex"),
  };
}

const distDir = requiredEnv("RELEASE_DIST_DIR");
const version = requiredEnv("RELEASE_VERSION");
const buildSha = process.env.RELEASE_BUILD_SHA || "unknown";
const buildTime = process.env.RELEASE_BUILD_TIME || new Date().toISOString();
const baseUrl = process.env.RELEASE_BASE_URL || "";

const firmware = fileInfo(join(distDir, "firmware.bin"), baseUrl);
const fullImage = fileInfo(join(distDir, "m5-paper-clock-complete.bin"), baseUrl);
const spiffs = fileInfo(join(distDir, "spiffs.bin"), baseUrl);

const ota = {
  name: "M5Paper Ink Clock",
  version,
  buildSha,
  buildTime,
  generatedAt: new Date().toISOString(),
  chipFamily: "ESP32",
  ota: {
    ...firmware,
    offset: "0x10000",
  },
  full: {
    ...fullImage,
    offset: "0x0",
  },
  filesystem: {
    ...spiffs,
    offset: "0x810000",
  },
};

const webFlashManifest = {
  name: "M5Paper Ink Clock",
  version,
  new_install_prompt_erase: true,
  builds: [
    {
      chipFamily: "ESP32",
      parts: [
        {
          path: fullImage.url,
          offset: 0,
        },
      ],
    },
  ],
};

writeFileSync(join(distDir, "ota.json"), `${JSON.stringify(ota, null, 2)}\n`);
writeFileSync(
  join(distDir, "web-flash-manifest.json"),
  `${JSON.stringify(webFlashManifest, null, 2)}\n`,
);

import { execFileSync } from "node:child_process";
import path from "node:path";
import { fileURLToPath } from "node:url";

const scriptDir = path.dirname(fileURLToPath(import.meta.url));
const yearArgs = process.argv.slice(2);

const updateDataScript = path.resolve(scriptDir, "update_holiday_data.mjs");
const updateBitmapScript = path.resolve(scriptDir, "generate_holiday_bitmaps.mjs");

const dataCommand = [updateDataScript, ...yearArgs];
console.log(`Updating holiday data with: node ${path.basename(updateDataScript)} ${yearArgs.join(" ")}`.trim());
execFileSync("node", dataCommand, {
  cwd: scriptDir,
  stdio: "inherit",
  env: process.env,
});

console.log(`Updating holiday bitmap resources with: node ${path.basename(updateBitmapScript)}`);
execFileSync("node", [updateBitmapScript], {
  cwd: scriptDir,
  stdio: "inherit",
  env: process.env,
});

console.log("Holiday assets are up to date.");

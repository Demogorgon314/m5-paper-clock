# Repository Guidelines

## Project Structure & Module Organization

This is a PlatformIO firmware project for an M5Paper-style e-ink clock. Main Arduino/C++ sources live in `src/`: `ClockApp.*` owns app state and rendering flow, `ConnectivityService.*` handles Wi-Fi/NTP, `SettingsStore.*` persists settings, and service/rendering modules stay split by responsibility. Pure logic helpers live in `include/logic/` and are covered by native tests. Web Serial/Bluetooth configuration assets are in `web/`. Device data assets, including `data/cjk.ttf`, are packaged into SPIFFS. Build, release, and flashing helpers live in `tools/`.

## Build, Test, and Development Commands

- `platformio run -e m5stack-fire`: build firmware for the device target.
- `platformio test -e native`: run Unity-based native logic tests.
- `./tools/build_merged_image.sh`: build a complete flash image with bootloader, partitions, firmware, and SPIFFS.
- `./tools/start_web_config.sh`: serve the local configuration UI at `http://localhost:4173`.
- `./tools/flash_firmware.sh /dev/cu.usbserial-...`: upload only the app firmware.
- `./tools/flash_all.sh /dev/cu.usbserial-...`: full flash for first install, recovery, or SPIFFS changes.
- `./tools/package_release.sh`: produce release artifacts under `dist/`.

## Coding Style & Naming Conventions

Use C++17 as configured in `platformio.ini`. Follow the existing style: 4-space indentation, braces on function/control blocks, `PascalCase` for classes and public data types, `camelCase` for methods, and `snake_case` for simple struct fields in `include/logic`. Keep hardware-dependent code in `src/` services and keep deterministic business logic in `include/logic` so it remains testable under the native environment. Prefer small functions and direct, readable control flow over clever abstractions.

## Testing Guidelines

Native tests use Unity and live in `test/test_logic/test_main.cpp`. Add tests for every new public helper in `include/logic`, including boundary values and malformed inputs. Name tests as `test_<behavior>()` and register them in the test runner. Run `platformio test -e native` before submitting changes. For firmware-facing changes that cannot run natively, also build with `platformio run -e m5stack-fire`.

## Commit & Pull Request Guidelines

Recent commits use concise imperative subjects, for example `Fix online OTA TLS memory pressure` and `Stabilize local OTA serial upload`. Keep subjects focused on one behavior change. Pull requests should describe the user-visible change, list verification commands, call out hardware-tested paths, and include screenshots for `web/` UI updates when relevant. Link related issues and note any required flashing mode or migration concern.

## Release Process

Releases are produced by the GitHub Actions workflow in `.github/workflows/release.yml`. Do not manually upload local `dist/` artifacts for normal releases. Before releasing, make sure `main` is clean and the intended release commit has passed the relevant local checks, normally `platformio test -e native` plus `platformio run -e m5stack-fire` for firmware changes.

Use patch/minor semantic tags matching the existing `v0.1.x` style. Push the release commit to `main`, then create and push an annotated tag:

```sh
git push origin main
git tag -a v0.1.10 -m "Release v0.1.10"
git push origin v0.1.10
```

Pushing the tag triggers the release workflow. The workflow builds the release package with `./tools/package_release.sh`, creates or updates the GitHub Release, and uploads `firmware.bin`, `m5-paper-clock-complete.bin`, `spiffs.bin`, `partitions.bin`, `ota.json`, and `web-flash-manifest.json`. After pushing the tag, verify the Actions run succeeds and confirm the Release assets are present. The OTA UI reads the latest GitHub Release metadata, so a successful tagged release is enough to make the new OTA package available.

## Security & Configuration Tips

Do not commit Wi-Fi credentials, local serial ports, generated `dist/` artifacts, or device-specific secrets. Treat OTA metadata and release URLs as configuration; override them with documented environment variables only when packaging a release.

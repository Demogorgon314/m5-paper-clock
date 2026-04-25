# M5Paper Ink Clock

[English README](README.md)

一个横屏墨水屏时钟固件，面向 M5Paper 类硬件，基于 PlatformIO 和
M5EPD Arduino 库构建。

![M5Paper Ink Clock 封面图](docs/images/hero.webp)

## 功能

- 首次启动 Wi-Fi 配置流程
- 通过 NTP 同步 RTC，支持配置时区
- 开机自动连接 Wi-Fi，同步成功后自动进入时钟界面
- 经典数码时钟和可编辑的仪表盘布局预设
- Web 端一比一布局预览，可拖动时间、日历、行情、温湿度等组件
- 支持 A 股股票和指数行情卡片，可按代码、中文名称、拼音搜索
- 读取板载 SHT30 温湿度
- 可配置日期格式、舒适度阈值、墨水屏清晰度校准频率
- SPIFFS 内置 CJK TTF 字体，用于 UI 文本和中文行情名称
- G38 侧边按键：
  - 左/右键：切换已保存布局预设
  - 中键短按：全量刷新
  - 中键长按：进入设置

## 项目结构

- `src/ClockApp.*` - 应用状态、页面、输入处理和渲染流程
- `src/SegmentRenderer.*` - 数码管字体绘制
- `src/ConnectivityService.*` - Wi-Fi 和 NTP 同步
- `src/SettingsStore.*` - 配置持久化
- `src/SensorService.*` - SHT30 温湿度读取
- `web/*` - 静态 Chrome Web Serial/Bluetooth 配置页面
- `include/logic/*` - 可测试的 UI、布局、行情、日期等逻辑
- `test/test_logic/test_main.cpp` - native 逻辑测试

## 构建

项目使用仓库内置的节假日数据，日常构建不需要联网预处理。

```bash
platformio run -e m5stack-fire
```

`data/cjk.ttf` 字体会放在 SPIFFS 中。可以构建包含 bootloader、分区表、
固件和 SPIFFS 的完整刷机镜像：

```bash
./tools/build_merged_image.sh
```

输出文件：

```text
.pio/build/m5stack-fire/m5-paper-clock-complete.bin
```

## Web 配置

刷入固件后，可以在 Chrome 中通过 USB 或 Bluetooth 配置设备，无需进入设备
端设置页。

`web/` 目录是纯静态资源，可以直接部署到 Netlify 等 HTTPS 静态站点。
Web Serial 和 Web Bluetooth 需要 HTTPS；本地调试可以启动本地服务：

在线 Web 配置页：

```text
https://m5-paper-clock.netlify.app/
```

```bash
./tools/start_web_config.sh
```

然后打开：

```text
http://localhost:4173
```

请使用 Chrome 或 Edge。USB 方式下，连接设备后点击一次 `连接 USB` 授权串
口；页面之后会自动重连已授权的 USB 设备。蓝牙方式下，点击 `连接蓝牙` 并
选择 `M5Paper Clock`。

Web 页面可以：

- 读取设备状态
- 扫描附近 Wi-Fi
- 保存 SSID 和密码
- 保存时区和墨水屏清晰度校准频率
- 新建、复制、删除、保存布局预设
- 添加/删除仪表盘组件，并在一比一预览里拖动位置
- 配置行情标的、日期显示、温湿度舒适度阈值等组件属性
- 连接 Wi-Fi 并同步时间
- 触发全量刷新或重启设备
- 根据 GitHub Release metadata 执行在线 OTA
- 通过 USB 串口上传本地 OTA `firmware.bin`

USB 和 Bluetooth 共用同一套 JSON 命令协议。只有以 `@cfg:` 开头的行会被
当作协议消息处理，因此设备普通日志仍然可以继续打印到 USB 串口。

行情搜索在浏览器中通过 Eastmoney JSONP 完成，不需要后端服务。支持股票/
指数代码、中文名称、拼音搜索，例如 `600519`、`茅台`、`上证`、`GZMT`。

本地 OTA 上传仅支持 USB，只更新 app 固件分区，因此 Wi-Fi、时区、行情标
的、布局等已保存配置会保留。设备必须已经运行支持本地 OTA 协议的固件；
如果从更老版本升级，请先执行一次完整刷机或固件刷写。

Web UI 最多保存 5 个仪表盘布局预设。经典数字布局是内置布局，可以通过设
备按键或 Web UI 选择。

![Web 配置页面](docs/images/web-config-setup.webp)

![带一比一墨水屏预览的布局编辑器](docs/images/layout-editor.webp)

## 更新节假日数据

需要更新内置节假日表时手动运行：

```bash
node tools/update_holiday_data.mjs 2026 2027
```

## 测试

```bash
platformio test -e native
```

## Release 包

构建 GitHub Release、Web 完整刷写和 OTA metadata 需要的产物：

```bash
./tools/package_release.sh
```

产物会写入 `dist/` 下的版本目录，包含：

- `firmware.bin` - OTA app 固件更新
- `m5-paper-clock-complete.bin` - 浏览器/串口完整刷机镜像
- `spiffs.bin` - 文件系统恢复
- `ota.json` - OTA 更新检查
- `web-flash-manifest.json` - ESP Web Tools 风格完整刷写 manifest

可以通过 `FIRMWARE_VERSION`、`FIRMWARE_GIT_SHA`、`FIRMWARE_BUILD_TIME`
或 `RELEASE_BASE_URL` 覆盖生成的 metadata。推送 `v1.2.3` 这类 tag 会触
发 release workflow，并上传这些文件到 GitHub Release。

## 刷机

把下面的串口替换成 `platformio device list` 看到的端口。

支持两种刷机模式：

1. 完整镜像

首次安装、恢复出厂、救砖或 SPIFFS 内容变化时使用。

```bash
./tools/flash_all.sh /dev/cu.usbserial-02120D19
```

该命令会构建 `m5-paper-clock-complete.bin`，并从 `0x0` 一次性写入。

2. 仅固件

日常开发优先使用，和 OTA 包更新范围一致。

```bash
./tools/flash_firmware.sh /dev/cu.usbserial-02120D19
```

该命令只更新 app 镜像，保留现有 SPIFFS/font 分区。

如果省略端口，两个脚本都会尝试自动检测常见 USB 串口设备。

手动完整镜像刷写：

```bash
python3 -m esptool \
  --chip esp32 --port /dev/cu.usbserial-02120D19 --baud 1500000 \
  write_flash --flash_mode dio --flash_freq 40m --flash_size 16MB \
  0x0 .pio/build/m5stack-fire/m5-paper-clock-complete.bin
```

手动仅固件刷写：

```bash
platformio run -e m5stack-fire -t upload \
  --upload-port /dev/cu.usbserial-02120D19
```

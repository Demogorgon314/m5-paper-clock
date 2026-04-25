import {
  COMPONENT_REGISTRY,
  DEFAULT_CLASSIC_LAYOUT,
  DEFAULT_DASHBOARD_LAYOUT,
} from "./components/registry.js";

const CFG_PREFIX = "@cfg:";
const DEFAULT_BAUD_RATE = 1500000;
const FALLBACK_BAUD_RATE = 115200;
const SERIAL_BAUD_RATES = Object.freeze([DEFAULT_BAUD_RATE, FALLBACK_BAUD_RATE]);
const SERIAL_HANDSHAKE_TIMEOUT_MS = 6000;
const LOCAL_OTA_CHUNK_SIZE = 3072;
const LOCAL_OTA_BINARY_CHUNK_SIZE = 512;
const LOCAL_OTA_BINARY_CHUNK_DELAY_MS = 2;
const LOCAL_OTA_CHUNK_RETRIES = 4;
const LOCAL_OTA_CHUNK_DELAY_MS = 6;
const LOCAL_OTA_RETRY_DELAY_MS = 250;
const BLE_DEVICE_NAME_PREFIX = "M5Paper";
const BLE_SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const BLE_RX_CHARACTERISTIC_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
const BLE_TX_CHARACTERISTIC_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";
const BLE_WRITE_CHUNK_SIZE = 20;
const BLE_AUTH_STORAGE_KEY = "m5-paper-clock.bleAuthToken";
const LANGUAGE_STORAGE_KEY = "m5-paper-clock.language";
const LAYOUT_PRESETS_STORAGE_KEY = "m5-paper-clock.layoutPresets";
const REQUEST_TIMEOUT_MS = 15000;
const POLL_INTERVAL_MS = 20000;
const MARKET_SEARCH_API_PATH = "/api/market-search";
const DEFAULT_OTA_MANIFEST_URL =
  "https://github.com/Demogorgon314/m5-paper-clock/releases/latest/download/ota.json";
const DEFAULT_WEB_FLASH_MANIFEST_URL =
  "https://github.com/Demogorgon314/m5-paper-clock/releases/latest/download/web-flash-manifest.json";
const GITHUB_RELEASE_DOWNLOAD_RE =
  /^https:\/\/github\.com\/([^/]+)\/([^/]+)\/releases\/(?:latest\/download|download\/([^/]+))\/[^?#]+/;
const DEFAULT_COMFORT_SETTINGS = Object.freeze({
  comfortTemperatureMin: 19,
  comfortTemperatureMax: 27,
  comfortHumidityMin: 20,
  comfortHumidityMax: 85,
});
const DEFAULT_REFRESH_SETTINGS = Object.freeze({
  partialCleanInterval: 4,
});
const COMFORT_LIMITS = Object.freeze({
  temperatureMin: -20,
  temperatureMax: 60,
  humidityMin: 0,
  humidityMax: 100,
});
const REFRESH_LIMITS = Object.freeze({
  partialCleanIntervalMin: 1,
  partialCleanIntervalMax: 30,
});
const DASHBOARD_PREVIEW = Object.freeze({
  width: 960,
  height: 540,
  snap: 4,
  guideThreshold: 8,
});
const DEFAULT_CLASSIC_LAYOUT_ID = "classic-default";
const DEFAULT_CLASSIC_LAYOUT_NAME = "经典数字";
const DEFAULT_LAYOUT_ID = "dashboard-default";
const DEFAULT_LAYOUT_NAME = "仪表盘";
const BUILT_IN_LAYOUT_IDS = Object.freeze([
  DEFAULT_CLASSIC_LAYOUT_ID,
  DEFAULT_LAYOUT_ID,
]);
const DASHBOARD_PREVIEW_SAMPLE = Object.freeze({
  date: "2026-04-24",
  weekdayZh: "周五",
  weekdayEn: "Fri",
  holidayZh: "距 劳动节 还有 7 天",
  holidayEn: "7 days to Labor Day",
  batteryPercent: 86,
  wifiConnected: true,
  wifiSignalLevel: 3,
  time: "22:12",
  marketNameZh: "上证指数",
  marketNameEn: "SSE Composite",
  marketValue: "4079.90",
  marketStatusZh: "休市 16:14",
  marketStatusEn: "Closed 16:14",
  marketChange: "▼ -13.35  -0.33%",
  humidity: "62",
  temperature: "29.3",
  comfortFace: "(-^-)",
  configConnectionIcon: "none",
});
const DASHBOARD_PREVIEW_FONT = Object.freeze({
  cjkFamily: "M5PaperCJK",
  asciiFamily: "ui-monospace, Menlo, Consolas, monospace",
  fallbackCjkFamily: "system-ui, -apple-system, BlinkMacSystemFont, sans-serif",
});

const DEFAULT_LOCALE = "zh-CN";
const SUPPORTED_LOCALES = Object.freeze(["zh-CN", "en"]);

const I18N = Object.freeze({
  "zh-CN": {
    "app.title": "M5Paper Ink Clock 配置",
    "language.label": "语言",
    "intro.heading": "设备配置",
    "intro.lead": "通过 USB 或蓝牙连接设备，保存 Wi-Fi、时区和布局。",
    "intro.step.connect": "连接 USB 或蓝牙",
    "intro.step.scan": "扫描并选择 Wi-Fi",
    "intro.step.save": "保存并连接 Wi-Fi",
    "intro.step.sync": "同步时间并回到时钟页",
    "intro.deviceAlt": "M5Paper 设备",
    "browser.note": "需要 Chrome 或 Edge，并从 <code>localhost</code> 或 HTTPS 打开。",
    "browser.secureContextRequired": "请通过 localhost 或 HTTPS 打开这个页面。",
    "browser.noDeviceSupport": "当前浏览器不支持 Web Serial 或 Web Bluetooth，请使用 Chrome 或 Edge。",
    "browser.webSerialUnsupported": "当前浏览器不支持 WebSerial，请使用桌面版 Chrome 或 Edge。",
    "browser.notAllowed": "请通过 localhost 或 HTTPS 打开页面。",
    "status.heading": "设备状态",
    "status.disconnected": "未连接",
    "status.connected": "已连接 {transport}",
    "status.firmware": "固件",
    "status.market": "行情",
    "status.battery": "电量",
    "status.timeSync": "时间同步",
    "status.details": "详细状态",
    "status.device": "设备",
    "status.page": "页面",
    "status.timezone": "时区",
    "status.layout": "布局",
    "status.synced": "已同步",
    "status.unsynced": "未同步",
    "status.usb": "USB",
    "status.bluetooth": "蓝牙",
    "message.waitingDevice": "连接后会在这里显示设备返回的信息。",
    "wifi.helper": "先扫描，再选网络，最后保存到设备或立即连接 Wi-Fi。",
    "wifi.ssid": "网络名称",
    "wifi.ssidPlaceholder": "选择或输入 SSID",
    "wifi.password": "密码",
    "wifi.passwordPlaceholder": "输入 Wi-Fi 密码",
    "wifi.savedNetwork": "已保存网络",
    "wifi.emptyConnected": "还没有扫描结果。",
    "wifi.emptyDisconnected": "连接设备后可以开始扫描。",
    "settings.heading": "设置",
    "settings.helper": "保存后会写入设备存储，重启后继续生效。",
    "settings.timezone": "时区",
    "settings.partialCleanInterval": "墨水屏清晰度校准频率",
    "settings.saveSection": "配置保存",
    "settings.saveHelp": "仅保存不会联网；保存并连接会写入配置、连接 Wi-Fi，并同步时间。",
    "settings.deviceSection": "设备操作",
    "settings.dangerSection": "危险操作",
    "comfort.heading": "表情条件",
    "comfort.helper": "舒适区间显示 <code>(^_^)</code>，超出范围显示 <code>(-^-)</code>。",
    "comfort.tempMin": "最低舒适温度",
    "comfort.tempMax": "最高舒适温度",
    "comfort.humidityMin": "最低舒适湿度",
    "comfort.humidityMax": "最高舒适湿度",
    "comfort.save": "保存属性",
    "comfort.restoreDefaults": "恢复默认设置",
    "layout.heading": "布局预设",
    "layout.helper": "选择布局预设；仪表盘类布局可拖动组件调整位置，保存后设备会全量刷新一次。",
    "layout.preset": "布局预设",
    "layout.preview": "一比一预览",
    "layout.position": "位置",
    "layout.properties": "属性",
    "layout.components": "组件",
    "layout.addComponent": "添加组件",
    "layout.removeComponent": "删除组件",
    "layout.keyboardHint": "选中画布组件后，可用方向键微调位置，Shift + 方向键大步移动。",
    "layout.contextReset": "重置位置",
    "layout.contextRemove": "删除组件",
    "layout.contextConfirmReset": "确认重置这个组件的位置？",
    "layout.contextConfirmRemove": "确认删除这个组件？",
    "layout.contextConfirm": "确认",
    "layout.contextCancel": "取消",
    "layout.hiddenComponents": "可添加组件",
    "layout.visibleComponents": "已显示组件",
    "layout.noVisibleComponents": "没有显示组件，请先添加组件。",
    "layout.noHiddenComponents": "所有组件都已添加。",
    "layout.blankName": "空白布局",
    "layout.copySuffix": "副本",
    "layout.status": "{name} · {visible}/{total} 个组件{dirty}",
    "layout.unsaved": " · 未保存",
    "layout.component.date": "日期",
    "layout.component.battery": "状态",
    "layout.component.calendar": "日历",
    "layout.component.time": "时间",
    "layout.component.summary": "行情",
    "layout.component.climate": "温湿度",
    "layout.component.classicTime": "经典大时间",
    "layout.component.classicInfo": "经典温湿度",
    "layout.prop.symbol": "行情代码",
    "layout.prop.symbolPlaceholder": "例如 sh000001",
    "layout.noProperties": "这个组件暂时没有可配置属性。",
    "layout.nudge": "微调",
    "layout.resetComponent": "重置此组件",
    "hint.serial": "Chrome 会记住你授权过的串口，页面打开后会自动恢复连接。",
    "hint.bluetooth": "蓝牙连接需要先在设备附近点“连接蓝牙”并选择 M5Paper Clock。",
    "update.heading": "固件更新",
    "update.helper": "日常升级优先使用 OTA；字体、分区变化或首次安装时再使用完整刷写。",
    "update.otaHeading": "OTA 更新",
    "update.otaHelper": "保留配置，只替换应用固件。",
    "update.recommended": "推荐",
    "update.otaManifest": "OTA 信息地址",
    "update.otaSummaryDefault": "读取 release metadata 后可以通过 Wi-Fi 执行 OTA 更新。",
    "update.localOtaFirmware": "本地 OTA 固件",
    "update.localOtaSummaryDefault": "仅支持 USB 串口上传应用固件 bin，保留设备配置。",
    "update.fullFlashHeading": "完整刷写",
    "update.fullFlashHelper": "用于首次安装、字体或分区变化。",
    "update.advanced": "高级",
    "update.fullFlashWarning": "可能清除设备配置，包括 Wi-Fi、时区、布局、行情标的和表情条件。",
    "update.fullFlashManifest": "完整刷写 Manifest 地址",
    "update.noOtaInfo": "没有可用的 OTA 固件信息。",
    "update.latestSummary": "当前 {current}，可用 {available}，已是最新版本，固件 {size} MB。",
    "update.availableSummary": "当前 {current}，可用 {available}，固件 {size} MB。",
    "update.otaProgressPreparing": "准备更新",
    "update.otaProgressDownloading": "正在下载 {percent}% ({written} / {total})",
    "update.otaProgressVerifying": "正在校验固件",
    "update.otaProgressInstalled": "OTA 已安装",
    "update.noFile": "未选择文件",
    "update.localUploading": "{file}，正在上传 {percent}% ({written} / {total})。",
    "update.localReady": "{file}，可通过 USB 上传并更新。",
    "update.localNeedUsb": "{file}，请先连接 USB 串口后再上传。",
    "market.heading": "行情标的",
    "market.helper": "支持大盘指数和 A 股股票，按代码搜索后直接设为首页行情卡片。",
    "market.current": "当前标的",
    "market.searchCode": "代码搜索",
    "market.searchPlaceholder": "输入 600519 / 000001 / 399001 / sh000300",
    "market.searchHelper": "热门标的在下方；输入代码后右侧显示搜索结果。",
    "market.hot": "热门标的",
    "market.hotGroup": "热门{kind}",
    "market.results": "搜索结果",
    "market.resultCount": "{count} 个结果",
    "market.pending": "待搜索",
    "market.emptyBeforeSearch": "输入代码后在这里显示搜索结果。",
    "market.emptyConnected": "没有找到匹配的标的。",
    "market.emptyDisconnected": "连接设备后即可搜索并切换行情标的。",
    "market.currentTag": "当前",
    "market.index": "指数",
    "market.stock": "股票",
    "market.defaultName": "上证指数",
    "market.signalUnknown": "信号未知",
    "market.signalStrong": "很强",
    "market.signalGood": "良好",
    "market.signalFair": "一般",
    "market.signalWeak": "较弱",
    "log.heading": "通信日志",
    "log.error": "错误",
    "log.device": "设备",
    "log.info": "信息",
    "button.connectUsb": "连接 USB",
    "button.connectBluetooth": "连接蓝牙",
    "button.reconnectUsb": "恢复 USB",
    "button.disconnect": "断开",
    "button.readStatus": "读取状态",
    "button.scan": "扫描",
    "button.showPassword": "显示",
    "button.hidePassword": "隐藏",
    "button.saveOnly": "仅保存配置",
    "button.saveConnect": "保存并连接 Wi-Fi",
    "button.syncTime": "同步时间",
    "button.fullRefresh": "全量刷新",
    "button.reboot": "重启设备",
    "button.checkUpdate": "检查更新",
    "button.otaUpdate": "OTA 更新",
    "button.chooseBin": "选择 bin 文件",
    "button.usbUploadUpdate": "USB 上传并更新",
    "button.fullFlash": "完整刷写",
    "button.search": "搜索",
    "button.clear": "清空",
    "button.newLayout": "新建空白",
    "button.duplicateLayout": "复制预设",
    "button.deleteLayout": "删除预设",
    "button.saveLayout": "保存布局",
    "button.resetLayout": "恢复默认布局",
  },
  en: {
    "app.title": "M5Paper Ink Clock Setup",
    "language.label": "Language",
    "intro.heading": "Device Setup",
    "intro.lead": "Connect over USB or Bluetooth to save Wi-Fi, timezone, and layout settings.",
    "intro.step.connect": "Connect USB or Bluetooth",
    "intro.step.scan": "Scan and choose Wi-Fi",
    "intro.step.save": "Save and connect Wi-Fi",
    "intro.step.sync": "Sync time and return to the clock",
    "intro.deviceAlt": "M5Paper device",
    "browser.note": "Requires Chrome or Edge, opened from <code>localhost</code> or HTTPS.",
    "browser.secureContextRequired": "Open this page from localhost or HTTPS.",
    "browser.noDeviceSupport": "This browser does not support Web Serial or Web Bluetooth. Use Chrome or Edge.",
    "browser.webSerialUnsupported": "This browser does not support WebSerial. Use desktop Chrome or Edge.",
    "browser.notAllowed": "Open this page from localhost or HTTPS.",
    "status.heading": "Device Status",
    "status.disconnected": "Disconnected",
    "status.connected": "Connected {transport}",
    "status.firmware": "Firmware",
    "status.market": "Market",
    "status.battery": "Battery",
    "status.timeSync": "Time Sync",
    "status.details": "Details",
    "status.device": "Device",
    "status.page": "Page",
    "status.timezone": "Timezone",
    "status.layout": "Layout",
    "status.synced": "Synced",
    "status.unsynced": "Not synced",
    "status.usb": "USB",
    "status.bluetooth": "Bluetooth",
    "message.waitingDevice": "Device responses will appear here after connecting.",
    "wifi.helper": "Scan first, choose a network, then save it to the device or connect immediately.",
    "wifi.ssid": "Network name",
    "wifi.ssidPlaceholder": "Choose or enter an SSID",
    "wifi.password": "Password",
    "wifi.passwordPlaceholder": "Enter Wi-Fi password",
    "wifi.savedNetwork": "Saved network",
    "wifi.emptyConnected": "No scan results yet.",
    "wifi.emptyDisconnected": "Connect the device to start scanning.",
    "settings.heading": "Settings",
    "settings.helper": "Settings are saved on the device and persist after reboot.",
    "settings.timezone": "Timezone",
    "settings.partialCleanInterval": "Screen clarity refresh frequency",
    "settings.saveSection": "Save Settings",
    "settings.saveHelp": "Save only does not connect; save and connect stores settings, connects Wi-Fi, and syncs time.",
    "settings.deviceSection": "Device Actions",
    "settings.dangerSection": "Danger Zone",
    "comfort.heading": "Face Conditions",
    "comfort.helper": "The comfortable range shows <code>(^_^)</code>; out-of-range values show <code>(-^-)</code>.",
    "comfort.tempMin": "Minimum comfort temperature",
    "comfort.tempMax": "Maximum comfort temperature",
    "comfort.humidityMin": "Minimum comfort humidity",
    "comfort.humidityMax": "Maximum comfort humidity",
    "comfort.save": "Save Properties",
    "comfort.restoreDefaults": "Restore Defaults",
    "layout.heading": "Layout Presets",
    "layout.helper": "Choose a layout preset. Dashboard layouts can be edited by dragging components; saving refreshes the device once.",
    "layout.preset": "Layout preset",
    "layout.preview": "1:1 Preview",
    "layout.position": "Position",
    "layout.properties": "Properties",
    "layout.components": "Components",
    "layout.addComponent": "Add component",
    "layout.removeComponent": "Remove component",
    "layout.keyboardHint": "Select a canvas component, then use arrow keys to nudge. Hold Shift for larger steps.",
    "layout.contextReset": "Reset position",
    "layout.contextRemove": "Remove component",
    "layout.contextConfirmReset": "Reset this component position?",
    "layout.contextConfirmRemove": "Remove this component?",
    "layout.contextConfirm": "Confirm",
    "layout.contextCancel": "Cancel",
    "layout.hiddenComponents": "Available components",
    "layout.visibleComponents": "Visible components",
    "layout.noVisibleComponents": "No visible components. Add one first.",
    "layout.noHiddenComponents": "All components are already added.",
    "layout.blankName": "Blank layout",
    "layout.copySuffix": "Copy",
    "layout.status": "{name} · {visible}/{total} components{dirty}",
    "layout.unsaved": " · Unsaved",
    "layout.component.date": "Date",
    "layout.component.battery": "Status",
    "layout.component.calendar": "Calendar",
    "layout.component.time": "Time",
    "layout.component.summary": "Market",
    "layout.component.climate": "Climate",
    "layout.component.classicTime": "Classic Time",
    "layout.component.classicInfo": "Classic Climate",
    "layout.prop.symbol": "Market symbol",
    "layout.prop.symbolPlaceholder": "For example sh000001",
    "layout.noProperties": "This component has no configurable properties yet.",
    "layout.nudge": "Nudge",
    "layout.resetComponent": "Reset component",
    "hint.serial": "Chrome remembers authorized serial ports and can reconnect when the page opens.",
    "hint.bluetooth": "For Bluetooth, stay near the device, click Connect Bluetooth, and choose M5Paper Clock.",
    "update.heading": "Firmware Update",
    "update.helper": "Use OTA for routine upgrades; use full flashing for first install, font changes, or partition changes.",
    "update.otaHeading": "OTA Update",
    "update.otaHelper": "Keeps settings and replaces only the app firmware.",
    "update.recommended": "Recommended",
    "update.otaManifest": "OTA manifest URL",
    "update.otaSummaryDefault": "Read release metadata to run an OTA update over Wi-Fi.",
    "update.localOtaFirmware": "Local OTA firmware",
    "update.localOtaSummaryDefault": "Uploads an app firmware .bin over USB serial and keeps device settings.",
    "update.fullFlashHeading": "Full Flash",
    "update.fullFlashHelper": "For first install, font changes, or partition changes.",
    "update.advanced": "Advanced",
    "update.fullFlashWarning": "May erase device settings, including Wi-Fi, timezone, layout, market symbol, and face conditions.",
    "update.fullFlashManifest": "Full flash manifest URL",
    "update.noOtaInfo": "No OTA firmware information is available.",
    "update.latestSummary": "Current {current}, available {available}, already up to date, firmware {size} MB.",
    "update.availableSummary": "Current {current}, available {available}, firmware {size} MB.",
    "update.otaProgressPreparing": "Preparing update",
    "update.otaProgressDownloading": "Downloading {percent}% ({written} / {total})",
    "update.otaProgressVerifying": "Verifying firmware",
    "update.otaProgressInstalled": "OTA installed",
    "update.noFile": "No file selected",
    "update.localUploading": "{file}, uploading {percent}% ({written} / {total}).",
    "update.localReady": "{file}, ready to upload and update over USB.",
    "update.localNeedUsb": "{file}, connect USB serial before uploading.",
    "market.heading": "Market Symbol",
    "market.helper": "Supports major indexes and China A-shares. Search by code and set the home market card directly.",
    "market.current": "Current symbol",
    "market.searchCode": "Code search",
    "market.searchPlaceholder": "Enter 600519 / 000001 / 399001 / sh000300",
    "market.searchHelper": "Popular symbols are below; search results appear on the right.",
    "market.hot": "Popular Symbols",
    "market.hotGroup": "Popular {kind}",
    "market.results": "Search Results",
    "market.resultCount": "{count} results",
    "market.pending": "Waiting",
    "market.emptyBeforeSearch": "Search results will appear here after you enter a code.",
    "market.emptyConnected": "No matching symbols found.",
    "market.emptyDisconnected": "Connect the device to search and switch the market symbol.",
    "market.currentTag": "Current",
    "market.index": "Index",
    "market.stock": "Stock",
    "market.defaultName": "SSE Composite",
    "market.signalUnknown": "Signal unknown",
    "market.signalStrong": "very strong",
    "market.signalGood": "good",
    "market.signalFair": "fair",
    "market.signalWeak": "weak",
    "log.heading": "Communication Log",
    "log.error": "Error",
    "log.device": "Device",
    "log.info": "Info",
    "button.connectUsb": "Connect USB",
    "button.connectBluetooth": "Connect Bluetooth",
    "button.reconnectUsb": "Reconnect USB",
    "button.disconnect": "Disconnect",
    "button.readStatus": "Read Status",
    "button.scan": "Scan",
    "button.showPassword": "Show",
    "button.hidePassword": "Hide",
    "button.saveOnly": "Save Only",
    "button.saveConnect": "Save and Connect Wi-Fi",
    "button.syncTime": "Sync Time",
    "button.fullRefresh": "Full Refresh",
    "button.reboot": "Reboot Device",
    "button.checkUpdate": "Check Update",
    "button.otaUpdate": "OTA Update",
    "button.chooseBin": "Choose bin file",
    "button.usbUploadUpdate": "USB Upload and Update",
    "button.fullFlash": "Full Flash",
    "button.search": "Search",
    "button.clear": "Clear",
    "button.newLayout": "Blank",
    "button.duplicateLayout": "Duplicate",
    "button.deleteLayout": "Delete",
    "button.saveLayout": "Save Layout",
    "button.resetLayout": "Reset Layout",
  },
});

const DEFAULT_MARKETS = [
  {
    requestSymbol: "sh000001",
    code: "000001",
    displayName: "上证指数",
    displayNameEn: "SSE Composite",
    kind: "index",
  },
  {
    requestSymbol: "sz399001",
    code: "399001",
    displayName: "深证成指",
    displayNameEn: "SZSE Component",
    kind: "index",
  },
  {
    requestSymbol: "sz399006",
    code: "399006",
    displayName: "创业板指",
    displayNameEn: "ChiNext Index",
    kind: "index",
  },
  {
    requestSymbol: "sh000300",
    code: "000300",
    displayName: "沪深300",
    displayNameEn: "CSI 300",
    kind: "index",
  },
  {
    requestSymbol: "sh000905",
    code: "000905",
    displayName: "中证500",
    displayNameEn: "CSI 500",
    kind: "index",
  },
  {
    requestSymbol: "sh000688",
    code: "000688",
    displayName: "科创50",
    displayNameEn: "STAR 50",
    kind: "index",
  },
  {
    requestSymbol: "sh600519",
    code: "600519",
    displayName: "贵州茅台",
    displayNameEn: "Kweichow Moutai",
    kind: "stock",
  },
  {
    requestSymbol: "sh601318",
    code: "601318",
    displayName: "中国平安",
    displayNameEn: "Ping An",
    kind: "stock",
  },
  {
    requestSymbol: "sz000858",
    code: "000858",
    displayName: "五粮液",
    displayNameEn: "Wuliangye",
    kind: "stock",
  },
  {
    requestSymbol: "sz300750",
    code: "300750",
    displayName: "宁德时代",
    displayNameEn: "CATL",
    kind: "stock",
  },
];

const elements = {
  languageSelect: document.querySelector("#language-select"),
  connectButton: document.querySelector("#connect-button"),
  bluetoothButton: document.querySelector("#bluetooth-button"),
  reconnectButton: document.querySelector("#reconnect-button"),
  disconnectButton: document.querySelector("#disconnect-button"),
  statusButton: document.querySelector("#status-button"),
  scanButton: document.querySelector("#scan-button"),
  saveButton: document.querySelector("#save-button"),
  applyButton: document.querySelector("#apply-button"),
  syncButton: document.querySelector("#sync-button"),
  refreshButton: document.querySelector("#refresh-button"),
  rebootButton: document.querySelector("#reboot-button"),
  checkUpdateButton: document.querySelector("#check-update-button"),
  otaUpdateButton: document.querySelector("#ota-update-button"),
  localOtaFileInput: document.querySelector("#local-ota-file-input"),
  localOtaFileName: document.querySelector("#local-ota-file-name"),
  localOtaUploadButton: document.querySelector("#local-ota-upload-button"),
  togglePasswordButton: document.querySelector("#toggle-password-button"),
  clearLogButton: document.querySelector("#clear-log-button"),
  connectionState: document.querySelector("#connection-state"),
  connectionStateLabel: document.querySelector("#connection-state-label"),
  browserNote: document.querySelector("#browser-note"),
  deviceName: document.querySelector("#device-name"),
  firmwareVersion: document.querySelector("#firmware-version"),
  pageName: document.querySelector("#page-name"),
  transportIcon: document.querySelector("#transport-icon"),
  wifiName: document.querySelector("#wifi-name"),
  ipAddress: document.querySelector("#ip-address"),
  timezoneLabel: document.querySelector("#timezone-label"),
  layoutLabel: document.querySelector("#layout-label"),
  marketLabel: document.querySelector("#market-label"),
  batteryLabel: document.querySelector("#battery-label"),
  syncState: document.querySelector("#sync-state"),
  rtcLabel: document.querySelector("#rtc-label"),
  deviceMessage: document.querySelector("#device-message"),
  savedSsid: document.querySelector("#saved-ssid"),
  ssidInput: document.querySelector("#ssid-input"),
  passwordInput: document.querySelector("#password-input"),
  timezoneSelect: document.querySelector("#timezone-select"),
  partialCleanIntervalInput: document.querySelector(
    "#partial-clean-interval-input",
  ),
  layoutPreview: document.querySelector("#layout-preview"),
  layoutComponentList: document.querySelector("#layout-component-list"),
  layoutEditorStatus: document.querySelector("#layout-editor-status"),
  layoutPresetSelect: document.querySelector("#layout-preset-select"),
  layoutNewButton: document.querySelector("#layout-new-button"),
  layoutDuplicateButton: document.querySelector("#layout-duplicate-button"),
  layoutDeleteButton: document.querySelector("#layout-delete-button"),
  layoutSaveButton: document.querySelector("#layout-save-button"),
  layoutResetButton: document.querySelector("#layout-reset-button"),
  otaManifestInput: document.querySelector("#ota-manifest-input"),
  webFlashManifestInput: document.querySelector("#web-flash-manifest-input"),
  webFlashButton: document.querySelector("#web-flash-button"),
  otaSummary: document.querySelector("#ota-summary"),
  otaProgressBox: document.querySelector("#ota-progress-box"),
  otaProgressLabel: document.querySelector("#ota-progress-label"),
  otaProgressPercent: document.querySelector("#ota-progress-percent"),
  otaProgressBar: document.querySelector("#ota-progress-bar"),
  localOtaSummary: document.querySelector("#local-ota-summary"),
  networkList: document.querySelector("#network-list"),
  logOutput: document.querySelector("#log-output"),
};

const state = {
  locale: detectInitialLocale(),
  connectionType: null,
  serialBaudRate: null,
  port: null,
  reader: null,
  writer: null,
  bluetoothDevice: null,
  bluetoothServer: null,
  bluetoothRxCharacteristic: null,
  bluetoothTxCharacteristic: null,
  decoder: new TextDecoder(),
  encoder: new TextEncoder(),
  readBuffer: "",
  requestId: 1,
  pendingRequests: new Map(),
  selectedSsid: "",
  networks: [],
  connected: false,
  pollTimer: null,
  busyCount: 0,
  supportsSerial: false,
  supportsBluetooth: false,
  bleAuthToken: window.localStorage.getItem(BLE_AUTH_STORAGE_KEY) || "",
  lastStatus: null,
  otaManifest: null,
  otaManifestUrl: "",
  otaUpdateAvailable: false,
  otaProgress: null,
  localOtaFile: null,
  localOtaLoggedWritten: 0,
  marketSearchQuery: "",
  marketResultsVisible: false,
  marketResults: [],
  activeLayoutId: DEFAULT_LAYOUT_ID,
  dashboardLayouts: [],
  dashboardLayout: cloneDashboardLayout(DEFAULT_DASHBOARD_LAYOUT),
  selectedLayoutId: DEFAULT_DASHBOARD_LAYOUT[0]?.id || null,
  draggedLayoutId: null,
  layoutDragOffset: { x: 0, y: 0 },
  layoutGuides: { vertical: [], horizontal: [] },
  layoutDirty: false,
  previewFontReady: false,
  previewWifiBitmaps: null,
};

function detectInitialLocale() {
  const savedLocale = window.localStorage.getItem(LANGUAGE_STORAGE_KEY);
  if (SUPPORTED_LOCALES.includes(savedLocale)) {
    return savedLocale;
  }

  const browserLocale = navigator.language || "";
  return browserLocale.toLowerCase().startsWith("zh") ? "zh-CN" : "en";
}

function t(key, params = {}) {
  const messages = I18N[state.locale] || I18N[DEFAULT_LOCALE];
  const template = messages[key] ?? I18N[DEFAULT_LOCALE][key] ?? key;
  return Object.entries(params).reduce(
    (text, [name, value]) => text.replaceAll(`{${name}}`, String(value)),
    template,
  );
}

function localized(zhCn, en) {
  return state.locale === "en" ? en : zhCn;
}

async function loadDashboardPreviewFont() {
  if (!("FontFace" in window) || !document.fonts) {
    return;
  }
  try {
    const font = new FontFace(DASHBOARD_PREVIEW_FONT.cjkFamily, "url('./assets/cjk.ttf')");
    const loadedFont = await font.load();
    document.fonts.add(loadedFont);
    state.previewFontReady = true;
    renderDashboardLayoutEditor();
  } catch {
    state.previewFontReady = false;
  }
}

function parsePreviewBitmapArray(headerText, arrayName) {
  const match = headerText.match(
    new RegExp(`static\\s+const\\s+uint8_t\\s+${arrayName}\\[\\]\\s*=\\s*\\{([\\s\\S]*?)\\n\\};`),
  );
  if (!match) {
    return null;
  }
  const bytes = Array.from(match[1].matchAll(/0x([0-9a-fA-F]{1,2})/g), (item) =>
    Number.parseInt(item[1], 16),
  );
  return bytes.length > 0 ? bytes : null;
}

function parseWifiBitmapsHeader(headerText) {
  const result = {};
  for (const level of [1, 2, 3]) {
    const data = parsePreviewBitmapArray(headerText, `kWifiBitmap${level}_32x32`);
    if (data) {
      result[level] = { width: 32, height: 32, data };
    }
  }
  return Object.keys(result).length > 0 ? result : null;
}

async function loadDashboardWifiBitmaps() {
  try {
    const response = await fetch("./assets/wifi_bitmaps.h", { cache: "no-store" });
    if (!response.ok) {
      return;
    }
    state.previewWifiBitmaps = parseWifiBitmapsHeader(await response.text());
    renderDashboardLayoutEditor();
  } catch {
    state.previewWifiBitmaps = null;
  }
}

function previewWifiBitmapForLevel(level) {
  const normalized = Math.max(1, Math.min(3, Number(level) || 1));
  return state.previewWifiBitmaps?.[normalized] || null;
}

function cloneDashboardLayout(layout) {
  return layout.map((item) => ({
    ...item,
    props: { ...(item.props || {}) },
  }));
}

function createLayoutPreset(id, name, components, kind = "dashboard") {
  return {
    id,
    name,
    kind,
    canvas: {
      width: DASHBOARD_PREVIEW.width,
      height: DASHBOARD_PREVIEW.height,
    },
    components:
      kind === "dashboard"
        ? dashboardLayoutWithLabels(components)
        : cloneDashboardLayout(components),
  };
}

function defaultDashboardLayoutPreset() {
  return createLayoutPreset(
    DEFAULT_LAYOUT_ID,
    DEFAULT_LAYOUT_NAME,
    DEFAULT_DASHBOARD_LAYOUT,
    "dashboard",
  );
}

function defaultClassicLayoutPreset() {
  return createLayoutPreset(
    DEFAULT_CLASSIC_LAYOUT_ID,
    DEFAULT_CLASSIC_LAYOUT_NAME,
    DEFAULT_CLASSIC_LAYOUT,
    "classic",
  );
}

function blankDashboardLayoutPreset() {
  return createLayoutPreset(
    uniqueLayoutId("dashboard-blank"),
    t("layout.blankName"),
    DEFAULT_DASHBOARD_LAYOUT.map((item) => ({ ...item, visible: false })),
    "dashboard",
  );
}

function uniqueLayoutId(prefix = "dashboard-custom") {
  const ids = new Set(state.dashboardLayouts.map((layout) => layout.id));
  let index = Date.now().toString(36);
  let candidate = `${prefix}-${index}`;
  while (ids.has(candidate)) {
    index = `${index}-1`;
    candidate = `${prefix}-${index}`;
  }
  return candidate;
}

function normalizeDashboardLayoutPreset(layout, fallback = defaultDashboardLayoutPreset()) {
  const kind = layout?.kind === "classic" || layout?.id === DEFAULT_CLASSIC_LAYOUT_ID
    ? "classic"
    : "dashboard";
  return createLayoutPreset(
    String(layout?.id || fallback.id || DEFAULT_LAYOUT_ID),
    String(layout?.name || fallback.name || DEFAULT_LAYOUT_NAME),
    Array.isArray(layout?.components) ? layout.components : fallback.components,
    kind,
  );
}

function ensureBaseLayoutPresets(layouts) {
  const normalizedLayouts = Array.isArray(layouts) ? [...layouts] : [];
  const hasClassic = normalizedLayouts.some(
    (layout) => layout.id === DEFAULT_CLASSIC_LAYOUT_ID,
  );
  const hasDashboard = normalizedLayouts.some(
    (layout) => layout.id === DEFAULT_LAYOUT_ID,
  );
  if (!hasClassic) {
    normalizedLayouts.unshift(defaultClassicLayoutPreset());
  }
  if (!hasDashboard) {
    normalizedLayouts.push(defaultDashboardLayoutPreset());
  }
  return normalizedLayouts;
}

function activeLayoutPreset() {
  return (
    state.dashboardLayouts.find((layout) => layout.id === state.activeLayoutId) ||
    state.dashboardLayouts[0] ||
    defaultDashboardLayoutPreset()
  );
}

function activeLayoutKind() {
  return activeLayoutPreset().kind === "classic" ? "classic" : "dashboard";
}

function isBuiltInLayoutId(id) {
  return BUILT_IN_LAYOUT_IDS.includes(id);
}

function layoutDocumentFromState() {
  const activeLayout = state.dashboardLayouts.find(
    (layout) => layout.id === state.activeLayoutId,
  );
  const layouts = ensureBaseLayoutPresets(
    state.dashboardLayouts.length
      ? state.dashboardLayouts
      : [defaultClassicLayoutPreset(), defaultDashboardLayoutPreset()],
  );
  return {
    activeLayoutId: activeLayout?.id || DEFAULT_LAYOUT_ID,
    layouts: layouts.map((layout) => ({
      id: layout.id,
      name: layout.name,
      kind: layout.kind || "dashboard",
      canvas: {
        width: DASHBOARD_PREVIEW.width,
        height: DASHBOARD_PREVIEW.height,
      },
      components: layout.components.map((item) => ({
        id: item.id,
        type: item.type,
        x: item.x,
        y: item.y,
        w: item.w,
        h: item.h,
        visible: item.visible !== false,
        props: { ...(item.props || {}) },
      })),
    })),
  };
}

function saveLayoutPresets() {
  try {
    window.localStorage.setItem(
      LAYOUT_PRESETS_STORAGE_KEY,
      JSON.stringify(layoutDocumentFromState()),
    );
  } catch {
    // Ignore quota or private-mode storage failures; device save still works.
  }
}

function loadLayoutPresets() {
  try {
    const raw = window.localStorage.getItem(LAYOUT_PRESETS_STORAGE_KEY);
    if (!raw) {
      return;
    }
    const document = JSON.parse(raw);
    if (!document || !Array.isArray(document.layouts)) {
      return;
    }
    state.dashboardLayouts = ensureBaseLayoutPresets(
      document.layouts.map((layout) => normalizeDashboardLayoutPreset(layout)),
    );
    state.activeLayoutId = String(document.activeLayoutId || state.dashboardLayouts[0]?.id || DEFAULT_LAYOUT_ID);
    const activeLayout =
      state.dashboardLayouts.find((layout) => layout.id === state.activeLayoutId) ||
      state.dashboardLayouts[0];
    if (activeLayout) {
      state.dashboardLayout = cloneDashboardLayout(activeLayout.components);
      state.activeLayoutId = activeLayout.id;
    }
  } catch {
    state.dashboardLayouts = [defaultClassicLayoutPreset(), defaultDashboardLayoutPreset()];
    state.activeLayoutId = DEFAULT_LAYOUT_ID;
    state.dashboardLayout = cloneDashboardLayout(DEFAULT_DASHBOARD_LAYOUT);
  }
}

function dashboardLayoutWithLabels(layout) {
  return Object.values(COMPONENT_REGISTRY).map((definition) => {
    const defaultItem = definition.defaultItem;
    const savedItem =
      layout.find(
        (item) => item.id === defaultItem.id || item.type === defaultItem.type,
      ) || {};
    const hasSavedVisibility = Object.prototype.hasOwnProperty.call(
      savedItem,
      "visible",
    );
    return {
      ...defaultItem,
      ...savedItem,
      w: defaultItem.w,
      h: defaultItem.h,
      labelKey: definition.labelKey,
      type: defaultItem.type,
      props: {
        ...defaultItem.props,
        ...(savedItem.props || {}),
      },
      visible: hasSavedVisibility
        ? savedItem.visible !== false
        : defaultItem.visible !== false,
    };
  });
}

function activeLayoutFromDocument(document) {
  if (!document || !Array.isArray(document.layouts)) {
    return null;
  }
  const activeLayoutId = String(document.activeLayoutId || "");
  return (
    document.layouts.find((layout) => layout.id === activeLayoutId) ||
    document.layouts[0] ||
    null
  );
}

function layoutDocumentFromComponents(components) {
  return {
    activeLayoutId: DEFAULT_LAYOUT_ID,
    layouts: [
      {
        id: DEFAULT_LAYOUT_ID,
        name: DEFAULT_LAYOUT_NAME,
        kind: "dashboard",
        canvas: {
          width: DASHBOARD_PREVIEW.width,
          height: DASHBOARD_PREVIEW.height,
        },
        components: components.map((item) => ({
          id: item.id,
          type: item.type,
          x: item.x,
          y: item.y,
          w: item.w,
          h: item.h,
          visible: item.visible !== false,
          props: { ...(item.props || {}) },
        })),
      },
    ],
  };
}

function clampDashboardLayoutItem(item) {
  const x = Math.min(
    Math.max(0, Number(item.x) || 0),
    DASHBOARD_PREVIEW.width - item.w,
  );
  const y = Math.min(
    Math.max(0, Number(item.y) || 0),
    DASHBOARD_PREVIEW.height - item.h,
  );
  return {
    ...item,
    x: Math.round(x / DASHBOARD_PREVIEW.snap) * DASHBOARD_PREVIEW.snap,
    y: Math.round(y / DASHBOARD_PREVIEW.snap) * DASHBOARD_PREVIEW.snap,
  };
}

function snapToDashboardGrid(value) {
  return Math.round(value / DASHBOARD_PREVIEW.snap) * DASHBOARD_PREVIEW.snap;
}

function layoutSnapPoints(item) {
  return {
    vertical: [
      { kind: "left", value: item.x },
      { kind: "center", value: item.x + item.w / 2 },
      { kind: "right", value: item.x + item.w },
    ],
    horizontal: [
      { kind: "top", value: item.y },
      { kind: "center", value: item.y + item.h / 2 },
      { kind: "bottom", value: item.y + item.h },
    ],
  };
}

function dashboardGuideCandidates(draggedId) {
  const vertical = [
    0,
    DASHBOARD_PREVIEW.width / 2,
    DASHBOARD_PREVIEW.width,
  ];
  const horizontal = [
    0,
    DASHBOARD_PREVIEW.height / 2,
    DASHBOARD_PREVIEW.height,
  ];

  for (const item of state.dashboardLayout.filter((layoutItem) => layoutItem.visible !== false)) {
    if (item.id === draggedId) {
      continue;
    }
    const points = layoutSnapPoints(item);
    vertical.push(...points.vertical.map((point) => point.value));
    horizontal.push(...points.horizontal.map((point) => point.value));
  }

  return { vertical, horizontal };
}

function nearestDashboardGuide(points, candidates) {
  let best = null;
  for (const point of points) {
    for (const candidate of candidates) {
      const distance = Math.abs(point.value - candidate);
      if (
        distance > DASHBOARD_PREVIEW.guideThreshold ||
        (best && distance >= best.distance)
      ) {
        continue;
      }
      best = {
        kind: point.kind,
        value: candidate,
        distance,
      };
    }
  }
  return best;
}

function applyDashboardGuideSnap(item) {
  const candidates = dashboardGuideCandidates(item.id);
  const points = layoutSnapPoints(item);
  const verticalGuide = nearestDashboardGuide(points.vertical, candidates.vertical);
  const horizontalGuide = nearestDashboardGuide(
    points.horizontal,
    candidates.horizontal,
  );
  const snapped = { ...item };

  if (verticalGuide) {
    if (verticalGuide.kind === "left") {
      snapped.x = verticalGuide.value;
    } else if (verticalGuide.kind === "center") {
      snapped.x = verticalGuide.value - item.w / 2;
    } else {
      snapped.x = verticalGuide.value - item.w;
    }
  }
  if (horizontalGuide) {
    if (horizontalGuide.kind === "top") {
      snapped.y = horizontalGuide.value;
    } else if (horizontalGuide.kind === "center") {
      snapped.y = horizontalGuide.value - item.h / 2;
    } else {
      snapped.y = horizontalGuide.value - item.h;
    }
  }

  return {
    item: {
      ...snapped,
      x: snapToDashboardGrid(snapped.x),
      y: snapToDashboardGrid(snapped.y),
    },
    guides: {
      vertical: verticalGuide ? [verticalGuide.value] : [],
      horizontal: horizontalGuide ? [horizontalGuide.value] : [],
    },
  };
}

function createDashboardPreviewCanvas() {
  const canvas = document.createElement("canvas");
  canvas.className = "layout-preview-canvas";
  canvas.dataset.logicalWidth = String(DASHBOARD_PREVIEW.width);
  canvas.dataset.logicalHeight = String(DASHBOARD_PREVIEW.height);
  canvas.width = DASHBOARD_PREVIEW.width;
  canvas.height = DASHBOARD_PREVIEW.height;
  canvas.style.width = "100%";
  canvas.style.height = "100%";
  const ctx = canvas.getContext("2d");
  return { canvas, ctx };
}

const INK = Object.freeze({
  white: "#ffffff",
  paper: "#ffffff",
  text: "#181512",
  edgeText: "#2f2a24",
  border: "#746e67",
  mutedText: "#6e6860",
  calendarPastFill: "#bcb5aa",
  calendarTodayFill: "#181512",
  frame: "#c8c0b5",
});
const TEXT_DATUM = Object.freeze({
  TL: "TL",
  TR: "TR",
  CL: "CL",
  CR: "CR",
  CC: "CC",
});
const SEGMENT_MASKS = Object.freeze({
  "0": 0b1110111,
  "1": 0b0100100,
  "2": 0b1011101,
  "3": 0b1101101,
  "4": 0b0101110,
  "5": 0b1101011,
  "6": 0b1111011,
  "7": 0b0100101,
  "8": 0b1111111,
  "9": 0b1101111,
});
const SEGMENT = Object.freeze({
  top: 0b0000001,
  upperLeft: 0b0000010,
  upperRight: 0b0000100,
  middle: 0b0001000,
  lowerLeft: 0b0010000,
  lowerRight: 0b0100000,
  bottom: 0b1000000,
});
const DASHBOARD_RENDER_CONSTANTS = Object.freeze({
  dateW: 744,
  dateH: 44,
  dateCjkTextSize: 7,
  dateCjkDateX: 0,
  dateCjkWeekdayX: 142,
  dateCjkHolidayX: 222,
  batteryW: 176,
  batteryH: 44,
  calendarW: 304,
  calendarH: 232,
  calendarCellW: 42,
  calendarCellH: 24,
  calendarStartX: 5,
  calendarHeaderY: 54,
  calendarGridY: 86,
  timeW: 472,
  timeH: 236,
  timeDigitW: 92,
  timeDigitH: 210,
  timeGap: 18,
  timeColonW: 30,
  timeDrawX: 7,
  timeDigitY: 13,
  summaryW: 332,
  summaryH: 86,
  summaryTitleX: 16,
  summaryTitleY: 9,
  summaryPriceRight: 318,
  summaryPriceY: 10,
  summaryBottomY: 58,
  summaryArrowGap: 12,
  summaryValueGap: 14,
  climateW: 456,
  climateH: 86,
  climateHumidityDividerX: 126,
  climateTemperatureDividerX: 286,
});
const CLASSIC_RENDER_CONSTANTS = Object.freeze({
  dateX: 24,
  dateY: 20,
  batteryX: 768,
  batteryY: 20,
  timeX: 100,
  timeY: 72,
  timeW: 760,
  timeH: 300,
  timeDigitW: 150,
  timeDigitH: 280,
  timeGap: 24,
  timeDigitY: 10,
  infoX: 100,
  infoY: 378,
  infoW: 760,
  infoH: 120,
  smallDigitW: 54,
  smallDigitH: 96,
  smallGap: 8,
  infoDigitY: 6,
  humidityDigitX: 40,
  humidityUnitX: 226,
  humidityUnitY: 72,
  temperatureDigitX: 280,
  temperatureUnitX: 495,
  temperatureUnitY: 72,
  temperatureDegreeX: 480,
  temperatureDegreeY: 62,
  comfortCenterX: 640,
  comfortCenterY: 58,
});

function uiTextPixelSize(legacySize) {
  if (legacySize === 2) return 20;
  if (legacySize === 3) return 30;
  if (legacySize === 4) return 38;
  if (legacySize === 5) return 48;
  if (legacySize === 6) return 60;
  if (legacySize === 7) return 24;
  return legacySize * 10;
}

const BUILTIN_FONT_5X7 = Object.freeze({
  " ": ["00000", "00000", "00000", "00000", "00000", "00000", "00000"],
  "+": ["00000", "00100", "00100", "11111", "00100", "00100", "00000"],
  "-": ["00000", "00000", "00000", "11111", "00000", "00000", "00000"],
  ".": ["00000", "00000", "00000", "00000", "00000", "01100", "01100"],
  "%": ["11001", "11010", "00100", "01000", "10110", "00110", "00000"],
  "(": ["00010", "00100", "01000", "01000", "01000", "00100", "00010"],
  ")": ["01000", "00100", "00010", "00010", "00010", "00100", "01000"],
  "0": ["01110", "10001", "10011", "10101", "11001", "10001", "01110"],
  "1": ["00100", "01100", "00100", "00100", "00100", "00100", "01110"],
  "2": ["01110", "10001", "00001", "00010", "00100", "01000", "11111"],
  "3": ["11110", "00001", "00001", "01110", "00001", "00001", "11110"],
  "4": ["00010", "00110", "01010", "10010", "11111", "00010", "00010"],
  "5": ["11111", "10000", "10000", "11110", "00001", "00001", "11110"],
  "6": ["00110", "01000", "10000", "11110", "10001", "10001", "01110"],
  "7": ["11111", "00001", "00010", "00100", "01000", "01000", "01000"],
  "8": ["01110", "10001", "10001", "01110", "10001", "10001", "01110"],
  "9": ["01110", "10001", "10001", "01111", "00001", "00010", "01100"],
  "C": ["01110", "10001", "10000", "10000", "10000", "10001", "01110"],
  "o": ["00000", "00000", "01110", "10001", "10001", "10001", "01110"],
});

function canDrawBuiltinText(text) {
  return Array.from(String(text)).every((char) => BUILTIN_FONT_5X7[char]);
}

class InkCanvas {
  constructor(ctx, width, height) {
    this.ctx = ctx;
    this.width = width;
    this.height = height;
    this.textColor = INK.text;
    this.textDatum = TEXT_DATUM.TL;
    this.textSize = 20;
    this.legacyTextSize = 2;
    this.useCjkFont = false;
  }

  child(x, y, w, h) {
    this.ctx.save();
    this.ctx.translate(x, y);
    this.ctx.beginPath();
    this.ctx.rect(0, 0, w, h);
    this.ctx.clip();
    return new InkCanvas(this.ctx, w, h);
  }

  restore() {
    this.ctx.restore();
  }

  fillCanvas(color = INK.white) {
    this.fillRect(0, 0, this.width, this.height, color);
  }

  setTextColor(color) {
    this.textColor = color;
  }

  setTextDatum(datum) {
    this.textDatum = datum;
  }

  setTextSize(legacySize, useCjkFont = false) {
    this.useCjkFont = useCjkFont;
    this.legacyTextSize = legacySize;
    this.textSize = useCjkFont ? uiTextPixelSize(legacySize) : legacySize * 10;
  }

  font(weight = 400) {
    const family = this.useCjkFont
      ? `"${DASHBOARD_PREVIEW_FONT.cjkFamily}", ${DASHBOARD_PREVIEW_FONT.fallbackCjkFamily}`
      : DASHBOARD_PREVIEW_FONT.asciiFamily;
    return `${weight} ${this.textSize}px ${family}`;
  }

  textWidth(text) {
    if (!this.useCjkFont && canDrawBuiltinText(text)) {
      return String(text).length * 6 * this.legacyTextSize;
    }
    this.ctx.save();
    this.ctx.font = this.font();
    const width = this.ctx.measureText(String(text)).width;
    this.ctx.restore();
    return width;
  }

  drawString(text, x, y, options = {}) {
    const value = String(text);
    if (!this.useCjkFont && canDrawBuiltinText(value)) {
      this.drawBuiltinString(value, x, y, options.color || this.textColor);
      return;
    }
    this.ctx.save();
    this.ctx.fillStyle = options.color || this.textColor;
    this.ctx.font = this.font(options.weight || 400);
    const metrics = this.ctx.measureText(value);
    const width = metrics.width;
    const ascent = metrics.actualBoundingBoxAscent || this.textSize * 0.8;
    const descent = metrics.actualBoundingBoxDescent || this.textSize * 0.2;
    let drawX = x;
    let drawY = y + ascent;
    if (this.textDatum === TEXT_DATUM.TR) {
      drawX = x - width;
    } else if (this.textDatum === TEXT_DATUM.CL) {
      drawY = y + (ascent - descent) / 2;
    } else if (this.textDatum === TEXT_DATUM.CR) {
      drawX = x - width;
      drawY = y + (ascent - descent) / 2;
    } else if (this.textDatum === TEXT_DATUM.CC) {
      drawX = x - width / 2;
      drawY = y + (ascent - descent) / 2;
    }
    this.ctx.textAlign = "left";
    this.ctx.textBaseline = "alphabetic";
    this.ctx.fillText(value, drawX, drawY);
    this.ctx.restore();
  }

  drawBuiltinString(text, x, y, color) {
    const value = String(text);
    const scale = this.legacyTextSize;
    const width = value.length * 6 * scale;
    const height = 8 * scale;
    let drawX = x;
    let drawY = y;
    if (this.textDatum === TEXT_DATUM.TR) {
      drawX = x - width;
    } else if (this.textDatum === TEXT_DATUM.CL) {
      drawY = y - height / 2;
    } else if (this.textDatum === TEXT_DATUM.CR) {
      drawX = x - width;
      drawY = y - height / 2;
    } else if (this.textDatum === TEXT_DATUM.CC) {
      drawX = x - width / 2;
      drawY = y - height / 2;
    }

    this.ctx.save();
    this.ctx.fillStyle = color;
    Array.from(value).forEach((char, charIndex) => {
      const glyph = BUILTIN_FONT_5X7[char];
      const charX = drawX + charIndex * 6 * scale;
      glyph.forEach((row, rowIndex) => {
        Array.from(row).forEach((pixel, colIndex) => {
          if (pixel === "1") {
            this.ctx.fillRect(
              charX + colIndex * scale,
              drawY + rowIndex * scale,
              scale,
              scale,
            );
          }
        });
      });
    });
    this.ctx.restore();
  }

  drawFauxBoldString(text, x, y, offsetX = 1) {
    this.drawString(text, x, y);
    if (String(text).length > 0 && offsetX !== 0) {
      this.drawString(text, x + offsetX, y);
    }
  }

  fillRect(x, y, w, h, color) {
    this.ctx.save();
    this.ctx.fillStyle = color;
    this.ctx.fillRect(x, y, w, h);
    this.ctx.restore();
  }

  pushImage4bpp(x, y, w, h, data) {
    const bitmapCanvas = document.createElement("canvas");
    bitmapCanvas.width = w;
    bitmapCanvas.height = h;
    const bitmapCtx = bitmapCanvas.getContext("2d");
    const imageData = bitmapCtx.createImageData(w, h);
    const pixels = imageData.data;
    for (let pixelIndex = 0; pixelIndex < w * h; pixelIndex += 1) {
      const byte = data[Math.floor(pixelIndex / 2)] || 0;
      const value = pixelIndex % 2 === 0 ? byte >> 4 : byte & 0x0f;
      const gray = 255 - value * 17;
      const outputIndex = pixelIndex * 4;
      pixels[outputIndex] = gray;
      pixels[outputIndex + 1] = gray;
      pixels[outputIndex + 2] = gray;
      pixels[outputIndex + 3] = 255;
    }
    bitmapCtx.putImageData(imageData, 0, 0);
    this.ctx.drawImage(bitmapCanvas, x, y);
  }

  drawRect(x, y, w, h, color = INK.border, lineWidth = 1) {
    this.ctx.save();
    this.ctx.strokeStyle = color;
    this.ctx.lineWidth = lineWidth;
    this.ctx.strokeRect(x + 0.5, y + 0.5, w - 1, h - 1);
    this.ctx.restore();
  }

  drawRoundRect(x, y, w, h, radius, color = INK.border, lineWidth = 1) {
    this.ctx.save();
    this.ctx.strokeStyle = color;
    this.ctx.lineWidth = lineWidth;
    this.ctx.beginPath();
    this.ctx.roundRect(x + 0.5, y + 0.5, w - 1, h - 1, radius);
    this.ctx.stroke();
    this.ctx.restore();
  }

  drawFastVLine(x, y, h, color = INK.border) {
    this.fillRect(x, y, 1, h, color);
  }

  drawFastHLine(x, y, w, color = INK.border) {
    this.fillRect(x, y, w, 1, color);
  }

  drawLine(x0, y0, x1, y1, color = INK.text, lineWidth = 1) {
    if (lineWidth <= 1) {
      this.drawPixelLine(x0, y0, x1, y1, color);
      return;
    }
    this.ctx.save();
    this.ctx.strokeStyle = color;
    this.ctx.lineWidth = lineWidth;
    this.ctx.beginPath();
    this.ctx.moveTo(x0 + 0.5, y0 + 0.5);
    this.ctx.lineTo(x1 + 0.5, y1 + 0.5);
    this.ctx.stroke();
    this.ctx.restore();
  }

  drawPixelLine(x0, y0, x1, y1, color) {
    let currentX = Math.round(x0);
    let currentY = Math.round(y0);
    const targetX = Math.round(x1);
    const targetY = Math.round(y1);
    const dx = Math.abs(targetX - currentX);
    const sx = currentX < targetX ? 1 : -1;
    const dy = -Math.abs(targetY - currentY);
    const sy = currentY < targetY ? 1 : -1;
    let error = dx + dy;

    this.ctx.save();
    this.ctx.fillStyle = color;
    while (true) {
      this.ctx.fillRect(currentX, currentY, 1, 1);
      if (currentX === targetX && currentY === targetY) {
        break;
      }
      const nextError = 2 * error;
      if (nextError >= dy) {
        error += dy;
        currentX += sx;
      }
      if (nextError <= dx) {
        error += dx;
        currentY += sy;
      }
    }
    this.ctx.restore();
  }

  fillTriangle(x1, y1, x2, y2, x3, y3, color) {
    this.ctx.save();
    this.ctx.fillStyle = color;
    this.ctx.beginPath();
    this.ctx.moveTo(x1, y1);
    this.ctx.lineTo(x2, y2);
    this.ctx.lineTo(x3, y3);
    this.ctx.closePath();
    this.ctx.fill();
    this.ctx.restore();
  }

  fillCircle(x, y, radius, color) {
    this.ctx.save();
    this.ctx.fillStyle = color;
    this.ctx.beginPath();
    this.ctx.arc(x, y, radius, 0, Math.PI * 2);
    this.ctx.fill();
    this.ctx.restore();
  }
}

class WebSegmentRenderer {
  charWidth(ch, digitWidth) {
    if (ch === ":") return Math.floor(digitWidth / 3);
    if (ch === ".") return Math.floor(digitWidth / 5);
    return digitWidth;
  }

  drawText(canvas, x, y, text, digitWidth, digitHeight, gap, bodyColor, edgeColor) {
    let cursorX = x;
    for (const ch of String(text)) {
      if (ch >= "0" && ch <= "9") {
        this.drawDigit(canvas, cursorX, y, ch, digitWidth, digitHeight, bodyColor, edgeColor);
      } else if (ch === "-") {
        this.drawMinus(canvas, cursorX, y, digitWidth, digitHeight, bodyColor, edgeColor);
      } else if (ch === ":") {
        this.drawColon(canvas, cursorX, y, digitWidth, digitHeight, bodyColor);
      } else if (ch === ".") {
        this.drawDot(canvas, cursorX, y, digitWidth, digitHeight, bodyColor);
      }
      cursorX += this.charWidth(ch, digitWidth) + gap;
    }
  }

  drawDigit(canvas, x, y, digit, digitWidth, digitHeight, bodyColor, edgeColor) {
    const mask = SEGMENT_MASKS[digit];
    const thickness = Math.max(6, Math.floor(digitWidth / 7));
    const slant = Math.max(2, Math.floor(thickness / 2));
    const innerInset = Math.max(2, Math.floor(thickness / 5));
    const horizontalLength = digitWidth - thickness;
    const verticalLength = Math.floor(digitHeight / 2) - Math.floor(thickness / 2);
    const drawHorizontal = (sx, sy) => {
      this.drawHorizontalSegment(canvas, sx, sy, horizontalLength, thickness, slant, edgeColor);
      this.drawHorizontalSegment(canvas, sx + innerInset, sy + innerInset,
        horizontalLength - innerInset * 2, thickness - innerInset * 2,
        Math.max(1, slant - innerInset), bodyColor);
    };
    const drawVertical = (sx, sy) => {
      this.drawVerticalSegment(canvas, sx, sy, verticalLength, thickness, slant, edgeColor);
      this.drawVerticalSegment(canvas, sx + innerInset, sy + innerInset,
        verticalLength - innerInset * 2, thickness - innerInset * 2,
        Math.max(1, slant - innerInset), bodyColor);
    };
    const topX = x + Math.floor(thickness / 2);
    const upperY = y + Math.floor(thickness / 2);
    const middleY = y + Math.floor((digitHeight - thickness) / 2);
    const lowerY = y + Math.floor(digitHeight / 2);
    const rightX = x + digitWidth - thickness;
    if (mask & SEGMENT.top) drawHorizontal(topX, y);
    if (mask & SEGMENT.upperLeft) drawVertical(x, upperY);
    if (mask & SEGMENT.upperRight) drawVertical(rightX, upperY);
    if (mask & SEGMENT.middle) drawHorizontal(topX, middleY);
    if (mask & SEGMENT.lowerLeft) drawVertical(x, lowerY);
    if (mask & SEGMENT.lowerRight) drawVertical(rightX, lowerY);
    if (mask & SEGMENT.bottom) drawHorizontal(topX, y + digitHeight - thickness);
  }

  drawColon(canvas, x, y, digitWidth, digitHeight, color) {
    const radius = Math.max(3, Math.floor(digitWidth / 10));
    const centerX = x + Math.floor(digitWidth / 6);
    canvas.fillCircle(centerX, y + Math.floor(digitHeight / 3), radius, color);
    canvas.fillCircle(centerX, y + Math.floor((digitHeight * 2) / 3), radius, color);
  }

  drawDot(canvas, x, y, digitWidth, digitHeight, color) {
    const radius = Math.max(3, Math.floor(digitWidth / 12));
    canvas.fillCircle(x + radius, y + digitHeight - radius * 2, radius, color);
  }

  drawMinus(canvas, x, y, digitWidth, digitHeight, bodyColor, edgeColor) {
    const thickness = Math.max(6, Math.floor(digitWidth / 7));
    const slant = Math.max(2, Math.floor(thickness / 2));
    const innerInset = Math.max(2, Math.floor(thickness / 5));
    const horizontalLength = digitWidth - thickness;
    const middleY = y + Math.floor((digitHeight - thickness) / 2);
    const topX = x + Math.floor(thickness / 2);
    this.drawHorizontalSegment(canvas, topX, middleY, horizontalLength, thickness, slant, edgeColor);
    this.drawHorizontalSegment(canvas, topX + innerInset, middleY + innerInset,
      horizontalLength - innerInset * 2, thickness - innerInset * 2,
      Math.max(1, slant - innerInset), bodyColor);
  }

  drawHorizontalSegment(canvas, x, y, length, thickness, slant, color) {
    if (length <= 0 || thickness <= 0) return;
    canvas.fillRect(x + slant, y, length - slant * 2, thickness, color);
    canvas.fillTriangle(x, y + Math.floor(thickness / 2), x + slant, y,
      x + slant, y + thickness, color);
    canvas.fillTriangle(x + length, y + Math.floor(thickness / 2),
      x + length - slant, y, x + length - slant, y + thickness, color);
  }

  drawVerticalSegment(canvas, x, y, length, thickness, slant, color) {
    if (length <= 0 || thickness <= 0) return;
    canvas.fillRect(x, y + slant, thickness, length - slant * 2, color);
    canvas.fillTriangle(x + Math.floor(thickness / 2), y, x, y + slant,
      x + thickness, y + slant, color);
    canvas.fillTriangle(x + Math.floor(thickness / 2), y + length,
      x, y + length - slant, x + thickness, y + length - slant, color);
  }
}

const previewSegmentRenderer = new WebSegmentRenderer();

function drawWideLine(canvas, x0, y0, x1, y1, color, thickness) {
  const radius = Math.floor(thickness / 2);
  for (let offset = -radius; offset <= radius; offset += 1) {
    canvas.drawLine(x0, y0 + offset, x1, y1 + offset, color);
    if (offset !== 0) {
      canvas.drawLine(x0 + offset, y0, x1 + offset, y1, color);
    }
  }
}

function drawThickLine(canvas, x0, y0, x1, y1, color) {
  canvas.drawLine(x0, y0, x1, y1, color);
  canvas.drawLine(x0 + 1, y0, x1 + 1, y1, color);
}

function drawCaret(canvas, x, bottomY, halfW, rise, color, thickness) {
  drawWideLine(canvas, x - halfW, bottomY, x, bottomY - rise, color, thickness);
  drawWideLine(canvas, x + halfW, bottomY, x, bottomY - rise, color, thickness);
}

function drawDash(canvas, x, y, halfW, color, thickness) {
  const radius = Math.floor(thickness / 2);
  for (let offset = -radius; offset <= radius; offset += 1) {
    canvas.drawFastHLine(x - halfW, y + offset, halfW * 2 + 1, color);
  }
}

function drawComponentFrame(canvas, item) {
  canvas.fillCanvas(INK.white);
  canvas.drawRoundRect(0, 0, item.w, item.h, 6, INK.border);
}

function drawPreviewDate(canvas, item) {
  const c = DASHBOARD_RENDER_CONSTANTS;
  const weekday = DASHBOARD_PREVIEW_SAMPLE.weekdayZh;
  const holiday = DASHBOARD_PREVIEW_SAMPLE.holidayZh;
  canvas.setTextDatum(TEXT_DATUM.CL);
  canvas.setTextColor(INK.text);
  canvas.setTextSize(c.dateCjkTextSize, true);
  canvas.drawFauxBoldString(DASHBOARD_PREVIEW_SAMPLE.date, c.dateCjkDateX, item.h / 2);
  canvas.drawFauxBoldString(weekday, c.dateCjkWeekdayX, item.h / 2);
  canvas.drawFauxBoldString(holiday, c.dateCjkHolidayX, item.h / 2);
}

function drawWifiStatusIcon(canvas, originX, originY, connected = true) {
  const bitmap = previewWifiBitmapForLevel(DASHBOARD_PREVIEW_SAMPLE.wifiSignalLevel);
  if (bitmap) {
    canvas.pushImage4bpp(originX, originY, bitmap.width, bitmap.height, bitmap.data);
  } else {
    drawLegacyWifiStatusIcon(canvas, originX, originY, connected);
    return;
  }
  if (!connected) {
    canvas.drawLine(originX + 5, originY + 26, originX + 26, originY + 5, INK.text);
    canvas.drawLine(originX + 6, originY + 26, originX + 27, originY + 5, INK.text);
  }
}

function drawLegacyWifiStatusIcon(canvas, originX, originY, connected = true) {
  canvas.drawLine(originX + 5, originY + 19, originX + 16, originY + 8, INK.text, 3);
  canvas.drawLine(originX + 16, originY + 8, originX + 27, originY + 19, INK.text, 3);
  canvas.drawLine(originX + 9, originY + 23, originX + 16, originY + 16, INK.text, 3);
  canvas.drawLine(originX + 16, originY + 16, originX + 23, originY + 23, INK.text, 3);
  canvas.fillCircle(originX + 16, originY + 27, 3, INK.text);
  if (!connected) {
    canvas.drawLine(originX + 5, originY + 26, originX + 26, originY + 5, INK.text);
    canvas.drawLine(originX + 6, originY + 26, originX + 27, originY + 5, INK.text);
  }
}

function drawUsbStatusIcon(canvas, originX, originY) {
  const cx = originX + 9;
  canvas.fillTriangle(cx, originY + 1, originX + 6, originY + 7,
    originX + 12, originY + 7, INK.text);
  canvas.fillRect(cx, originY + 7, 2, 11, INK.text);
  canvas.drawLine(cx, originY + 16, originX + 5, originY + 13, INK.text);
  canvas.drawLine(originX + 5, originY + 13, originX + 5, originY + 11, INK.text);
  canvas.fillCircle(originX + 5, originY + 9, 2, INK.text);
  canvas.drawLine(cx + 1, originY + 14, originX + 15, originY + 10, INK.text);
  canvas.drawLine(originX + 15, originY + 10, originX + 15, originY + 8, INK.text);
  canvas.fillRect(originX + 13, originY + 5, 4, 4, INK.text);
  canvas.fillCircle(cx + 1, originY + 20, 2, INK.text);
}

function drawBluetoothStatusIcon(canvas, originX, originY) {
  const spineX = originX + 8;
  const topY = originY + 2;
  const midY = originY + 12;
  const bottomY = originY + 22;
  const armX = originX + 16;
  drawThickLine(canvas, spineX, topY, spineX, bottomY, INK.text);
  drawThickLine(canvas, spineX, topY, armX, originY + 8, INK.text);
  drawThickLine(canvas, armX, originY + 8, spineX, midY, INK.text);
  drawThickLine(canvas, spineX, midY, armX, originY + 16, INK.text);
  drawThickLine(canvas, armX, originY + 16, spineX, bottomY, INK.text);
  drawThickLine(canvas, originX + 3, originY + 6, spineX, midY, INK.text);
  drawThickLine(canvas, originX + 3, originY + 18, spineX, midY, INK.text);
}

function drawCompactComfortInfoAt(canvas, centerX, centerY, face, color, useCjkFont) {
  canvas.setTextDatum(TEXT_DATUM.CC);
  canvas.setTextColor(color);
  canvas.setTextSize(4, useCjkFont);
  canvas.drawString("(", centerX - 40, centerY);
  canvas.drawString(")", centerX + 40, centerY);

  if (face === "(^_^)") {
    drawCaret(canvas, centerX - 18, centerY - 2, 6, 7, color, 4);
    drawCaret(canvas, centerX + 18, centerY - 2, 6, 7, color, 4);
    drawDash(canvas, centerX, centerY + 10, 7, color, 4);
    return;
  }

  if (face === "(-^-)") {
    drawDash(canvas, centerX - 20, centerY - 5, 6, color, 2);
    drawDash(canvas, centerX + 20, centerY - 5, 6, color, 2);
    drawCaret(canvas, centerX, centerY + 9, 6, 7, color, 2);
    return;
  }

  drawDash(canvas, centerX - 20, centerY - 5, 6, color, 2);
  drawDash(canvas, centerX + 20, centerY - 5, 6, color, 2);
  drawDash(canvas, centerX, centerY + 10, 7, color, 2);
}

function drawClassicComfortInfoAt(canvas, centerX, centerY, face, color, useCjkFont) {
  canvas.setTextDatum(TEXT_DATUM.CC);
  canvas.setTextColor(color);
  canvas.setTextSize(6, useCjkFont);
  canvas.drawString("(", centerX - 78, centerY);
  canvas.drawString(")", centerX + 78, centerY);

  if (face === "(^_^)") {
    drawCaret(canvas, centerX - 30, centerY - 2, 10, 12, color, 4);
    drawCaret(canvas, centerX + 30, centerY - 2, 10, 12, color, 4);
    drawDash(canvas, centerX, centerY + 18, 12, color, 4);
    return;
  }

  if (face === "(-^-)") {
    drawDash(canvas, centerX - 34, centerY - 8, 10, color, 2);
    drawDash(canvas, centerX + 34, centerY - 8, 10, color, 2);
    drawCaret(canvas, centerX, centerY + 14, 9, 10, color, 2);
    return;
  }

  drawDash(canvas, centerX - 34, centerY - 8, 10, color, 2);
  drawDash(canvas, centerX + 34, centerY - 8, 10, color, 2);
  drawDash(canvas, centerX, centerY + 18, 12, color, 2);
}

function drawPreviewBattery(canvas) {
  const c = DASHBOARD_RENDER_CONSTANTS;
  const battery = DASHBOARD_PREVIEW_SAMPLE.batteryPercent;
  const batteryLabel = `${String(battery).padStart(3, " ")}%`;
  const bodyW = 38;
  const bodyH = 20;
  const capW = 4;
  const capH = 10;
  const iconRight = c.batteryW - 2;
  const capX = iconRight - capW;
  const bodyX = capX - bodyW;
  const bodyY = 12;
  const capY = bodyY + 5;
  const innerX = bodyX + 3;
  const innerY = bodyY + 3;
  const innerW = bodyW - 6;
  const innerH = bodyH - 6;
  const fillW = Math.floor((battery * innerW) / 100);
  const labelRight = bodyX - 10;
  canvas.fillCanvas(INK.white);
  canvas.setTextColor(INK.text);
  canvas.setTextSize(2, false);
  canvas.setTextDatum(TEXT_DATUM.CR);
  const labelWidth = canvas.textWidth(batteryLabel);
  const wifiGap = 8;
  const wifiSize = 32;
  const wifiX = labelRight - labelWidth - wifiGap - wifiSize;
  const wifiY = 6;
  const transportGap = 10;
  const transportSize = 18;
  const transportX = wifiX - transportGap - transportSize;
  const transportY = 10;
  canvas.drawString(batteryLabel, labelRight, c.batteryH / 2);
  if (DASHBOARD_PREVIEW_SAMPLE.configConnectionIcon === "serial") {
    drawUsbStatusIcon(canvas, transportX, transportY);
  } else if (DASHBOARD_PREVIEW_SAMPLE.configConnectionIcon === "bluetooth") {
    drawBluetoothStatusIcon(canvas, transportX, transportY);
  }
  drawWifiStatusIcon(canvas, wifiX, wifiY, true);
  canvas.drawRoundRect(bodyX, bodyY, bodyW, bodyH, 3, INK.text);
  canvas.fillRect(capX, capY, capW, capH, INK.text);
  if (fillW > 0) {
    canvas.fillRect(innerX, innerY, fillW, innerH, INK.text);
  }
}

function drawPreviewCalendar(canvas) {
  const c = DASHBOARD_RENDER_CONSTANTS;
  canvas.fillCanvas(INK.white);
  canvas.setTextColor(INK.text);
  canvas.setTextDatum(TEXT_DATUM.CC);
  canvas.setTextSize(5, false);
  canvas.drawString("04", c.calendarW / 2, 24, { weight: 700 });
  const weekdays =
    ["日", "一", "二", "三", "四", "五", "六"];
  weekdays.forEach((day, index) => {
    canvas.setTextSize(2, true);
    canvas.drawString(day, c.calendarStartX + index * c.calendarCellW +
      Math.floor((c.calendarCellW - 2) / 2), c.calendarHeaderY + 8);
  });
  canvas.setTextSize(2, false);
  for (let day = 1; day <= 30; day += 1) {
    const slot = day + 2;
    const row = Math.floor(slot / 7);
    const col = slot % 7;
    const x = c.calendarStartX + col * c.calendarCellW;
    const y = c.calendarGridY + row * c.calendarCellH;
    if (day === 24) {
      canvas.fillRect(x, y, c.calendarCellW - 2, c.calendarCellH - 2,
        INK.calendarTodayFill);
    } else if (day < 24) {
      canvas.fillRect(x, y, c.calendarCellW - 2, c.calendarCellH - 2,
        INK.calendarPastFill);
    }
    canvas.drawRect(x, y, c.calendarCellW - 2, c.calendarCellH - 2, INK.border);
    canvas.setTextColor(day === 24 ? INK.white : INK.text);
    canvas.drawString(String(day), x + Math.floor((c.calendarCellW - 2) / 2),
      y + Math.floor((c.calendarCellH - 2) / 2), { weight: 700 });
  }
}

function drawPreviewTime(canvas) {
  const c = DASHBOARD_RENDER_CONSTANTS;
  canvas.fillCanvas(INK.white);
  previewSegmentRenderer.drawText(canvas, c.timeDrawX, c.timeDigitY,
    DASHBOARD_PREVIEW_SAMPLE.time, c.timeDigitW, c.timeDigitH, c.timeGap,
    INK.text, INK.edgeText);
}

function drawPreviewMarket(canvas, item) {
  drawComponentFrame(canvas, item);
  const name = DASHBOARD_PREVIEW_SAMPLE.marketNameZh;
  const status = DASHBOARD_PREVIEW_SAMPLE.marketStatusZh;
  canvas.setTextDatum(TEXT_DATUM.TL);
  canvas.setTextColor(INK.text);
  canvas.setTextSize(2, true);
  canvas.drawFauxBoldString(name, 16, 9);
  canvas.setTextDatum(TEXT_DATUM.TR);
  canvas.setTextSize(4, false);
  canvas.drawString(DASHBOARD_PREVIEW_SAMPLE.marketValue, item.w - 14, 10);
  canvas.setTextSize(2, false);
  const percentValue = "-0.33%";
  const changeValue = "-13.35";
  const percentRight = item.w - 14;
  const percentWidth = canvas.textWidth(percentValue);
  const changeRight = percentRight - percentWidth - DASHBOARD_RENDER_CONSTANTS.summaryValueGap;
  const changeWidth = canvas.textWidth(changeValue);
  const changeLeft = changeRight - changeWidth;
  const arrowCenterX = changeLeft - DASHBOARD_RENDER_CONSTANTS.summaryArrowGap;
  const arrowCenterY = DASHBOARD_RENDER_CONSTANTS.summaryBottomY + 5;
  canvas.fillTriangle(arrowCenterX - 8, arrowCenterY - 6, arrowCenterX,
    arrowCenterY + 6, arrowCenterX + 8, arrowCenterY - 6, INK.text);
  canvas.setTextDatum(TEXT_DATUM.TL);
  canvas.setTextColor(INK.mutedText);
  canvas.setTextSize(2, true);
  canvas.drawString(status, 16, 58);
  canvas.setTextColor(INK.text);
  canvas.setTextSize(2, false);
  canvas.setTextDatum(TEXT_DATUM.TR);
  canvas.drawString(changeValue, changeRight, DASHBOARD_RENDER_CONSTANTS.summaryBottomY);
  canvas.drawString(percentValue, percentRight, DASHBOARD_RENDER_CONSTANTS.summaryBottomY);
}

function drawPreviewClimate(canvas, item) {
  const c = DASHBOARD_RENDER_CONSTANTS;
  drawComponentFrame(canvas, item);
  canvas.drawFastVLine(c.climateHumidityDividerX, 18, 50, INK.border);
  canvas.drawFastVLine(c.climateTemperatureDividerX, 18, 50, INK.border);
  const climateValueY = Math.floor(c.climateH / 2) + 2;
  canvas.setTextDatum(TEXT_DATUM.CL);
  canvas.setTextColor(INK.text);
  canvas.setTextSize(5, false);
  canvas.drawString(DASHBOARD_PREVIEW_SAMPLE.humidity, 18, climateValueY);
  const humidityWidth = canvas.textWidth(DASHBOARD_PREVIEW_SAMPLE.humidity);
  canvas.setTextSize(2, false);
  canvas.drawString("%", 18 + humidityWidth + 6, climateValueY + 14);
  canvas.setTextSize(5, false);
  canvas.drawString(DASHBOARD_PREVIEW_SAMPLE.temperature, 134, climateValueY);
  const temperatureWidth = canvas.textWidth(DASHBOARD_PREVIEW_SAMPLE.temperature);
  const unitX = 134 + temperatureWidth + 4;
  canvas.setTextSize(2, false);
  canvas.drawString("o", unitX, climateValueY - 8);
  canvas.drawString("C", unitX + 14, climateValueY + 14);
  drawCompactComfortInfoAt(canvas,
    c.climateTemperatureDividerX + Math.floor((item.w - c.climateTemperatureDividerX) / 2),
    Math.floor(c.climateH / 2) + 2, DASHBOARD_PREVIEW_SAMPLE.comfortFace,
    INK.text, false);
}

function drawClassicPreviewInfo(canvas) {
  const c = CLASSIC_RENDER_CONSTANTS;
  canvas.fillCanvas(INK.white);
  previewSegmentRenderer.drawText(canvas, c.humidityDigitX, c.infoDigitY,
    DASHBOARD_PREVIEW_SAMPLE.humidity.padStart(3, " "), c.smallDigitW,
    c.smallDigitH, c.smallGap, INK.text, INK.mutedText);
  canvas.setTextDatum(TEXT_DATUM.TL);
  canvas.setTextColor(INK.text);
  canvas.setTextSize(3, false);
  canvas.drawString("%", c.humidityUnitX, c.humidityUnitY);

  const temperatureDigits = DASHBOARD_PREVIEW_SAMPLE.temperature.replace(".", "").padStart(3, " ");
  const temperatureDisplay = `${temperatureDigits.slice(0, 2)}.${temperatureDigits.slice(2)}`;
  previewSegmentRenderer.drawText(canvas, c.temperatureDigitX, c.infoDigitY,
    temperatureDisplay, c.smallDigitW, c.smallDigitH, c.smallGap,
    INK.text, INK.mutedText);
  canvas.setTextSize(3, false);
  canvas.drawString(" C", c.temperatureUnitX, c.temperatureUnitY);
  canvas.setTextSize(2, false);
  canvas.drawString("o", c.temperatureDegreeX, c.temperatureDegreeY);

  drawClassicComfortInfoAt(canvas, c.comfortCenterX, c.comfortCenterY,
    DASHBOARD_PREVIEW_SAMPLE.comfortFace, INK.text, false);
}

function drawPreviewClassicTime(canvas) {
  const c = CLASSIC_RENDER_CONSTANTS;
  canvas.fillCanvas(INK.white);
  previewSegmentRenderer.drawText(canvas, 7, c.timeDigitY,
    DASHBOARD_PREVIEW_SAMPLE.time, c.timeDigitW, c.timeDigitH, c.timeGap,
    INK.text, INK.edgeText);
}

const DASHBOARD_COMPONENT_RENDERERS = Object.freeze({
  date: drawPreviewDate,
  battery: drawPreviewBattery,
  calendar: drawPreviewCalendar,
  time: drawPreviewTime,
  market: drawPreviewMarket,
  climate: drawPreviewClimate,
  "classic-time": drawPreviewClassicTime,
  "classic-info": drawClassicPreviewInfo,
});

function renderDashboardPreviewCanvas(ctx) {
  const rootCanvas = new InkCanvas(ctx, DASHBOARD_PREVIEW.width, DASHBOARD_PREVIEW.height);
  rootCanvas.fillCanvas(INK.paper);
  rootCanvas.drawRect(12, 12, DASHBOARD_PREVIEW.width - 24,
    DASHBOARD_PREVIEW.height - 24, INK.frame);

  for (const item of state.dashboardLayout.filter((layoutItem) => layoutItem.visible !== false)) {
    if (item.visible === false) {
      continue;
    }
    const renderComponent = DASHBOARD_COMPONENT_RENDERERS[item.type];
    if (!renderComponent) {
      continue;
    }
    const componentCanvas = rootCanvas.child(item.x, item.y, item.w, item.h);
    renderComponent(componentCanvas, item);
    componentCanvas.restore();
  }
}

function renderClassicPreviewCanvas(ctx) {
  const c = CLASSIC_RENDER_CONSTANTS;
  const rootCanvas = new InkCanvas(ctx, DASHBOARD_PREVIEW.width, DASHBOARD_PREVIEW.height);
  rootCanvas.fillCanvas(INK.paper);

  const dateCanvas = rootCanvas.child(c.dateX, c.dateY,
    DASHBOARD_RENDER_CONSTANTS.dateW, DASHBOARD_RENDER_CONSTANTS.dateH);
  drawPreviewDate(dateCanvas, {
    w: DASHBOARD_RENDER_CONSTANTS.dateW,
    h: DASHBOARD_RENDER_CONSTANTS.dateH,
  });
  dateCanvas.restore();

  const batteryCanvas = rootCanvas.child(c.batteryX, c.batteryY,
    DASHBOARD_RENDER_CONSTANTS.batteryW, DASHBOARD_RENDER_CONSTANTS.batteryH);
  drawPreviewBattery(batteryCanvas);
  batteryCanvas.restore();

  const timeCanvas = rootCanvas.child(c.timeX, c.timeY, c.timeW, c.timeH);
  timeCanvas.fillCanvas(INK.white);
  previewSegmentRenderer.drawText(timeCanvas, 7, c.timeDigitY,
    DASHBOARD_PREVIEW_SAMPLE.time, c.timeDigitW, c.timeDigitH, c.timeGap,
    INK.text, INK.edgeText);
  timeCanvas.restore();

  const infoCanvas = rootCanvas.child(c.infoX, c.infoY, c.infoW, c.infoH);
  drawClassicPreviewInfo(infoCanvas);
  infoCanvas.restore();
}

function setDashboardLayout(layout, { dirty = false } = {}) {
  state.dashboardLayout = dashboardLayoutWithLabels(layout).map(
    clampDashboardLayoutItem,
  );
  const index = state.dashboardLayouts.findIndex(
    (preset) => preset.id === state.activeLayoutId,
  );
  if (index >= 0) {
    state.dashboardLayouts[index] = {
      ...state.dashboardLayouts[index],
      components: cloneDashboardLayout(state.dashboardLayout),
    };
  } else {
    state.dashboardLayouts = [
      ...state.dashboardLayouts,
      createLayoutPreset(
        state.activeLayoutId || DEFAULT_LAYOUT_ID,
        DEFAULT_LAYOUT_NAME,
        state.dashboardLayout,
        "dashboard",
      ),
    ];
  }
  state.layoutDirty = dirty;
  if (dirty) {
    saveLayoutPresets();
  }
  renderDashboardLayoutEditor();
  setConnected(state.connected);
}

function setActiveDashboardLayout(id, { dirty = false } = {}) {
  const layout =
    state.dashboardLayouts.find((preset) => preset.id === id) ||
    state.dashboardLayouts[0] ||
    defaultDashboardLayoutPreset();
  state.activeLayoutId = layout.id;
  state.dashboardLayout = cloneDashboardLayout(layout.components);
  state.selectedLayoutId =
    state.dashboardLayout.find((item) => item.visible !== false)?.id ||
    state.dashboardLayout[0]?.id ||
    null;
  state.layoutDirty = dirty;
  saveLayoutPresets();
  renderDashboardLayoutEditor();
  setConnected(state.connected);
}

function createBlankDashboardPreset() {
  const preset = blankDashboardLayoutPreset();
  state.dashboardLayouts = [...state.dashboardLayouts, preset];
  setActiveDashboardLayout(preset.id, { dirty: true });
}

function duplicateDashboardPreset() {
  const source =
    state.dashboardLayouts.find((layout) => layout.id === state.activeLayoutId) ||
    defaultDashboardLayoutPreset();
  const preset = createLayoutPreset(
    uniqueLayoutId(`${source.id}-copy`),
    `${source.name} ${t("layout.copySuffix")}`,
    source.components,
    "dashboard",
  );
  state.dashboardLayouts = [...state.dashboardLayouts, preset];
  setActiveDashboardLayout(preset.id, { dirty: true });
}

function deleteDashboardPreset() {
  if (isBuiltInLayoutId(state.activeLayoutId)) {
    return;
  }
  const nextLayouts = state.dashboardLayouts.filter(
    (layout) => layout.id !== state.activeLayoutId,
  );
  state.dashboardLayouts = nextLayouts.length
    ? nextLayouts
    : [defaultDashboardLayoutPreset()];
  setActiveDashboardLayout(
    state.dashboardLayouts.find((layout) => layout.id === DEFAULT_LAYOUT_ID)?.id ||
      state.dashboardLayouts[0].id,
    { dirty: true },
  );
}

function applyTranslations() {
  document.documentElement.lang = state.locale;
  document.title = t("app.title");
  if (elements.languageSelect) {
    elements.languageSelect.value = state.locale;
    elements.languageSelect.setAttribute("aria-label", t("language.label"));
  }

  document.querySelectorAll("[data-i18n]").forEach((element) => {
    element.innerHTML = t(element.dataset.i18n);
  });
  document.querySelectorAll("[data-i18n-placeholder]").forEach((element) => {
    element.setAttribute("placeholder", t(element.dataset.i18nPlaceholder));
  });
  document.querySelectorAll("[data-i18n-alt]").forEach((element) => {
    element.setAttribute("alt", t(element.dataset.i18nAlt));
  });
}

function setLocale(locale) {
  if (!SUPPORTED_LOCALES.includes(locale)) {
    return;
  }
  state.locale = locale;
  window.localStorage.setItem(LANGUAGE_STORAGE_KEY, locale);
  applyTranslations();
  elements.togglePasswordButton.textContent =
    elements.passwordInput.type === "password"
      ? t("button.showPassword")
      : t("button.hidePassword");
  setConnected(state.connected);
  if (state.lastStatus) {
    updateStatus(state.lastStatus);
  }
  renderNetworks();
  renderOtaSummary(state.otaManifest);
  renderOtaProgress();
  renderLocalOtaSummary();
  renderDashboardLayoutEditor();
}

function normalizeMarketQuery(value) {
  return String(value || "")
    .replace(/\s+/g, "")
    .toLowerCase();
}

function currentMarketSymbol() {
  return normalizeMarketQuery(
    currentMarketLayoutItem()?.props?.symbol || state.lastStatus?.marketSymbol || "",
  );
}

function currentMarketLayoutItem() {
  const activeLayout =
    state.dashboardLayouts.find((layout) => layout.id === state.activeLayoutId) ||
    activeLayoutPreset();
  return activeLayout?.components?.find((item) => item.type === "market") || null;
}

function updateMarketSymbolInActiveLayout(symbol) {
  const requestSymbol = normalizeMarketQuery(symbol);
  if (!requestSymbol) {
    return false;
  }
  const activeLayout =
    state.dashboardLayouts.find((layout) => layout.id === state.activeLayoutId) ||
    null;
  if (!activeLayout || activeLayout.kind === "classic") {
    return false;
  }

  let updated = false;
  const nextComponents = activeLayout.components.map((item) => {
    if (item.type !== "market") {
      return item;
    }
    updated = true;
    return {
      ...item,
      props: {
        ...(item.props || {}),
        symbol: requestSymbol,
      },
    };
  });
  if (!updated) {
    return false;
  }

  state.dashboardLayouts = state.dashboardLayouts.map((layout) =>
    layout.id === activeLayout.id
      ? {
          ...layout,
          components: nextComponents,
        }
      : layout,
  );
  if (activeLayout.id === state.activeLayoutId) {
    state.dashboardLayout = cloneDashboardLayout(nextComponents);
  }
  state.layoutDirty = true;
  saveLayoutPresets();
  renderDashboardLayoutEditor();
  return true;
}

function normalizeVersion(value) {
  return String(value || "")
    .trim()
    .replace(/^v/i, "")
    .toLowerCase();
}

function isOtaUpdateAvailable(manifest) {
  const currentVersion = normalizeVersion(state.lastStatus?.firmwareVersion);
  const availableVersion = normalizeVersion(manifest?.version);
  if (!manifest?.ota?.url) {
    return false;
  }
  if (!currentVersion || !availableVersion) {
    return true;
  }
  return currentVersion !== availableVersion;
}

function formatBytes(bytes) {
  const value = Number(bytes) || 0;
  if (value >= 1024 * 1024) {
    return `${(value / 1024 / 1024).toFixed(2)} MB`;
  }
  if (value >= 1024) {
    return `${(value / 1024).toFixed(1)} KB`;
  }
  return `${value} B`;
}

function bytesToHex(bytes) {
  return Array.from(bytes, (byte) => byte.toString(16).padStart(2, "0")).join(
    "",
  );
}

function bytesToBase64(bytes) {
  let binary = "";
  for (const byte of bytes) {
    binary += String.fromCharCode(byte);
  }
  return window.btoa(binary);
}

function withCurrentFlag(items) {
  const currentSymbol = currentMarketSymbol();
  return items.map((item) => ({
    ...item,
    current:
      Boolean(currentSymbol) &&
      normalizeMarketQuery(item.requestSymbol) === currentSymbol,
  }));
}

function marketKindLabel(kind) {
  return kind === "index" ? t("market.index") : t("market.stock");
}

function marketDisplayName(market) {
  return state.locale === "en" && market.displayNameEn
    ? market.displayNameEn
    : market.displayName;
}

function formatConfigNumber(value, fractionDigits = 1) {
  const number = Number(value);
  if (!Number.isFinite(number)) {
    return "";
  }
  const rounded = Number(number.toFixed(fractionDigits));
  return Number.isInteger(rounded) ? String(rounded) : String(rounded);
}

function applyRefreshInputs(settings = DEFAULT_REFRESH_SETTINGS) {
  elements.partialCleanIntervalInput.value = formatConfigNumber(
    settings.partialCleanInterval,
    0,
  );
}

function readNumberField(element, label, min, max) {
  const rawValue = String(element.value || "").trim();
  const value = Number(rawValue);
  if (!rawValue || !Number.isFinite(value)) {
    throw new Error(
      state.locale === "en" ? `Enter a valid ${label}` : `请输入有效的${label}`,
    );
  }
  if (value < min || value > max) {
    throw new Error(
      state.locale === "en"
        ? `${label} must be between ${min} and ${max}`
        : `${label}需要在 ${min} 到 ${max} 之间`,
    );
  }
  return value;
}

function readIntegerField(element, label, min, max) {
  const value = readNumberField(element, label, min, max);
  if (!Number.isInteger(value)) {
    throw new Error(
      state.locale === "en" ? `${label} must be an integer` : `${label}需要是整数`,
    );
  }
  return value;
}

function validateComfortSettings(settings) {
  const comfortTemperatureMin = Number(settings.comfortTemperatureMin);
  const comfortTemperatureMax = Number(settings.comfortTemperatureMax);
  const comfortHumidityMin = Number(settings.comfortHumidityMin);
  const comfortHumidityMax = Number(settings.comfortHumidityMax);
  if (
    !Number.isFinite(comfortTemperatureMin) ||
    !Number.isFinite(comfortTemperatureMax) ||
    !Number.isFinite(comfortHumidityMin) ||
    !Number.isFinite(comfortHumidityMax)
  ) {
    throw new Error(
      state.locale === "en"
        ? "Enter valid comfort thresholds"
        : "请输入有效的表情条件阈值",
    );
  }
  if (comfortTemperatureMin > comfortTemperatureMax) {
    throw new Error(
      state.locale === "en"
        ? "Minimum comfort temperature cannot be higher than maximum comfort temperature"
        : "最低舒适温度不能高于最高舒适温度",
    );
  }
  if (comfortHumidityMin > comfortHumidityMax) {
    throw new Error(
      state.locale === "en"
        ? "Minimum comfort humidity cannot be higher than maximum comfort humidity"
        : "最低舒适湿度不能高于最高舒适湿度",
    );
  }

  return {
    comfortTemperatureMin,
    comfortTemperatureMax,
    comfortHumidityMin,
    comfortHumidityMax,
  };
}

function readRefreshSettingsFromInputs() {
  return {
    partialCleanInterval: readIntegerField(
      elements.partialCleanIntervalInput,
      t("settings.partialCleanInterval"),
      REFRESH_LIMITS.partialCleanIntervalMin,
      REFRESH_LIMITS.partialCleanIntervalMax,
    ),
  };
}

function localMarketMatches(item, query) {
  const normalizedQuery = normalizeMarketQuery(query);
  return (
    !normalizedQuery ||
    normalizeMarketQuery(item.code).startsWith(normalizedQuery) ||
    normalizeMarketQuery(item.requestSymbol).startsWith(normalizedQuery)
  );
}

function normalizeMarketSearchKey(value) {
  const normalized = normalizeMarketQuery(value);
  if (/^(sh|sz)\d+$/.test(normalized)) {
    return normalized.slice(2);
  }
  return normalized;
}

function dedupeMarkets(items) {
  const seen = new Set();
  return items.filter((item) => {
    const key = normalizeMarketQuery(item.requestSymbol || item.code || "");
    if (!key || seen.has(key)) {
      return false;
    }
    seen.add(key);
    return true;
  });
}

async function fetchMarketSearchResults(query) {
  const searchKey = normalizeMarketSearchKey(query);
  if (!searchKey) {
    return [];
  }

  const url = new URL(MARKET_SEARCH_API_PATH, window.location.origin);
  url.searchParams.set("q", searchKey);

  const response = await fetch(url.toString(), {
    cache: "no-store",
  });
  if (!response.ok) {
    throw new Error(
      state.locale === "en"
        ? `Market search failed (HTTP ${response.status})`
        : `行情搜索失败（HTTP ${response.status}）`,
    );
  }

  const data = await response.json();
  if (!data?.ok) {
    throw new Error(
      data?.error || (state.locale === "en" ? "Market search failed" : "行情搜索失败"),
    );
  }

  return Array.isArray(data.results)
    ? data.results.map((item) => ({
        requestSymbol: String(item.requestSymbol || ""),
        code: String(item.code || ""),
        displayName: String(item.displayName || item.code || ""),
        kind: String(item.kind || "stock"),
      }))
    : [];
}

function appendLog(message, level = "info") {
  const timestamp = new Date().toLocaleTimeString(state.locale, {
    hour12: false,
  });
  const prefix =
    level === "error"
      ? `[${t("log.error")}]`
      : level === "device"
        ? `[${t("log.device")}]`
        : `[${t("log.info")}]`;
  const nextLine = `${timestamp} ${prefix} ${message}`;
  elements.logOutput.textContent = elements.logOutput.textContent
    ? `${elements.logOutput.textContent}\n${nextLine}`
    : nextLine;
  elements.logOutput.scrollTop = elements.logOutput.scrollHeight;
}

function setMessage(message, isError = false) {
  elements.deviceMessage.textContent = message;
  elements.deviceMessage.classList.toggle("error", isError);
}

function setConnected(connected) {
  state.connected = connected;
  const connectionLabel =
    state.connectionType === "bluetooth"
      ? t("status.bluetooth")
      : state.serialBaudRate
        ? `${t("status.usb")} ${state.serialBaudRate}`
        : t("status.usb");
  elements.transportIcon.className = connected
    ? `transport-icon ${state.connectionType === "bluetooth" ? "bluetooth" : "serial"}`
    : "transport-icon";
  elements.connectionStateLabel.textContent = connected
    ? t("status.connected", { transport: connectionLabel })
    : t("status.disconnected");
  elements.connectionState.classList.toggle("offline", !connected);

  const allowDeviceActions = connected && state.busyCount === 0;
  elements.connectButton.disabled = !state.supportsSerial || state.busyCount > 0;
  elements.bluetoothButton.disabled =
    !state.supportsBluetooth || state.busyCount > 0;
  elements.reconnectButton.disabled =
    !state.supportsSerial || state.busyCount > 0;
  elements.disconnectButton.disabled = !connected;
  elements.statusButton.disabled = !allowDeviceActions;
  elements.scanButton.disabled = !allowDeviceActions;
  elements.saveButton.disabled = !allowDeviceActions;
  elements.applyButton.disabled = !allowDeviceActions;
  elements.syncButton.disabled = !allowDeviceActions;
  elements.refreshButton.disabled = !allowDeviceActions;
  elements.rebootButton.disabled = !allowDeviceActions;
  const editingClassicLayout = activeLayoutKind() === "classic";
  elements.layoutPresetSelect.disabled = false;
  elements.layoutNewButton.disabled = editingClassicLayout;
  elements.layoutDuplicateButton.disabled = editingClassicLayout;
  elements.layoutDeleteButton.disabled =
    editingClassicLayout || isBuiltInLayoutId(state.activeLayoutId);
  elements.layoutSaveButton.disabled =
    !allowDeviceActions || !state.layoutDirty;
  elements.layoutResetButton.disabled =
    !allowDeviceActions;
  elements.checkUpdateButton.disabled = state.busyCount > 0;
  elements.otaUpdateButton.disabled =
    !allowDeviceActions ||
    !state.otaManifest?.ota?.url ||
    !state.otaUpdateAvailable;
  elements.localOtaUploadButton.disabled =
    !allowDeviceActions ||
    state.connectionType !== "serial" ||
    !state.localOtaFile;
  renderLocalOtaSummary();
}

function setBusy(active) {
  state.busyCount += active ? 1 : -1;
  state.busyCount = Math.max(0, state.busyCount);
  setConnected(state.connected);
}

function renderTimezoneOptions() {
  const fragment = document.createDocumentFragment();
  for (let offset = -12; offset <= 14; offset += 1) {
    const option = document.createElement("option");
    option.value = String(offset);
    option.textContent = offset >= 0 ? `UTC+${offset}` : `UTC${offset}`;
    fragment.appendChild(option);
  }
  elements.timezoneSelect.innerHTML = "";
  elements.timezoneSelect.appendChild(fragment);
}

function renderNetworks() {
  elements.networkList.innerHTML = "";

  if (!state.networks.length) {
    const empty = document.createElement("p");
    empty.className = "empty-state";
    empty.textContent = state.connected
      ? t("wifi.emptyConnected")
      : t("wifi.emptyDisconnected");
    elements.networkList.appendChild(empty);
    return;
  }

  for (const network of state.networks) {
    const button = document.createElement("button");
    button.type = "button";
    button.className = "network-item";
    if (network.ssid === state.selectedSsid) {
      button.classList.add("selected");
    }
    button.innerHTML = `<strong>${escapeHtml(network.ssid)}</strong><span>${formatSignalLabel(
      network.rssi,
    )}</span>`;
    button.addEventListener("click", () => {
      state.selectedSsid = network.ssid;
      elements.ssidInput.value = network.ssid;
      renderNetworks();
    });
    elements.networkList.appendChild(button);
  }
}

function renderDashboardLayoutEditor() {
  if (!elements.layoutPreview || !elements.layoutComponentList) {
    return;
  }
  renderLayoutPresetControls();
  renderLayoutEditorStatus();

  elements.layoutPreview.innerHTML = "";
  const previewCanvas = createDashboardPreviewCanvas();
  if (activeLayoutKind() === "classic") {
    renderClassicPreviewCanvas(previewCanvas.ctx);
    elements.layoutPreview.appendChild(previewCanvas.canvas);
    elements.layoutComponentList.innerHTML = "";
    const empty = document.createElement("p");
    empty.className = "empty-state";
    empty.textContent = localized(
      "经典数字布局使用固定位置，不需要位置编辑。",
      "Classic digits use a fixed layout and do not need position editing.",
    );
    elements.layoutComponentList.appendChild(empty);
    return;
  }

  renderDashboardPreviewCanvas(previewCanvas.ctx);
  elements.layoutPreview.appendChild(previewCanvas.canvas);

  for (const x of state.layoutGuides.vertical) {
    const guide = document.createElement("span");
    guide.className = "layout-guide layout-guide-vertical";
    guide.style.left = `${(x / DASHBOARD_PREVIEW.width) * 100}%`;
    elements.layoutPreview.appendChild(guide);
  }
  for (const y of state.layoutGuides.horizontal) {
    const guide = document.createElement("span");
    guide.className = "layout-guide layout-guide-horizontal";
    guide.style.top = `${(y / DASHBOARD_PREVIEW.height) * 100}%`;
    elements.layoutPreview.appendChild(guide);
  }

  const visibleLayoutItems = state.dashboardLayout.filter(
    (layoutItem) => layoutItem.visible !== false,
  );
  for (const item of visibleLayoutItems) {
    const node = document.createElement("button");
    node.type = "button";
    node.className = `layout-component layout-component-${item.type}`;
    if (item.id === state.selectedLayoutId) {
      node.classList.add("is-selected");
    }
    node.dataset.id = item.id;
    node.tabIndex = 0;
    node.style.left = `${(item.x / DASHBOARD_PREVIEW.width) * 100}%`;
    node.style.top = `${(item.y / DASHBOARD_PREVIEW.height) * 100}%`;
    node.style.width = `${(item.w / DASHBOARD_PREVIEW.width) * 100}%`;
    node.style.height = `${(item.h / DASHBOARD_PREVIEW.height) * 100}%`;
    node.setAttribute(
      "aria-label",
      `${t(item.labelKey)} ${item.x}, ${item.y}`,
    );
    node.addEventListener("click", () => {
      selectDashboardLayoutItem(item.id);
      focusSelectedLayoutComponent();
    });
    node.addEventListener("contextmenu", (event) => {
      event.preventDefault();
      selectDashboardLayoutItem(item.id);
      showLayoutContextMenu(item.id, event.clientX, event.clientY);
    });
    node.addEventListener("keydown", (event) => {
      const moves = {
        ArrowLeft: [-1, 0],
        ArrowRight: [1, 0],
        ArrowUp: [0, -1],
        ArrowDown: [0, 1],
      };
      const move = moves[event.key];
      if (!move) {
        return;
      }
      const step = event.shiftKey ? DASHBOARD_PREVIEW.snap * 5 : DASHBOARD_PREVIEW.snap;
      nudgeDashboardLayoutItem(item.id, move[0] * step, move[1] * step);
      event.preventDefault();
    });
    node.addEventListener("pointerdown", (event) => {
      const point = dashboardPreviewPoint(event);
      state.selectedLayoutId = item.id;
      state.draggedLayoutId = item.id;
      state.layoutDragOffset = {
        x: point.x - item.x,
        y: point.y - item.y,
      };
      state.layoutGuides = { vertical: [], horizontal: [] };
      node.setPointerCapture?.(event.pointerId);
      node.focus({ preventScroll: true });
      event.preventDefault();
    });
    elements.layoutPreview.appendChild(node);
  }

  elements.layoutComponentList.innerHTML = "";
  renderDashboardComponentLibrary();
  const selectedItem =
    state.dashboardLayout.find(
      (item) => item.id === state.selectedLayoutId && item.visible !== false,
    ) ||
    state.dashboardLayout.find((item) => item.visible !== false);
  if (!selectedItem) {
    const empty = document.createElement("p");
    empty.className = "empty-state";
    empty.textContent = t("layout.noVisibleComponents");
    elements.layoutComponentList.appendChild(empty);
    return;
  }
  state.selectedLayoutId = selectedItem.id;

  const row = document.createElement("div");
  row.className = "layout-position-row is-selected";
  row.dataset.id = selectedItem.id;
  row.innerHTML = `
    <div class="layout-position-section-title">${escapeHtml(t("layout.position"))}</div>
    <div class="layout-position-heading">
      <strong>${escapeHtml(t(selectedItem.labelKey))}</strong>
      <span>${escapeHtml(selectedItem.id)}</span>
    </div>
    <div class="layout-position-fields">
      <label><span>X</span><input type="number" data-axis="x" data-id="${escapeHtml(selectedItem.id)}" min="0" max="${DASHBOARD_PREVIEW.width - selectedItem.w}" step="${DASHBOARD_PREVIEW.snap}" value="${selectedItem.x}" /></label>
      <label><span>Y</span><input type="number" data-axis="y" data-id="${escapeHtml(selectedItem.id)}" min="0" max="${DASHBOARD_PREVIEW.height - selectedItem.h}" step="${DASHBOARD_PREVIEW.snap}" value="${selectedItem.y}" /></label>
    </div>
    <p class="layout-position-hint">${escapeHtml(t("layout.keyboardHint"))}</p>
  `;
  row.querySelectorAll("input").forEach((input) => {
    input.addEventListener("change", () => {
      updateDashboardLayoutItem(input.dataset.id, {
        [input.dataset.axis]: Number(input.value),
      });
    });
  });
  elements.layoutComponentList.appendChild(row);
  elements.layoutComponentList.appendChild(renderComponentPropertiesPanel(selectedItem));
}

function showLayoutContextMenu(id, clientX, clientY) {
  closeLayoutContextMenu();
  const menu = document.createElement("div");
  menu.className = "layout-context-menu";
  renderLayoutContextMenuActions(menu);
  menu.style.left = `${clientX}px`;
  menu.style.top = `${clientY}px`;
  menu.addEventListener("pointerdown", (event) => {
    event.stopPropagation();
  });
  menu.addEventListener("click", (event) => {
    const action = event.target?.dataset?.action;
    event.stopPropagation();
    if (action === "reset") {
      renderLayoutContextMenuConfirm(menu, id, "reset");
    } else if (action === "remove") {
      renderLayoutContextMenuConfirm(menu, id, "remove");
    } else if (action === "confirm-reset") {
      resetDashboardLayoutItem(id);
      closeLayoutContextMenu();
    } else if (action === "confirm-remove") {
      removeDashboardLayoutItem(id);
      closeLayoutContextMenu();
    } else if (action === "cancel") {
      closeLayoutContextMenu();
    }
  });
  document.body.appendChild(menu);

  const rect = menu.getBoundingClientRect();
  const left = Math.min(clientX, window.innerWidth - rect.width - 8);
  const top = Math.min(clientY, window.innerHeight - rect.height - 8);
  menu.style.left = `${Math.max(8, left)}px`;
  menu.style.top = `${Math.max(8, top)}px`;

  window.setTimeout(() => {
    document.addEventListener("pointerdown", closeLayoutContextMenu, { once: true });
    document.addEventListener("keydown", handleLayoutContextMenuKeydown, { once: true });
  }, 0);
}

function renderLayoutContextMenuActions(menu) {
  menu.innerHTML = `
    <button type="button" data-action="reset">${escapeHtml(t("layout.contextReset"))}</button>
    <button type="button" data-action="remove">${escapeHtml(t("layout.contextRemove"))}</button>
  `;
}

function renderLayoutContextMenuConfirm(menu, id, action) {
  const selectedItem = state.dashboardLayout.find((item) => item.id === id);
  const label = selectedItem ? t(selectedItem.labelKey) : id;
  const message = action === "reset"
    ? t("layout.contextConfirmReset")
    : t("layout.contextConfirmRemove");
  menu.innerHTML = `
    <div class="layout-context-confirm">
      <strong>${escapeHtml(label)}</strong>
      <span>${escapeHtml(message)}</span>
    </div>
    <div class="layout-context-confirm-actions">
      <button type="button" data-action="cancel">${escapeHtml(t("layout.contextCancel"))}</button>
      <button type="button" data-action="confirm-${escapeHtml(action)}">${escapeHtml(t("layout.contextConfirm"))}</button>
    </div>
  `;
}

function closeLayoutContextMenu() {
  document.querySelector(".layout-context-menu")?.remove();
}

function handleLayoutContextMenuKeydown(event) {
  if (event.key === "Escape") {
    closeLayoutContextMenu();
  }
}

function renderLayoutEditorStatus() {
  if (!elements.layoutEditorStatus) {
    return;
  }
  const activeLayout =
    state.dashboardLayouts.find((layout) => layout.id === state.activeLayoutId) ||
    defaultDashboardLayoutPreset();
  const visibleCount = state.dashboardLayout.filter(
    (item) => item.visible !== false,
  ).length;
  elements.layoutEditorStatus.textContent = t("layout.status", {
    name: activeLayout.name,
    visible: visibleCount,
    total: state.dashboardLayout.length,
    dirty: state.layoutDirty ? t("layout.unsaved") : "",
  });
  elements.layoutEditorStatus.classList.toggle("is-dirty", state.layoutDirty);
}

function renderLayoutPresetControls() {
  if (!elements.layoutPresetSelect) {
    return;
  }
  elements.layoutPresetSelect.innerHTML = "";
  const layouts = state.dashboardLayouts.length
    ? state.dashboardLayouts
    : [defaultClassicLayoutPreset(), defaultDashboardLayoutPreset()];
  for (const layout of layouts) {
    const option = document.createElement("option");
    option.value = layout.id;
    option.textContent = layout.name;
    elements.layoutPresetSelect.appendChild(option);
  }
  elements.layoutPresetSelect.value = state.activeLayoutId;
  elements.layoutDeleteButton.disabled =
    activeLayoutKind() === "classic" ||
    isBuiltInLayoutId(state.activeLayoutId) ||
    state.busyCount > 0;
}

function renderDashboardComponentLibrary() {
  const panel = document.createElement("div");
  panel.className = "layout-component-library";
  const visibleItems = state.dashboardLayout.filter((item) => item.visible !== false);
  const hiddenItems = state.dashboardLayout.filter((item) => item.visible === false);
  panel.innerHTML = `
    <div class="layout-library-group">
      <p class="layout-library-title">${escapeHtml(t("layout.visibleComponents"))}</p>
      <div class="layout-chip-list" data-list="visible"></div>
    </div>
    <div class="layout-library-group">
      <p class="layout-library-title">${escapeHtml(t("layout.hiddenComponents"))}</p>
      <div class="layout-chip-list" data-list="hidden"></div>
    </div>
  `;

  const visibleList = panel.querySelector('[data-list="visible"]');
  const hiddenList = panel.querySelector('[data-list="hidden"]');
  if (visibleItems.length) {
    visibleItems.forEach((item) => {
      const button = document.createElement("button");
      button.type = "button";
      button.className = "layout-chip";
      if (item.id === state.selectedLayoutId) {
        button.classList.add("is-selected");
      }
      button.textContent = t(item.labelKey);
      button.addEventListener("click", () => selectDashboardLayoutItem(item.id));
      visibleList.appendChild(button);
    });
  } else {
    const empty = document.createElement("span");
    empty.className = "layout-chip-empty";
    empty.textContent = t("layout.noVisibleComponents");
    visibleList.appendChild(empty);
  }

  if (hiddenItems.length) {
    hiddenItems.forEach((item) => {
      const button = document.createElement("button");
      button.type = "button";
      button.className = "layout-chip layout-chip-add";
      button.textContent = `+ ${t(item.labelKey)}`;
      button.addEventListener("click", () => addDashboardLayoutItem(item.id));
      hiddenList.appendChild(button);
    });
  } else {
    const empty = document.createElement("span");
    empty.className = "layout-chip-empty";
    empty.textContent = t("layout.noHiddenComponents");
    hiddenList.appendChild(empty);
  }

  elements.layoutComponentList.appendChild(panel);
}

function dashboardPreviewPoint(event) {
  const rect = elements.layoutPreview.getBoundingClientRect();
  return {
    x: ((event.clientX - rect.left) / rect.width) * DASHBOARD_PREVIEW.width,
    y: ((event.clientY - rect.top) / rect.height) * DASHBOARD_PREVIEW.height,
  };
}

function updateDashboardLayoutItem(id, patch) {
  state.selectedLayoutId = id;
  const nextLayout = state.dashboardLayout.map((item) =>
    item.id === id ? clampDashboardLayoutItem({ ...item, ...patch }) : item,
  );
  setDashboardLayout(nextLayout, { dirty: true });
}

function updateDashboardLayoutItemProps(id, patch) {
  state.selectedLayoutId = id;
  const nextLayout = state.dashboardLayout.map((item) =>
    item.id === id
      ? {
          ...item,
          props: {
            ...(item.props || {}),
            ...patch,
          },
        }
      : item,
  );
  setDashboardLayout(nextLayout, { dirty: true });
}

function selectDashboardLayoutItem(id) {
  const item = state.dashboardLayout.find(
    (layoutItem) => layoutItem.id === id && layoutItem.visible !== false,
  );
  if (!item || state.selectedLayoutId === id) {
    return;
  }
  state.selectedLayoutId = id;
  renderDashboardLayoutEditor();
}

function focusSelectedLayoutComponent() {
  if (!state.selectedLayoutId) {
    return;
  }
  window.requestAnimationFrame(() => {
    const selector = `.layout-component[data-id="${CSS.escape(state.selectedLayoutId)}"]`;
    elements.layoutPreview.querySelector(selector)?.focus({ preventScroll: true });
  });
}

function renderComponentPropertiesPanel(item) {
  const panel = document.createElement("div");
  panel.className = "layout-position-row layout-properties-row";
  if (item.type === "market") {
    panel.innerHTML = `
      <div class="layout-position-section-title">${escapeHtml(t("layout.properties"))}</div>
    `;
    panel.appendChild(renderMarketPropertiesPanel(item));
    return panel;
  }
  if (item.type === "climate" || item.type === "classic-info") {
    panel.innerHTML = `
      <div class="layout-position-section-title">${escapeHtml(t("layout.properties"))}</div>
    `;
    panel.appendChild(renderClimatePropertiesPanel(item));
    return panel;
  }

  const definition = COMPONENT_REGISTRY[item.type];
  const schema = definition?.propsSchema || [];
  const fields = schema.map((field) => {
    const value = item.props?.[field.key] ?? "";
    if (field.type === "text") {
      return `
        <label>
          <span>${escapeHtml(t(field.labelKey))}</span>
          <input
            type="text"
            data-prop="${escapeHtml(field.key)}"
            data-id="${escapeHtml(item.id)}"
            value="${escapeHtml(value)}"
            placeholder="${escapeHtml(t(field.placeholderKey))}"
          />
        </label>
      `;
    }
    return "";
  }).join("");

  panel.innerHTML = `
    <div class="layout-position-section-title">${escapeHtml(t("layout.properties"))}</div>
    ${
      fields
        ? `<div class="layout-property-fields">${fields}</div>`
        : `<p class="layout-position-hint">${escapeHtml(t("layout.noProperties"))}</p>`
    }
  `;
  panel.querySelectorAll("input").forEach((input) => {
    input.addEventListener("change", () => {
      updateDashboardLayoutItemProps(input.dataset.id, {
        [input.dataset.prop]: input.value.trim(),
      });
    });
  });
  return panel;
}

function comfortSettingsFromLayoutItem(item) {
  const props = item.props || {};
  return {
    comfortTemperatureMin:
      props.comfortTemperatureMin ??
      state.lastStatus?.comfortTemperatureMin ??
      DEFAULT_COMFORT_SETTINGS.comfortTemperatureMin,
    comfortTemperatureMax:
      props.comfortTemperatureMax ??
      state.lastStatus?.comfortTemperatureMax ??
      DEFAULT_COMFORT_SETTINGS.comfortTemperatureMax,
    comfortHumidityMin:
      props.comfortHumidityMin ??
      state.lastStatus?.comfortHumidityMin ??
      DEFAULT_COMFORT_SETTINGS.comfortHumidityMin,
    comfortHumidityMax:
      props.comfortHumidityMax ??
      state.lastStatus?.comfortHumidityMax ??
      DEFAULT_COMFORT_SETTINGS.comfortHumidityMax,
  };
}

function readComfortSettingsFromPanel(panel) {
  const settings = {};
  panel.querySelectorAll("[data-comfort-prop]").forEach((input) => {
    settings[input.dataset.comfortProp] = readNumberField(
      input,
      input.previousElementSibling?.textContent || "",
      Number(input.min),
      Number(input.max),
    );
  });
  return validateComfortSettings(settings);
}

function applyComfortSettingsToPanel(panel, settings) {
  panel.querySelectorAll("[data-comfort-prop]").forEach((input) => {
    const value = settings[input.dataset.comfortProp];
    const fractionDigits = input.step === "1" ? 0 : 1;
    input.value = formatConfigNumber(value, fractionDigits);
  });
}

function renderClimatePropertiesPanel(item) {
  const panel = document.createElement("div");
  panel.className = "climate-properties-panel";
  const settings = comfortSettingsFromLayoutItem(item);
  panel.innerHTML = `
    <p class="helper-text market-helper">${t("comfort.helper")}</p>
    <div class="field-grid comfort-grid">
      <label class="field">
        <span>${escapeHtml(t("comfort.tempMin"))}</span>
        <input data-comfort-prop="comfortTemperatureMin" data-id="${escapeHtml(item.id)}" type="number" inputmode="decimal" min="-20" max="60" step="0.5" value="${escapeHtml(formatConfigNumber(settings.comfortTemperatureMin, 1))}" />
      </label>
      <label class="field">
        <span>${escapeHtml(t("comfort.tempMax"))}</span>
        <input data-comfort-prop="comfortTemperatureMax" data-id="${escapeHtml(item.id)}" type="number" inputmode="decimal" min="-20" max="60" step="0.5" value="${escapeHtml(formatConfigNumber(settings.comfortTemperatureMax, 1))}" />
      </label>
      <label class="field">
        <span>${escapeHtml(t("comfort.humidityMin"))}</span>
        <input data-comfort-prop="comfortHumidityMin" data-id="${escapeHtml(item.id)}" type="number" inputmode="decimal" min="0" max="100" step="1" value="${escapeHtml(formatConfigNumber(settings.comfortHumidityMin, 0))}" />
      </label>
      <label class="field">
        <span>${escapeHtml(t("comfort.humidityMax"))}</span>
        <input data-comfort-prop="comfortHumidityMax" data-id="${escapeHtml(item.id)}" type="number" inputmode="decimal" min="0" max="100" step="1" value="${escapeHtml(formatConfigNumber(settings.comfortHumidityMax, 0))}" />
      </label>
    </div>
    <div class="climate-property-actions">
      <button class="action-button primary" type="button" data-comfort-save="true">
        <span>${escapeHtml(t("comfort.save"))}</span>
      </button>
      <button class="action-button subtle" type="button" data-comfort-reset="true">
        <span>${escapeHtml(t("comfort.restoreDefaults"))}</span>
      </button>
    </div>
  `;
  panel.querySelector("[data-comfort-save]")?.addEventListener("click", () => {
    try {
      updateDashboardLayoutItemProps(item.id, readComfortSettingsFromPanel(panel));
    } catch (error) {
      handleActionError(error);
    }
  });
  panel.querySelector("[data-comfort-reset]")?.addEventListener("click", () => {
    applyComfortSettingsToPanel(panel, DEFAULT_COMFORT_SETTINGS);
    updateDashboardLayoutItemProps(item.id, DEFAULT_COMFORT_SETTINGS);
  });
  panel.querySelectorAll("[data-comfort-prop]").forEach((input) => {
    input.addEventListener("keydown", (event) => {
      if (event.key === "Enter") {
        event.preventDefault();
        panel.querySelector("[data-comfort-save]")?.click();
      }
    });
  });
  return panel;
}

function renderMarketPropertiesPanel(item) {
  const panel = document.createElement("div");
  panel.className = "market-properties-panel";
  const currentSymbol = normalizeMarketQuery(item.props?.symbol || currentMarketSymbol());
  const displayName = state.lastStatus?.marketDisplayName || t("market.defaultName");
  const displayCode =
    state.lastStatus?.marketCode ||
    currentSymbol.replace(/^(sh|sz)/, "") ||
    "000001";
  panel.innerHTML = `
    <div class="market-current">
      <span class="status-label">${escapeHtml(t("market.current"))}</span>
      <strong>${escapeHtml(displayName)}</strong>
      <span class="market-code">${escapeHtml(displayCode)}</span>
    </div>
    <div class="layout-property-fields">
      <label>
        <span>${escapeHtml(t("layout.prop.symbol"))}</span>
        <input
          type="text"
          data-prop="symbol"
          data-id="${escapeHtml(item.id)}"
          value="${escapeHtml(currentSymbol)}"
          placeholder="${escapeHtml(t("layout.prop.symbolPlaceholder"))}"
        />
      </label>
    </div>
    <div class="market-search-row">
      <label class="field market-search-field">
        <span>${escapeHtml(t("market.searchCode"))}</span>
        <input
          type="text"
          inputmode="search"
          value="${escapeHtml(state.marketSearchQuery)}"
          placeholder="${escapeHtml(t("market.searchPlaceholder"))}"
          data-market-search-input="true"
        />
      </label>
      <button class="action-button" type="button" data-market-search-button="true">
        <span>${escapeHtml(t("button.search"))}</span>
      </button>
    </div>
    <p class="helper-text market-helper">${escapeHtml(t("market.searchHelper"))}</p>
    <div class="market-picker-grid market-picker-grid-embedded">
      <div class="market-panel">
        <div class="market-subheading">
          <h3>${escapeHtml(t("market.hot"))}</h3>
        </div>
        <div class="market-hot-list" data-market-hot-list="true"></div>
      </div>
      <div class="market-panel">
        <div class="market-subheading">
          <h3>${escapeHtml(t("market.results"))}</h3>
          <span class="status-label" data-market-result-count="true"></span>
        </div>
        <div class="market-results" data-market-results="true" aria-live="polite"></div>
      </div>
    </div>
  `;

  const symbolInput = panel.querySelector("[data-prop='symbol']");
  symbolInput.addEventListener("change", () => {
    updateDashboardLayoutItemProps(symbolInput.dataset.id, {
      symbol: normalizeMarketQuery(symbolInput.value),
    });
  });

  const searchInput = panel.querySelector("[data-market-search-input]");
  const searchButton = panel.querySelector("[data-market-search-button]");
  searchButton.disabled = state.busyCount > 0;
  searchInput.addEventListener("input", () => {
    state.marketSearchQuery = searchInput.value;
  });
  searchInput.addEventListener("keydown", (event) => {
    if (event.key !== "Enter") {
      return;
    }
    event.preventDefault();
    void withBusyWork(() => searchMarkets(searchInput.value)).catch(handleActionError);
  });
  searchButton.addEventListener("click", () => {
    void withBusyWork(() => searchMarkets(searchInput.value)).catch(handleActionError);
  });

  renderMarketHotList(panel.querySelector("[data-market-hot-list]"));
  renderMarketResults(
    panel.querySelector("[data-market-results]"),
    panel.querySelector("[data-market-result-count]"),
  );
  return panel;
}

function isEditableTextTarget(target) {
  if (!(target instanceof Element)) {
    return false;
  }
  return Boolean(
    target.closest("input, textarea, select, button, [contenteditable='true']"),
  );
}

function handleLayoutKeyboardNudge(event) {
  if (
    activeLayoutKind() === "classic" ||
    !state.selectedLayoutId ||
    isEditableTextTarget(event.target)
  ) {
    return;
  }
  const moves = {
    ArrowLeft: [-1, 0],
    ArrowRight: [1, 0],
    ArrowUp: [0, -1],
    ArrowDown: [0, 1],
  };
  const move = moves[event.key];
  if (!move) {
    return;
  }
  const selectedItem = state.dashboardLayout.find(
    (item) => item.id === state.selectedLayoutId && item.visible !== false,
  );
  if (!selectedItem) {
    return;
  }
  const step = event.shiftKey ? DASHBOARD_PREVIEW.snap * 5 : DASHBOARD_PREVIEW.snap;
  nudgeDashboardLayoutItem(selectedItem.id, move[0] * step, move[1] * step);
  focusSelectedLayoutComponent();
  event.preventDefault();
}

function addDashboardLayoutItem(id) {
  const defaultItem = Object.values(COMPONENT_REGISTRY)
    .find((definition) => definition.defaultItem.id === id)
    ?.defaultItem;
  state.selectedLayoutId = id;
  const nextLayout = state.dashboardLayout.map((item) => {
    if (item.id !== id) {
      return item;
    }
    return clampDashboardLayoutItem({
      ...item,
      x: item.x ?? defaultItem?.x ?? 0,
      y: item.y ?? defaultItem?.y ?? 0,
      visible: true,
    });
  });
  setDashboardLayout(nextLayout, { dirty: true });
}

function removeDashboardLayoutItem(id) {
  const visibleItems = state.dashboardLayout.filter((item) => item.visible !== false);
  const nextSelected =
    visibleItems.find((item) => item.id !== id)?.id || null;
  state.selectedLayoutId = nextSelected;
  const nextLayout = state.dashboardLayout.map((item) =>
    item.id === id ? { ...item, visible: false } : item,
  );
  setDashboardLayout(nextLayout, { dirty: true });
}

function nudgeDashboardLayoutItem(id, dx, dy) {
  const currentItem = state.dashboardLayout.find(
    (item) => item.id === id && item.visible !== false,
  );
  if (!currentItem) {
    return;
  }
  updateDashboardLayoutItem(id, {
    x: currentItem.x + dx,
    y: currentItem.y + dy,
  });
}

function resetDashboardLayoutItem(id) {
  const defaultItem = Object.values(COMPONENT_REGISTRY)
    .find((definition) => definition.defaultItem.id === id)
    ?.defaultItem;
  if (!defaultItem) {
    return;
  }
  updateDashboardLayoutItem(id, {
    x: defaultItem.x,
    y: defaultItem.y,
  });
}

function moveDraggedDashboardComponent(event) {
  if (!state.draggedLayoutId) {
    return;
  }
  const point = dashboardPreviewPoint(event);
  const currentItem = state.dashboardLayout.find(
    (item) => item.id === state.draggedLayoutId && item.visible !== false,
  );
  if (!currentItem) {
    return;
  }
  const snapResult = applyDashboardGuideSnap({
    ...currentItem,
    x: point.x - state.layoutDragOffset.x,
    y: point.y - state.layoutDragOffset.y,
  });
  state.layoutGuides = snapResult.guides;
  updateDashboardLayoutItem(state.draggedLayoutId, snapResult.item);
}

function stopDashboardLayoutDrag() {
  if (state.draggedLayoutId) {
    state.layoutGuides = { vertical: [], horizontal: [] };
    renderDashboardLayoutEditor();
  }
  state.draggedLayoutId = null;
}

function renderMarketHotList(container) {
  if (!container) {
    return;
  }
  const markets = withCurrentFlag(DEFAULT_MARKETS);
  container.innerHTML = "";

  for (const kind of ["index", "stock"]) {
    const groupMarkets = markets.filter((market) => market.kind === kind);
    if (!groupMarkets.length) {
      continue;
    }

    const group = document.createElement("div");
    group.className = "market-hot-group";

    const title = document.createElement("p");
    title.className = "market-hot-title";
    title.textContent = t("market.hotGroup", { kind: marketKindLabel(kind) });
    group.appendChild(title);

    const list = document.createElement("div");
    list.className = "market-chip-list";
    for (const market of groupMarkets) {
      const button = document.createElement("button");
      button.type = "button";
      button.className = "market-chip";
      button.disabled = !state.connected || state.busyCount > 0;
      if (market.current) {
        button.classList.add("current");
      }
      button.innerHTML = `<strong>${escapeHtml(marketDisplayName(market))}</strong><span>${escapeHtml(
        market.code,
      )} · ${marketKindLabel(market.kind)}</span>`;
      button.addEventListener("click", () => {
        void selectMarket(market).catch(handleActionError);
      });
      list.appendChild(button);
    }
    group.appendChild(list);
    container.appendChild(group);
  }
}

function renderMarketResults(container, countElement) {
  if (!container || !countElement) {
    return;
  }
  container.innerHTML = "";
  if (!state.marketResultsVisible) {
    countElement.textContent = t("market.pending");
    const empty = document.createElement("p");
    empty.className = "empty-state";
    empty.textContent = t("market.emptyBeforeSearch");
    container.appendChild(empty);
    return;
  }

  const results = withCurrentFlag(state.marketResults);
  countElement.textContent = t("market.resultCount", {
    count: results.length,
  });

  if (!results.length) {
    const empty = document.createElement("p");
    empty.className = "empty-state";
    empty.textContent = state.connected
      ? t("market.emptyConnected")
      : t("market.emptyDisconnected");
    container.appendChild(empty);
    return;
  }

  for (const market of results) {
    const button = document.createElement("button");
    button.type = "button";
    button.className = "market-result";
    button.disabled = !state.connected || state.busyCount > 0;
    if (market.current) {
      button.classList.add("current");
    }
    button.innerHTML = `
      <span class="market-result-copy">
        <span class="market-result-name">${escapeHtml(marketDisplayName(market))}</span>
        <span class="market-result-meta">${escapeHtml(market.code)} · ${escapeHtml(
      market.requestSymbol,
    )}</span>
      </span>
      <span class="market-kind">${market.current ? t("market.currentTag") : marketKindLabel(
        market.kind,
      )}</span>
    `;
    button.addEventListener("click", () => {
      void selectMarket(market).catch(handleActionError);
    });
    container.appendChild(button);
  }
}

function formatSignalLabel(rssi) {
  if (typeof rssi !== "number") {
    return t("market.signalUnknown");
  }
  if (rssi >= -55) {
    return `${rssi} dBm · ${t("market.signalStrong")}`;
  }
  if (rssi >= -67) {
    return `${rssi} dBm · ${t("market.signalGood")}`;
  }
  if (rssi >= -75) {
    return `${rssi} dBm · ${t("market.signalFair")}`;
  }
  return `${rssi} dBm · ${t("market.signalWeak")}`;
}

function escapeHtml(value) {
  return String(value)
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;");
}

function updateStatus(status) {
  if (!status) {
    return;
  }

  state.lastStatus = status;
  elements.deviceName.textContent = status.deviceName || "M5Paper Ink Clock";
  const firmwareSha = status.firmwareBuildSha
    ? String(status.firmwareBuildSha).slice(0, 7)
    : "";
  elements.firmwareVersion.textContent = status.firmwareVersion
    ? `${status.firmwareVersion}${firmwareSha ? ` · ${firmwareSha}` : ""}`
    : "-";
  elements.pageName.textContent = status.page || "-";
  elements.wifiName.textContent = status.wifiConnected
    ? status.currentSsid || status.savedSsid || "-"
    : status.savedSsid || t("status.disconnected");
  elements.ipAddress.textContent = status.ipAddress || "-";
  elements.timezoneLabel.textContent =
    typeof status.timezone === "number"
      ? status.timezone >= 0
        ? `UTC+${status.timezone}`
        : `UTC${status.timezone}`
      : "-";
  const statusLayoutId = String(status.activeLayoutId || "");
  const statusLayout =
    state.dashboardLayouts.find((layout) => layout.id === statusLayoutId) ||
    activeLayoutFromDocument(status.layoutDocument);
  elements.layoutLabel.textContent =
    statusLayout?.name || statusLayoutId || "-";
  const marketDisplayName = status.marketDisplayName || t("market.defaultName");
  const marketCode = status.marketCode || "000001";
  elements.marketLabel.textContent = `${marketDisplayName} · ${marketCode}`;
  elements.batteryLabel.textContent =
    typeof status.batteryPercent === "number"
      ? `${status.batteryPercent}%`
      : "-";
  elements.syncState.textContent = status.timeSynced
    ? t("status.synced")
    : t("status.unsynced");
  elements.rtcLabel.textContent = status.rtc || "-";
  elements.savedSsid.textContent = status.savedSsid || "-";

  if (
    !elements.ssidInput.value ||
    elements.ssidInput.value === state.selectedSsid ||
    !state.selectedSsid
  ) {
    elements.ssidInput.value = status.savedSsid || elements.ssidInput.value;
  }

  if (typeof status.timezone === "number") {
    elements.timezoneSelect.value = String(status.timezone);
  }
  applyRefreshInputs({
    partialCleanInterval:
      status.partialCleanInterval ??
      DEFAULT_REFRESH_SETTINGS.partialCleanInterval,
  });
  if (!state.layoutDirty) {
    const activeLayout = activeLayoutFromDocument(status.layoutDocument);
    if (activeLayout?.components) {
      const normalized = normalizeDashboardLayoutPreset(activeLayout);
      const existingIndex = state.dashboardLayouts.findIndex(
        (layout) => layout.id === normalized.id,
      );
      if (existingIndex >= 0) {
        state.dashboardLayouts[existingIndex] = normalized;
      } else {
        state.dashboardLayouts = [...state.dashboardLayouts, normalized];
      }
      state.activeLayoutId = normalized.id;
      state.dashboardLayout = cloneDashboardLayout(normalized.components);
      state.layoutDirty = false;
      saveLayoutPresets();
      renderDashboardLayoutEditor();
      setConnected(state.connected);
    } else if (Array.isArray(status.dashboardLayout)) {
      setDashboardLayout(status.dashboardLayout, { dirty: false });
    }
  }

  const message =
    status.statusMessage ||
    t("status.connected", {
      transport:
        state.connectionType === "bluetooth" ? t("status.bluetooth") : t("status.usb"),
    });
  setMessage(message, Boolean(status.statusError));
  renderMarketHotList();
  renderMarketResults();
}

async function writeJson(payload) {
  const line = `${JSON.stringify(payload)}\n`;
  const bytes = state.encoder.encode(line);

  if (state.connectionType === "serial") {
    if (!state.writer) {
      throw new Error(localized("USB 串口未连接", "USB serial is not connected"));
    }
    await state.writer.write(bytes);
  } else if (state.connectionType === "bluetooth") {
    if (!state.bluetoothRxCharacteristic) {
      throw new Error(localized("蓝牙未连接", "Bluetooth is not connected"));
    }
    for (let offset = 0; offset < bytes.length; offset += BLE_WRITE_CHUNK_SIZE) {
      const chunk = bytes.slice(offset, offset + BLE_WRITE_CHUNK_SIZE);
      if (state.bluetoothRxCharacteristic.writeValueWithResponse) {
        await state.bluetoothRxCharacteristic.writeValueWithResponse(chunk);
      } else {
        await state.bluetoothRxCharacteristic.writeValue(chunk);
      }
    }
  } else {
    throw new Error(localized("设备尚未连接", "Device is not connected"));
  }

  if (payload.cmd !== "local_ota_chunk") {
    appendLog(localized(`发送 ${line.trim()}`, `Send ${line.trim()}`));
  }
}

async function sendCommand(cmd, data = undefined, timeoutMs = REQUEST_TIMEOUT_MS) {
  if (!state.connected) {
    throw new Error(localized("设备尚未连接", "Device is not connected"));
  }

  const id = state.requestId++;
  const payload = { id, cmd };
  if (state.connectionType === "bluetooth" && state.bleAuthToken) {
    payload.auth = state.bleAuthToken;
  }
  if (data !== undefined) {
    payload.data = data;
  }

  return new Promise(async (resolve, reject) => {
    const timeoutId = window.setTimeout(() => {
      state.pendingRequests.delete(id);
      reject(new Error(localized(`${cmd} 超时`, `${cmd} timed out`)));
    }, timeoutMs);

    state.pendingRequests.set(id, {
      resolve: (value) => {
        window.clearTimeout(timeoutId);
        resolve(value);
      },
      reject: (error) => {
        window.clearTimeout(timeoutId);
        reject(error);
      },
    });

    try {
      await writeJson(payload);
    } catch (error) {
      state.pendingRequests.delete(id);
      window.clearTimeout(timeoutId);
      reject(error);
    }
  });
}

function handleResponse(packet) {
  if (packet.event === "ota_progress" && packet.data) {
    state.otaProgress = {
      stage: String(packet.data.stage || "downloading"),
      written: Number(packet.data.written || 0),
      total: Number(packet.data.size || 0),
    };
    renderOtaProgress();
  }

  if (packet.event === "local_ota_progress" && packet.data) {
    renderLocalOtaSummary({
      written: Number(packet.data.written || 0),
      total: Number(packet.data.size || 0),
    });
  }

  if (
    packet.data &&
    Object.prototype.hasOwnProperty.call(packet.data, "written") &&
    Object.prototype.hasOwnProperty.call(packet.data, "size")
  ) {
    const written = Number(packet.data.written || 0);
    const size = Number(packet.data.size || 0);
    if (
      packet.event === "local_ota_progress" &&
      (written >= size ||
        written - state.localOtaLoggedWritten >= LOCAL_OTA_CHUNK_SIZE * 20)
    ) {
      state.localOtaLoggedWritten = written;
      appendLog(
        localized(
          `接收 local_ota_chunk ${written}/${size}`,
          `Receive local_ota_chunk ${written}/${size}`,
        ),
        "device",
      );
    }
  } else {
    appendLog(
      localized(`接收 ${JSON.stringify(packet)}`, `Receive ${JSON.stringify(packet)}`),
      "device",
    );
  }
  const pending = state.pendingRequests.get(packet.id);
  if (pending) {
    state.pendingRequests.delete(packet.id);
  }

  const payload = packet.data || {};
  if (payload.status && typeof payload.status === "object") {
    updateStatus(payload.status);
  } else if (
    Object.prototype.hasOwnProperty.call(payload, "deviceName") ||
    Object.prototype.hasOwnProperty.call(payload, "page")
  ) {
    updateStatus(payload);
  }

  if (Array.isArray(payload.networks)) {
    state.networks = payload.networks
      .filter((entry) => entry && entry.ssid)
      .map((entry) => ({
        ssid: String(entry.ssid),
        rssi: Number(entry.rssi),
      }));
    renderNetworks();
  }

  if (!packet.ok) {
    const error = new Error(
      packet.error || localized("设备返回失败", "Device returned an error"),
    );
    error.data = payload;
    error.packet = packet;
    if (packet.error === "Bluetooth pairing required") {
      window.localStorage.removeItem(BLE_AUTH_STORAGE_KEY);
      state.bleAuthToken = "";
    }
    setMessage(error.message, true);
    pending?.reject(error);
    return;
  }

  pending?.resolve(payload);
}

function rejectPending(error) {
  for (const pending of state.pendingRequests.values()) {
    pending.reject(error);
  }
  state.pendingRequests.clear();
}

function processIncomingText(text) {
  state.readBuffer += text;
  let newlineIndex = state.readBuffer.indexOf("\n");
  while (newlineIndex >= 0) {
    const rawLine = state.readBuffer.slice(0, newlineIndex);
    state.readBuffer = state.readBuffer.slice(newlineIndex + 1);
    handleLine(rawLine.replace(/\r$/, "").trim());
    newlineIndex = state.readBuffer.indexOf("\n");
  }
}

async function readLoop() {
  while (state.port?.readable) {
    state.reader = state.port.readable.getReader();
    try {
      while (true) {
        const { value, done } = await state.reader.read();
        if (done) {
          break;
        }
        if (!value) {
          continue;
        }
        processIncomingText(state.decoder.decode(value, { stream: true }));
      }
    } finally {
      state.reader.releaseLock();
      state.reader = null;
    }
    break;
  }
}

function handleLine(line) {
  if (!line) {
    return;
  }

  if (!line.startsWith(CFG_PREFIX)) {
    appendLog(line, "device");
    return;
  }

  try {
    const packet = JSON.parse(line.slice(CFG_PREFIX.length));
    handleResponse(packet);
  } catch (error) {
    appendLog(localized(`无法解析设备返回: ${error.message}`, `Could not parse device response: ${error.message}`), "error");
  }
}

async function closePort() {
  try {
    if (state.reader) {
      await state.reader.cancel();
    }
  } catch (error) {
    appendLog(localized(`关闭读取器时出现提示: ${error.message}`, `Reader close warning: ${error.message}`), "error");
  }

  try {
    if (state.writer) {
      state.writer.releaseLock();
      state.writer = null;
    }
  } catch (error) {
    appendLog(localized(`释放写入器时出现提示: ${error.message}`, `Writer release warning: ${error.message}`), "error");
  }

  try {
    if (state.port) {
      await state.port.close();
    }
  } catch (error) {
    appendLog(localized(`关闭串口时出现提示: ${error.message}`, `Serial close warning: ${error.message}`), "error");
  }

  try {
    if (state.bluetoothTxCharacteristic) {
      state.bluetoothTxCharacteristic.removeEventListener(
        "characteristicvaluechanged",
        handleBluetoothNotification,
      );
      await state.bluetoothTxCharacteristic.stopNotifications();
    }
  } catch (error) {
    appendLog(localized(`停止蓝牙通知时出现提示: ${error.message}`, `Bluetooth notification stop warning: ${error.message}`), "error");
  }

  try {
    if (state.bluetoothDevice) {
      state.bluetoothDevice.removeEventListener(
        "gattserverdisconnected",
        handleBluetoothDisconnected,
      );
    }
    if (state.bluetoothDevice?.gatt?.connected) {
      state.bluetoothDevice.gatt.disconnect();
    }
  } catch (error) {
    appendLog(localized(`断开蓝牙时出现提示: ${error.message}`, `Bluetooth disconnect warning: ${error.message}`), "error");
  }

  state.port = null;
  state.bluetoothDevice = null;
  state.bluetoothServer = null;
  state.bluetoothRxCharacteristic = null;
  state.bluetoothTxCharacteristic = null;
  state.connectionType = null;
  state.serialBaudRate = null;
  state.readBuffer = "";
  setConnected(false);
  stopPolling();
  rejectPending(new Error(localized("设备已断开", "Device disconnected")));
  renderMarketHotList();
  renderMarketResults();
}

async function openPortAtBaud(port, baudRate) {
  if (!port) {
    throw new Error(localized("没有可用的串口", "No available serial port"));
  }

  await closePort();
  state.port = port;
  await state.port.open({
    baudRate,
    dataBits: 8,
    stopBits: 1,
    parity: "none",
    flowControl: "none",
  });
  state.writer = state.port.writable.getWriter();
  state.decoder = new TextDecoder();
  state.readBuffer = "";
  state.connectionType = "serial";
  state.serialBaudRate = baudRate;
  setConnected(true);
  appendLog(localized(`串口已连接 ${baudRate}`, `Serial connected at ${baudRate}`));
  setMessage(localized(`USB 已连接 ${baudRate}，正在读取状态。`, `USB connected at ${baudRate}. Reading status.`));
  void readLoop().catch((error) => {
    appendLog(localized(`读取串口失败: ${error.message}`, `Serial read failed: ${error.message}`), "error");
    setMessage(localized(`读取串口失败：${error.message}`, `Serial read failed: ${error.message}`), true);
    void closePort();
  });
  await state.writer.write(state.encoder.encode("\n"));
  await new Promise((resolve) => window.setTimeout(resolve, 120));
  const data = await sendCommand(
    "get_status",
    undefined,
    SERIAL_HANDSHAKE_TIMEOUT_MS,
  );
  updateStatus(data);
  startPolling();
  await searchMarkets("", { silent: true });
}

async function openPort(port) {
  let lastError = null;
  for (const baudRate of SERIAL_BAUD_RATES) {
    try {
      await openPortAtBaud(port, baudRate);
      return;
    } catch (error) {
      lastError = error;
      appendLog(localized(`串口 ${baudRate} 握手失败: ${error.message}`, `Serial ${baudRate} handshake failed: ${error.message}`), "error");
      await closePort();
    }
  }
  throw lastError || new Error(localized("串口连接失败", "Serial connection failed"));
}

async function requestPortAndConnect() {
  const port = await navigator.serial.requestPort();
  await openPort(port);
}

async function reconnectAuthorizedPort() {
  const ports = await navigator.serial.getPorts();
  if (!ports.length) {
    throw new Error(localized("还没有已授权设备", "No authorized device yet"));
  }
  await openPort(ports[0]);
}

async function autoConnectAuthorizedPort() {
  const ports = await navigator.serial.getPorts();
  if (!ports.length) {
    setMessage(localized("请先点“连接 USB”授权串口，或点“连接蓝牙”。", "Click Connect USB to authorize a serial port, or click Connect Bluetooth."));
    return;
  }

  appendLog(localized(`发现 ${ports.length} 个已授权设备，正在自动连接`, `Found ${ports.length} authorized devices. Auto-connecting.`));
  setMessage(localized("发现已授权设备，正在自动连接。", "Found an authorized device. Auto-connecting."));

  try {
    await withBusyWork(async () => {
      await openPort(ports[0]);
    });
  } catch (error) {
    appendLog(localized(`自动连接失败: ${error.message}`, `Auto-connect failed: ${error.message}`), "error");
    setMessage(localized("自动连接失败，请点“恢复已授权设备”重试。", "Auto-connect failed. Click Reconnect USB to retry."), true);
  }
}

function handleBluetoothNotification(event) {
  const value = event.target?.value;
  if (!value) {
    return;
  }
  processIncomingText(state.decoder.decode(value, { stream: true }));
}

async function openBluetoothDevice(device) {
  if (!device?.gatt) {
    throw new Error(localized("没有可用的蓝牙设备", "No available Bluetooth device"));
  }

  await closePort();
  state.bluetoothDevice = device;
  state.bluetoothDevice.addEventListener(
    "gattserverdisconnected",
    handleBluetoothDisconnected,
  );

  setMessage(localized("正在连接蓝牙设备。", "Connecting Bluetooth device."));
  const server = await state.bluetoothDevice.gatt.connect();
  const service = await server.getPrimaryService(BLE_SERVICE_UUID);
  const rxCharacteristic = await service.getCharacteristic(
    BLE_RX_CHARACTERISTIC_UUID,
  );
  const txCharacteristic = await service.getCharacteristic(
    BLE_TX_CHARACTERISTIC_UUID,
  );
  await txCharacteristic.startNotifications();
  txCharacteristic.addEventListener(
    "characteristicvaluechanged",
    handleBluetoothNotification,
  );

  state.bluetoothServer = server;
  state.bluetoothRxCharacteristic = rxCharacteristic;
  state.bluetoothTxCharacteristic = txCharacteristic;
  state.decoder = new TextDecoder();
  state.readBuffer = "";
  state.connectionType = "bluetooth";
  setConnected(true);
  appendLog(localized(`蓝牙已连接 ${device.name || BLE_DEVICE_NAME_PREFIX}`, `Bluetooth connected ${device.name || BLE_DEVICE_NAME_PREFIX}`));
  setMessage(localized("蓝牙已连接，正在校验配对。", "Bluetooth connected. Verifying pairing."));
  try {
    await ensureBluetoothPaired();
    startPolling();
    await refreshStatus({ silent: true });
    await searchMarkets("", { silent: true });
  } catch (error) {
    await closePort();
    throw error;
  }
}

async function requestBluetoothAndConnect() {
  if (!("bluetooth" in navigator)) {
    throw new Error(localized("当前浏览器不支持 Web Bluetooth", "This browser does not support Web Bluetooth"));
  }

  const device = await navigator.bluetooth.requestDevice({
    filters: [
      { services: [BLE_SERVICE_UUID] },
      { namePrefix: BLE_DEVICE_NAME_PREFIX },
    ],
    optionalServices: [BLE_SERVICE_UUID],
  });
  await openBluetoothDevice(device);
}

async function handleBluetoothDisconnected(event) {
  if (event.target !== state.bluetoothDevice) {
    return;
  }

  appendLog(localized("蓝牙已断开", "Bluetooth disconnected"), "error");
  setMessage(localized("设备蓝牙已断开。", "Device Bluetooth disconnected."), true);
  state.bluetoothDevice = null;
  state.bluetoothServer = null;
  state.bluetoothRxCharacteristic = null;
  state.bluetoothTxCharacteristic = null;
  state.connectionType = null;
  state.serialBaudRate = null;
  state.readBuffer = "";
  setConnected(false);
  stopPolling();
  rejectPending(new Error(localized("蓝牙已断开", "Bluetooth disconnected")));
}

function startPolling() {
  stopPolling();
  state.pollTimer = window.setInterval(() => {
    if (!state.connected || state.busyCount > 0) {
      return;
    }
    void refreshStatus({ silent: true });
  }, POLL_INTERVAL_MS);
}

function stopPolling() {
  if (state.pollTimer) {
    window.clearInterval(state.pollTimer);
    state.pollTimer = null;
  }
}

async function withBusyWork(fn) {
  setBusy(true);
  try {
    await fn();
  } finally {
    setBusy(false);
  }
}

async function refreshStatus(options = {}) {
  const { silent = false } = options;
  if (!silent) {
    setMessage(localized("正在读取设备状态。", "Reading device status."));
  }
  const data = await sendCommand("get_status");
  updateStatus(data);
}

async function ensureBluetoothPaired() {
  if (state.connectionType !== "bluetooth") {
    return;
  }

  if (state.bleAuthToken) {
    try {
      await refreshStatus({ silent: true });
      if (state.lastStatus?.bluetoothAuthorized) {
        return;
      }
    } catch (error) {
      appendLog(localized(`蓝牙授权校验失败: ${error.message}`, `Bluetooth authorization check failed: ${error.message}`), "error");
    }
    window.localStorage.removeItem(BLE_AUTH_STORAGE_KEY);
    state.bleAuthToken = "";
    appendLog(localized("本地蓝牙授权已失效，重新配对", "Local Bluetooth authorization expired. Pairing again."), "error");
  }

  setMessage(localized("请查看 M5Paper 顶栏显示的 4 位蓝牙配对码。", "Check the 4-digit Bluetooth pairing code shown in the M5Paper top bar."));
  await sendCommand("pair_begin", undefined, 8000);
  const code = window.prompt(localized("请输入 M5Paper 屏幕上显示的 4 位蓝牙配对码", "Enter the 4-digit Bluetooth pairing code shown on the M5Paper screen"));
  if (code === null) {
    throw new Error(localized("已取消蓝牙配对", "Bluetooth pairing canceled"));
  }

  const normalizedCode = String(code).replace(/\D/g, "").slice(0, 4);
  if (normalizedCode.length !== 4) {
    throw new Error(localized("请输入 4 位数字配对码", "Enter a 4-digit numeric pairing code"));
  }

  const data = await sendCommand("pair_verify", { code: normalizedCode }, 15000);
  if (!data.token) {
    throw new Error(localized("设备没有返回蓝牙授权 token", "Device did not return a Bluetooth authorization token"));
  }

  state.bleAuthToken = String(data.token);
  window.localStorage.setItem(BLE_AUTH_STORAGE_KEY, state.bleAuthToken);
  if (data.status) {
    updateStatus(data.status);
  }
  setMessage(localized("蓝牙配对成功。", "Bluetooth pairing succeeded."));
}

async function scanWifi() {
  setMessage(localized("正在扫描 Wi-Fi，请稍候。", "Scanning Wi-Fi. Please wait."));
  const data = await sendCommand("scan_wifi", undefined, 25000);
  const networks = Array.isArray(data.networks) ? data.networks.length : 0;
  setMessage(
    networks
      ? localized(`找到 ${networks} 个网络。`, `Found ${networks} networks.`)
      : localized("没有找到可用网络。", "No available networks found."),
  );
}

async function searchMarkets(queryOverride, options = {}) {
  const { silent = false } = options;
  const query = normalizeMarketQuery(
    queryOverride ?? state.marketSearchQuery,
  );
  state.marketSearchQuery = query;
  const localMatches = DEFAULT_MARKETS.filter((item) =>
    localMarketMatches(item, query),
  );

  if (!query) {
    state.marketResults = [];
    state.marketResultsVisible = false;
    renderDashboardLayoutEditor();
    if (!silent) {
      setMessage(
        state.connected
          ? localized(
              "可从左侧热门标的直接切换，或输入代码搜索。",
              "Choose a popular symbol on the left or search by code.",
            )
          : localized(
              "可查看左侧热门标的，连接设备后即可切换首页行情。",
              "Popular symbols are visible now. Connect the device to switch the home market card.",
            ),
      );
    }
    return;
  }

  state.marketResultsVisible = true;
  if (!silent) {
    setMessage(localized("正在搜索行情标的。", "Searching market symbols."));
  }

  try {
    const remoteMatches = await fetchMarketSearchResults(query);
    state.marketResults = dedupeMarkets([
      ...localMatches,
      ...remoteMatches,
    ]);
  } catch (error) {
    state.marketResults = dedupeMarkets(localMatches);
    renderDashboardLayoutEditor();
    if (state.marketResults.length) {
      if (!silent) {
        setMessage(
          localized(
            "搜索服务暂时不可用，先显示热门匹配标的。",
            "Search service is temporarily unavailable. Showing matching popular symbols.",
          ),
        );
      }
      return;
    }
    throw error;
  }

  renderDashboardLayoutEditor();
  if (!silent) {
    setMessage(
      state.marketResults.length
        ? state.connected
          ? localized(
              `找到 ${state.marketResults.length} 个匹配标的。`,
              `Found ${state.marketResults.length} matching symbols.`,
            )
          : localized(
              `找到 ${state.marketResults.length} 个匹配标的，连接设备后即可切换。`,
              `Found ${state.marketResults.length} matching symbols. Connect the device to switch.`,
            )
        : t("market.emptyConnected"),
    );
  }
}

async function selectMarket(market) {
  if (!state.connected) {
    throw new Error(localized("请先连接设备，再切换行情标的", "Connect the device before switching the market symbol"));
  }

  const requestSymbol = String(market.requestSymbol || "");
  const displayName = String(market.displayName || "");
  if (!requestSymbol) {
    throw new Error(localized("缺少标的代码", "Missing market symbol code"));
  }
  if (!updateMarketSymbolInActiveLayout(requestSymbol)) {
    throw new Error(localized("当前布局没有可配置的行情组件", "The current layout has no configurable market component"));
  }

  setMessage(
    localized(
      `正在保存行情组件为 ${displayName || requestSymbol}。`,
      `Saving the market component as ${displayName || requestSymbol}.`,
    ),
  );
  const data = await sendCommand(
    "apply_layout",
    {
      layoutDocument: layoutDocumentFromState(),
    },
    20000,
  );
  state.layoutDirty = false;
  updateStatus(data);
  state.marketResults = withCurrentFlag(state.marketResults);
  renderDashboardLayoutEditor();
  setMessage(
    localized(
      `行情组件已保存为 ${displayName || requestSymbol}。`,
      `Market component saved as ${displayName || requestSymbol}.`,
    ),
  );
}

async function saveSettings({ connectNow, syncTime }) {
  const typedSsid = elements.ssidInput.value.trim();
  const savedSsid = String(state.lastStatus?.savedSsid || "").trim();
  const ssid = typedSsid || savedSsid;
  const password = elements.passwordInput.value;
  const timezone = Number(elements.timezoneSelect.value);
  const refreshSettings = readRefreshSettingsFromInputs();

  if (!ssid) {
    throw new Error(localized("请先输入或选择 Wi-Fi 名称", "Enter or choose a Wi-Fi network name first"));
  }

  setMessage(
    connectNow
      ? localized("正在保存并连接 Wi-Fi。", "Saving and connecting Wi-Fi.")
      : localized("正在保存设置。", "Saving settings."),
  );

  const payload = {
    timezone,
    connectNow,
    syncTime,
    ...refreshSettings,
  };

  if (!savedSsid || typedSsid !== savedSsid) {
    payload.ssid = ssid;
    payload.password = password;
  } else if (password) {
    payload.password = password;
  }

  const data = await sendCommand(
    "apply_settings",
    payload,
    connectNow ? 30000 : REQUEST_TIMEOUT_MS,
  );
  updateStatus(data);
  elements.passwordInput.value = "";
  elements.passwordInput.type = "password";
  elements.togglePasswordButton.textContent = t("button.showPassword");
  setMessage(
    connectNow
      ? localized(
          "设置已保存，设备正在尝试连接 Wi-Fi 并同步时间。",
          "Settings saved. The device is connecting Wi-Fi and syncing time.",
        )
      : localized("设置已保存。", "Settings saved."),
  );
}

async function saveDashboardLayout() {
  if (!state.connected) {
    throw new Error(localized("请先连接设备，再保存布局", "Connect the device before saving the layout"));
  }
  setMessage(localized("正在保存布局。", "Saving layout."));
  const localLayoutDocument = layoutDocumentFromState();
  const localActiveLayoutId = localLayoutDocument.activeLayoutId;
  const data = await sendCommand(
    "apply_layout",
    {
      layoutDocument: localLayoutDocument,
    },
    REQUEST_TIMEOUT_MS,
  );
  updateStatus(data);
  state.dashboardLayouts = localLayoutDocument.layouts.map((layout) =>
    normalizeDashboardLayoutPreset(layout),
  );
  state.activeLayoutId = localActiveLayoutId;
  const activeLayout =
    state.dashboardLayouts.find((layout) => layout.id === state.activeLayoutId) ||
    state.dashboardLayouts[0];
  state.dashboardLayout = cloneDashboardLayout(activeLayout.components);
  state.layoutDirty = false;
  saveLayoutPresets();
  renderDashboardLayoutEditor();
  setConnected(state.connected);
  setMessage(localized("布局已保存。", "Layout saved."));
}

async function resetDashboardLayout() {
  if (!state.connected) {
    throw new Error(localized("请先连接设备，再恢复默认布局", "Connect the device before resetting the layout"));
  }
  setMessage(localized("正在恢复默认布局。", "Resetting layout."));
  const data = await sendCommand("apply_layout", { reset: true }, REQUEST_TIMEOUT_MS);
  state.layoutDirty = false;
  updateStatus(data);
  const activeLayout = activeLayoutFromDocument(data.layoutDocument);
  if (activeLayout?.components) {
    const normalized = normalizeDashboardLayoutPreset(activeLayout);
    const existingIndex = state.dashboardLayouts.findIndex(
      (layout) => layout.id === normalized.id,
    );
    if (existingIndex >= 0) {
      state.dashboardLayouts[existingIndex] = normalized;
    } else {
      state.dashboardLayouts = [...state.dashboardLayouts, normalized];
    }
    state.activeLayoutId = normalized.id;
    state.dashboardLayout = cloneDashboardLayout(normalized.components);
    saveLayoutPresets();
    renderDashboardLayoutEditor();
    setConnected(state.connected);
  }
  setMessage(localized("布局已恢复默认。", "Layout reset."));
}

async function syncTime() {
  setMessage(localized("正在同步时间。", "Syncing time."));
  const data = await sendCommand("sync_time", undefined, 20000);
  updateStatus(data);
  setMessage(localized("时间同步已完成。", "Time sync completed."));
}

function resolveReleaseAssetUrl(value, baseUrl) {
  if (!value) {
    return "";
  }
  return new URL(String(value), baseUrl).toString();
}

function githubReleaseApiUrlFromDownloadUrl(value) {
  const match = String(value || "").match(GITHUB_RELEASE_DOWNLOAD_RE);
  if (!match) {
    return "";
  }

  const [, owner, repo, tag] = match;
  const base = `https://api.github.com/repos/${owner}/${repo}/releases`;
  return tag ? `${base}/tags/${tag}` : `${base}/latest`;
}

function sha256FromGithubAsset(asset) {
  const digest = String(asset?.digest || "");
  return digest.startsWith("sha256:") ? digest.slice("sha256:".length) : "";
}

async function fetchGithubReleaseOtaManifest(apiUrl) {
  const response = await fetch(apiUrl, {
    cache: "no-store",
    headers: { Accept: "application/vnd.github+json" },
  });
  if (!response.ok) {
    throw new Error(
      localized(
        `GitHub Release 信息读取失败（HTTP ${response.status}）`,
        `Failed to read GitHub Release information (HTTP ${response.status})`,
      ),
    );
  }

  const release = await response.json();
  const firmwareAsset = release.assets?.find(
    (asset) => asset.name === "firmware.bin",
  );
  const sha256 = sha256FromGithubAsset(firmwareAsset);
  if (!firmwareAsset?.browser_download_url || !sha256) {
    throw new Error(
      localized(
        "GitHub Release 缺少 firmware.bin 或 sha256 digest",
        "GitHub Release is missing firmware.bin or a sha256 digest",
      ),
    );
  }

  return {
    version: release.tag_name || "",
    buildSha: release.target_commitish || "",
    buildTime: release.published_at || release.created_at || "",
    ota: {
      name: firmwareAsset.name,
      url: firmwareAsset.browser_download_url,
      sha256,
      size: firmwareAsset.size || 0,
    },
  };
}

async function fetchOtaManifest(manifestUrl) {
  const githubApiUrl = githubReleaseApiUrlFromDownloadUrl(manifestUrl);
  if (githubApiUrl) {
    return fetchGithubReleaseOtaManifest(githubApiUrl);
  }

  const response = await fetch(manifestUrl, { cache: "no-store" });
  if (!response.ok) {
    throw new Error(
      localized(
        `OTA 信息读取失败（HTTP ${response.status}）`,
        `Failed to read OTA information (HTTP ${response.status})`,
      ),
    );
  }
  return response.json();
}

function renderOtaSummary(manifest) {
  if (!manifest) {
    elements.otaSummary.textContent = t("update.otaSummaryDefault");
    return;
  }
  if (!manifest?.ota) {
    elements.otaSummary.textContent = t("update.noOtaInfo");
    return;
  }

  const currentVersion = state.lastStatus?.firmwareVersion || "-";
  const sizeMb = manifest.ota.size
    ? (Number(manifest.ota.size) / 1024 / 1024).toFixed(2)
    : "-";
  const availableVersion = manifest.version || "-";
  if (!isOtaUpdateAvailable(manifest)) {
    elements.otaSummary.textContent = t("update.latestSummary", {
      current: currentVersion,
      available: availableVersion,
      size: sizeMb,
    });
    return;
  }

  elements.otaSummary.textContent = t("update.availableSummary", {
    current: currentVersion,
    available: availableVersion,
    size: sizeMb,
  });
}

function renderOtaProgress(progress = state.otaProgress) {
  if (!elements.otaProgressBox) {
    return;
  }
  if (!progress) {
    elements.otaProgressBox.hidden = true;
    elements.otaProgressBar.value = 0;
    elements.otaProgressPercent.textContent = "0%";
    elements.otaProgressLabel.textContent = t("update.otaProgressPreparing");
    return;
  }

  const written = Math.max(0, Number(progress.written || 0));
  const total = Math.max(0, Number(progress.total || progress.size || 0));
  const percent =
    total > 0 ? Math.min(100, Math.floor((written / total) * 100)) : 0;
  const stage = String(progress.stage || "downloading");

  elements.otaProgressBox.hidden = false;
  elements.otaProgressBar.value = percent;
  elements.otaProgressPercent.textContent = `${percent}%`;
  if (stage === "installed") {
    elements.otaProgressLabel.textContent = t("update.otaProgressInstalled");
  } else if (stage === "verifying") {
    elements.otaProgressLabel.textContent = t("update.otaProgressVerifying");
  } else if (stage === "preparing") {
    elements.otaProgressLabel.textContent = t("update.otaProgressPreparing");
  } else {
    elements.otaProgressLabel.textContent = t("update.otaProgressDownloading", {
      percent,
      written: formatBytes(written),
      total: total > 0 ? formatBytes(total) : "-",
    });
  }
}

function renderLocalOtaSummary(progress = null) {
  const file = state.localOtaFile;
  elements.localOtaFileName.textContent = file
    ? `${file.name} · ${formatBytes(file.size)}`
    : t("update.noFile");
  if (!file) {
    elements.localOtaSummary.textContent = t("update.localOtaSummaryDefault");
    return;
  }

  const fileText = `${file.name} · ${formatBytes(file.size)}`;
  if (progress && progress.total > 0) {
    const percent = Math.floor((progress.written / progress.total) * 100);
    elements.localOtaSummary.textContent = t("update.localUploading", {
      file: fileText,
      percent,
      written: formatBytes(progress.written),
      total: formatBytes(progress.total),
    });
    return;
  }

  elements.localOtaSummary.textContent =
    state.connectionType === "serial"
      ? t("update.localReady", { file: fileText })
      : t("update.localNeedUsb", { file: fileText });
}

function syncWebFlashManifest() {
  const manifestUrl =
    elements.webFlashManifestInput.value.trim() ||
    DEFAULT_WEB_FLASH_MANIFEST_URL;
  elements.webFlashButton.setAttribute("manifest", manifestUrl);
}

async function checkForUpdate() {
  const manifestUrl =
    elements.otaManifestInput.value.trim() || DEFAULT_OTA_MANIFEST_URL;
  state.otaProgress = null;
  renderOtaProgress();
  setMessage(localized("正在读取 OTA 信息。", "Reading OTA information."));
  const manifest = await fetchOtaManifest(manifestUrl);
  if (!manifest?.ota?.url || !manifest?.ota?.sha256) {
    throw new Error(
      localized(
        "OTA 信息缺少 firmware URL 或 sha256",
        "OTA information is missing firmware URL or sha256",
      ),
    );
  }

  state.otaManifestUrl = manifestUrl;
  state.otaManifest = {
    ...manifest,
    ota: {
      ...manifest.ota,
      url: resolveReleaseAssetUrl(manifest.ota.url, manifestUrl),
    },
  };
  state.otaUpdateAvailable = isOtaUpdateAvailable(state.otaManifest);
  renderOtaSummary(state.otaManifest);
  setConnected(state.connected);
  setMessage(
    state.otaUpdateAvailable
      ? localized("OTA 信息已读取。", "OTA information loaded.")
      : localized("当前已是最新版本。", "Already on the latest version."),
  );
}

async function startOtaUpdate() {
  if (!state.otaManifest?.ota?.url) {
    await checkForUpdate();
  }
  if (!state.otaUpdateAvailable) {
    throw new Error(localized("当前已是最新版本，无需 OTA 更新", "Already on the latest version; OTA update is not needed"));
  }
  const ota = state.otaManifest?.ota;
  if (!ota?.url || !ota?.sha256) {
    throw new Error(t("update.noOtaInfo"));
  }

  setMessage(
    localized(
      "设备正在下载并安装 OTA 固件，请保持供电。",
      "The device is downloading and installing OTA firmware. Keep power connected.",
    ),
  );
  state.otaProgress = {
    stage: "preparing",
    written: 0,
    total: Number(ota.size || 0),
  };
  renderOtaProgress();
  const data = await sendCommand(
    "ota_update",
    {
      url: ota.url,
      sha256: ota.sha256,
    },
    180000,
  );
  if (data.status) {
    updateStatus(data.status);
  }
  state.otaProgress = {
    stage: "installed",
    written: Number(ota.size || 0),
    total: Number(ota.size || 0),
  };
  renderOtaProgress();
  setMessage(localized("OTA 已安装，设备即将重启。", "OTA installed. The device will reboot."));
  await closePort();
}

async function uploadLocalOtaChunk(offset, chunk) {
  let lastError = null;
  for (let attempt = 1; attempt <= LOCAL_OTA_CHUNK_RETRIES; attempt += 1) {
    try {
      return await sendCommand(
        "local_ota_chunk",
        {
          offset,
          data: bytesToBase64(chunk),
        },
        30000,
      );
    } catch (error) {
      lastError = error;
      const message = String(error.message || "");
      const data = error.data || {};
      if (
        message.includes("Local OTA offset mismatch") &&
        Number.isFinite(Number(data.written))
      ) {
        appendLog(
          localized(
            `本地 OTA offset 已恢复到 ${Number(data.written || 0)}`,
            `Local OTA offset recovered to ${Number(data.written || 0)}`,
          ),
          "device",
        );
        return data;
      }
      if (
        (message.includes("local_ota_chunk 超时") ||
          message.includes("Invalid local OTA chunk")) &&
        attempt < LOCAL_OTA_CHUNK_RETRIES
      ) {
        const status = await getLocalOtaStatus();
        if (status?.active && Number.isFinite(Number(status.written))) {
          const written = Number(status.written || 0);
          if (written !== offset) {
            appendLog(
              localized(
                `本地 OTA 进度已恢复到 ${written}`,
                `Local OTA progress recovered to ${written}`,
              ),
              "device",
            );
            return status;
          }
        } else if (!message.includes("Invalid JSON request")) {
          throw error;
        }
      } else if (!message.includes("Invalid JSON request")) {
        throw error;
      }
      const retryReason = message.includes("Invalid JSON request")
        ? localized("JSON 损坏", "JSON corrupted")
        : localized("响应异常", "unexpected response");
      appendLog(
        localized(
          `本地 OTA chunk ${offset} ${retryReason}，重试 ${attempt}/${LOCAL_OTA_CHUNK_RETRIES}`,
          `Local OTA chunk ${offset} ${retryReason}, retry ${attempt}/${LOCAL_OTA_CHUNK_RETRIES}`,
        ),
        "error",
      );
      await new Promise((resolve) =>
        window.setTimeout(resolve, LOCAL_OTA_RETRY_DELAY_MS),
      );
    }
  }
  throw lastError || new Error(localized("本地 OTA chunk 上传失败", "Local OTA chunk upload failed"));
}

async function getLocalOtaStatus() {
  try {
    return await sendCommand("local_ota_status", undefined, 5000);
  } catch (error) {
    appendLog(
      localized(
        `读取本地 OTA 进度失败: ${error.message}`,
        `Failed to read local OTA progress: ${error.message}`,
      ),
      "error",
    );
    return null;
  }
}

async function uploadLocalOtaBinary(firmware) {
  if (!state.writer) {
    throw new Error(localized("USB 串口未连接", "USB serial is not connected"));
  }

  let nextMilestone = 10;
  for (
    let offset = 0;
    offset < firmware.byteLength;
    offset += LOCAL_OTA_BINARY_CHUNK_SIZE
  ) {
    const nextOffset = Math.min(
      offset + LOCAL_OTA_BINARY_CHUNK_SIZE,
      firmware.byteLength,
    );

    await state.writer.write(firmware.subarray(offset, nextOffset));
    renderLocalOtaSummary({
      written: nextOffset,
      total: firmware.byteLength,
    });

    const percent = Math.floor((nextOffset / firmware.byteLength) * 100);
    if (percent >= nextMilestone || nextOffset >= firmware.byteLength) {
      setMessage(localized(`本地 OTA 上传 ${percent}%。`, `Local OTA upload ${percent}%.`));
      nextMilestone += 10;
    }

    await new Promise((resolve) =>
      window.setTimeout(resolve, LOCAL_OTA_BINARY_CHUNK_DELAY_MS),
    );
  }

  await new Promise((resolve) => window.setTimeout(resolve, 500));
}

async function startLocalOtaUpload() {
  if (state.connectionType !== "serial") {
    throw new Error(localized("本地 OTA 上传仅支持 USB 串口", "Local OTA upload only supports USB serial"));
  }
  if (!state.localOtaFile) {
    throw new Error(localized("请选择本地 OTA 固件 bin", "Choose a local OTA firmware bin file"));
  }

  const file = state.localOtaFile;
  if (!file.name.toLowerCase().endsWith(".bin")) {
    throw new Error(localized("请选择 .bin 固件文件", "Choose a .bin firmware file"));
  }
  if (!file.size) {
    throw new Error(localized("固件文件为空", "Firmware file is empty"));
  }

  setMessage(localized("正在计算本地固件 SHA256。", "Calculating local firmware SHA256."));
  renderLocalOtaSummary({ written: 0, total: file.size });
  const fileBuffer = await file.arrayBuffer();
  const digest = await window.crypto.subtle.digest("SHA-256", fileBuffer);
  const sha256 = bytesToHex(new Uint8Array(digest));
  const firmware = new Uint8Array(fileBuffer);

  setMessage(localized("正在准备设备接收本地 OTA 固件。", "Preparing the device to receive local OTA firmware."));
  state.localOtaLoggedWritten = 0;
  const beginData = await sendCommand(
    "local_ota_begin",
    {
      size: firmware.byteLength,
      sha256,
      mode: "json",
    },
    30000,
  );

  let offset = 0;
  try {
    if (beginData.mode === "binary") {
      await uploadLocalOtaBinary(firmware);
    } else {
      while (offset < firmware.byteLength) {
        const nextOffset = Math.min(
          offset + LOCAL_OTA_CHUNK_SIZE,
          firmware.byteLength,
        );
        const chunk = firmware.subarray(offset, nextOffset);
        const data = await uploadLocalOtaChunk(offset, chunk);
        offset = Number(data.written || nextOffset);
        renderLocalOtaSummary({ written: offset, total: firmware.byteLength });
        if (
          offset % (LOCAL_OTA_CHUNK_SIZE * 20) === 0 ||
          offset >= firmware.byteLength
        ) {
          const percent = Math.floor((offset / firmware.byteLength) * 100);
          setMessage(localized(`本地 OTA 上传 ${percent}%。`, `Local OTA upload ${percent}%.`));
        }
        await new Promise((resolve) =>
          window.setTimeout(resolve, LOCAL_OTA_CHUNK_DELAY_MS),
        );
      }
    }

    const data = await sendCommand("local_ota_end", undefined, 60000);
    renderLocalOtaSummary({
      written: firmware.byteLength,
      total: firmware.byteLength,
    });
    if (data.status) {
      updateStatus(data.status);
    }
    setMessage(localized("本地 OTA 已安装，设备即将重启。", "Local OTA installed. The device will reboot."));
    await closePort();
  } catch (error) {
    try {
      await sendCommand("local_ota_abort", undefined, 5000);
    } catch (abortError) {
      appendLog(localized(`本地 OTA 中止失败: ${abortError.message}`, `Local OTA abort failed: ${abortError.message}`), "error");
    }
    throw error;
  }
}

async function refreshScreen() {
  setMessage(localized("正在触发全量刷新。", "Triggering full refresh."));
  const data = await sendCommand("refresh_screen", undefined, 12000);
  updateStatus(data);
  setMessage(localized("设备已执行全量刷新。", "Device completed a full refresh."));
}

async function rebootDevice() {
  setMessage(localized("正在重启设备。", "Rebooting device."));
  await sendCommand("reboot", undefined, 5000);
  appendLog(localized("设备已收到重启请求", "Device received the reboot request"));
  setMessage(localized("设备正在重启，稍后点“恢复已授权设备”即可。", "Device is rebooting. Click Reconnect USB later."));
  await closePort();
}

function togglePasswordVisibility() {
  const nextType =
    elements.passwordInput.type === "password" ? "text" : "password";
  elements.passwordInput.type = nextType;
  elements.togglePasswordButton.textContent =
    nextType === "password" ? t("button.showPassword") : t("button.hidePassword");
}

function installEventHandlers() {
  elements.languageSelect?.addEventListener("change", () => {
    setLocale(elements.languageSelect.value);
  });

  elements.connectButton.addEventListener("click", () =>
    withBusyWork(async () => {
      await requestPortAndConnect();
    }).catch(handleActionError),
  );

  elements.bluetoothButton.addEventListener("click", () =>
    withBusyWork(async () => {
      await requestBluetoothAndConnect();
    }).catch(handleActionError),
  );

  elements.reconnectButton.addEventListener("click", () =>
    withBusyWork(async () => {
      await reconnectAuthorizedPort();
    }).catch(handleActionError),
  );

  elements.disconnectButton.addEventListener("click", () =>
    withBusyWork(async () => {
      await closePort();
      setMessage(localized("设备已断开。", "Device disconnected."));
    }).catch(handleActionError),
  );

  elements.statusButton.addEventListener("click", () =>
    withBusyWork(refreshStatus).catch(handleActionError),
  );

  elements.scanButton.addEventListener("click", () =>
    withBusyWork(scanWifi).catch(handleActionError),
  );

  elements.layoutPresetSelect.addEventListener("change", () => {
    setActiveDashboardLayout(elements.layoutPresetSelect.value, { dirty: true });
  });
  elements.layoutNewButton.addEventListener("click", () => {
    createBlankDashboardPreset();
  });
  elements.layoutDuplicateButton.addEventListener("click", () => {
    duplicateDashboardPreset();
  });
  elements.layoutDeleteButton.addEventListener("click", () => {
    deleteDashboardPreset();
  });

  elements.saveButton.addEventListener("click", () =>
    withBusyWork(() =>
      saveSettings({ connectNow: false, syncTime: false }),
    ).catch(handleActionError),
  );

  elements.applyButton.addEventListener("click", () =>
    withBusyWork(() =>
      saveSettings({ connectNow: true, syncTime: true }),
    ).catch(handleActionError),
  );

  elements.syncButton.addEventListener("click", () =>
    withBusyWork(syncTime).catch(handleActionError),
  );

  elements.layoutSaveButton.addEventListener("click", () =>
    withBusyWork(saveDashboardLayout).catch(handleActionError),
  );

  elements.layoutResetButton.addEventListener("click", () =>
    withBusyWork(resetDashboardLayout).catch(handleActionError),
  );

  window.addEventListener("pointermove", moveDraggedDashboardComponent);
  window.addEventListener("pointerup", stopDashboardLayoutDrag);
  window.addEventListener("keydown", handleLayoutKeyboardNudge);

  elements.checkUpdateButton.addEventListener("click", () =>
    withBusyWork(checkForUpdate).catch(handleActionError),
  );

  elements.otaUpdateButton.addEventListener("click", () =>
    withBusyWork(startOtaUpdate).catch(handleActionError),
  );

  elements.localOtaFileInput.addEventListener("change", () => {
    state.localOtaFile = elements.localOtaFileInput.files?.[0] || null;
    renderLocalOtaSummary();
    setConnected(state.connected);
  });

  elements.localOtaUploadButton.addEventListener("click", () =>
    withBusyWork(startLocalOtaUpload).catch(handleActionError),
  );

  elements.webFlashManifestInput.addEventListener(
    "input",
    syncWebFlashManifest,
  );
  elements.webFlashManifestInput.addEventListener(
    "change",
    syncWebFlashManifest,
  );

  elements.refreshButton.addEventListener("click", () =>
    withBusyWork(refreshScreen).catch(handleActionError),
  );

  elements.rebootButton.addEventListener("click", () =>
    withBusyWork(rebootDevice).catch(handleActionError),
  );

  elements.togglePasswordButton.addEventListener("click", togglePasswordVisibility);

  elements.clearLogButton.addEventListener("click", () => {
    elements.logOutput.textContent = "";
  });

  elements.ssidInput.addEventListener("input", () => {
    state.selectedSsid = elements.ssidInput.value.trim();
    renderNetworks();
  });

  navigator.serial?.addEventListener("disconnect", async (event) => {
    if (state.port && event.port === state.port) {
      appendLog(localized("串口已被系统断开", "Serial port was disconnected by the system"), "error");
      setMessage(localized("设备串口已断开。", "Device serial port disconnected."), true);
      await closePort();
    }
  });
}

function handleActionError(error) {
  appendLog(error.message, "error");
  setMessage(error.message, true);
}

async function initialize() {
  applyTranslations();
  renderTimezoneOptions();
  state.dashboardLayouts = [defaultClassicLayoutPreset(), defaultDashboardLayoutPreset()];
  loadLayoutPresets();
  elements.otaManifestInput.value = DEFAULT_OTA_MANIFEST_URL;
  elements.webFlashManifestInput.value = DEFAULT_WEB_FLASH_MANIFEST_URL;
  syncWebFlashManifest();
  renderNetworks();
  state.marketResults = [];
  state.marketResultsVisible = false;
  renderMarketHotList();
  renderMarketResults();
  renderDashboardLayoutEditor();
  void loadDashboardPreviewFont();
  void loadDashboardWifiBitmaps();
  installEventHandlers();
  setConnected(false);

  if (!window.isSecureContext) {
    elements.browserNote.textContent = t("browser.secureContextRequired");
    setMessage(
      localized(
        "当前不是安全上下文，无法使用 USB 或蓝牙连接。",
        "This is not a secure context, so USB and Bluetooth connection are unavailable.",
      ),
      true,
    );
    elements.connectButton.disabled = true;
    elements.reconnectButton.disabled = true;
    elements.bluetoothButton.disabled = true;
    return;
  }

  const supportsSerial = "serial" in navigator;
  const supportsBluetooth = "bluetooth" in navigator;
  state.supportsSerial = supportsSerial;
  state.supportsBluetooth = supportsBluetooth;
  setConnected(false);

  if (!supportsSerial && !supportsBluetooth) {
    elements.browserNote.textContent = t("browser.noDeviceSupport");
    setMessage(
      localized(
        "当前浏览器不支持 USB 或蓝牙连接。",
        "This browser does not support USB or Bluetooth connection.",
      ),
      true,
    );
    return;
  }

  if (!supportsSerial) {
    appendLog(localized("当前浏览器不支持 Web Serial", "Web Serial is not supported by this browser"));
  }
  if (!supportsBluetooth) {
    appendLog(localized("当前浏览器不支持 Web Bluetooth", "Web Bluetooth is not supported by this browser"));
  }

  appendLog(localized("页面已就绪", "Page is ready"));

  try {
    if (supportsSerial) {
      await autoConnectAuthorizedPort();
    } else {
      setMessage(localized("请点“连接蓝牙”选择 M5Paper Clock。", "Click Connect Bluetooth and choose M5Paper Clock."));
    }
  } catch (error) {
    handleActionError(error);
  }
}

void initialize();

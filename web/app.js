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
const COMFORT_LIMITS = Object.freeze({
  temperatureMin: -20,
  temperatureMax: 60,
  humidityMin: 0,
  humidityMax: 100,
});

const DEFAULT_LOCALE = "zh-CN";
const SUPPORTED_LOCALES = Object.freeze(["zh-CN", "en"]);

const I18N = Object.freeze({
  "zh-CN": {
    "app.title": "M5Paper Ink Clock 配置",
    "language.label": "语言",
    "intro.heading": "设备配置",
    "intro.lead": "通过 USB 或蓝牙连接设备，保存 Wi-Fi、时区和时钟样式。",
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
    "status.style": "样式",
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
    "settings.clockStyle": "时钟样式",
    "settings.saveSection": "配置保存",
    "settings.saveHelp": "仅保存不会联网；保存并连接会写入配置、连接 Wi-Fi，并同步时间。",
    "settings.deviceSection": "设备操作",
    "settings.dangerSection": "危险操作",
    "clockStyle.classic": "经典数字",
    "clockStyle.dashboard": "仪表盘",
    "comfort.heading": "表情条件",
    "comfort.helper": "舒适区间显示 <code>(^_^)</code>，超出范围显示 <code>(-^-)</code>。",
    "comfort.tempMin": "最低舒适温度",
    "comfort.tempMax": "最高舒适温度",
    "comfort.humidityMin": "最低舒适湿度",
    "comfort.humidityMax": "最高舒适湿度",
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
    "update.fullFlashWarning": "可能清除设备配置，包括 Wi-Fi、时区、时钟样式、行情标的和表情条件。",
    "update.fullFlashManifest": "完整刷写 Manifest 地址",
    "update.noOtaInfo": "没有可用的 OTA 固件信息。",
    "update.latestSummary": "当前 {current}，可用 {available}，已是最新版本，固件 {size} MB。",
    "update.availableSummary": "当前 {current}，可用 {available}，固件 {size} MB。",
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
  },
  en: {
    "app.title": "M5Paper Ink Clock Setup",
    "language.label": "Language",
    "intro.heading": "Device Setup",
    "intro.lead": "Connect over USB or Bluetooth to save Wi-Fi, timezone, and clock style settings.",
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
    "status.style": "Style",
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
    "settings.clockStyle": "Clock style",
    "settings.saveSection": "Save Settings",
    "settings.saveHelp": "Save only does not connect; save and connect stores settings, connects Wi-Fi, and syncs time.",
    "settings.deviceSection": "Device Actions",
    "settings.dangerSection": "Danger Zone",
    "clockStyle.classic": "Classic digits",
    "clockStyle.dashboard": "Dashboard",
    "comfort.heading": "Face Conditions",
    "comfort.helper": "The comfortable range shows <code>(^_^)</code>; out-of-range values show <code>(-^-)</code>.",
    "comfort.tempMin": "Minimum comfort temperature",
    "comfort.tempMax": "Maximum comfort temperature",
    "comfort.humidityMin": "Minimum comfort humidity",
    "comfort.humidityMax": "Maximum comfort humidity",
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
    "update.fullFlashWarning": "May erase device settings, including Wi-Fi, timezone, clock style, market symbol, and face conditions.",
    "update.fullFlashManifest": "Full flash manifest URL",
    "update.noOtaInfo": "No OTA firmware information is available.",
    "update.latestSummary": "Current {current}, available {available}, already up to date, firmware {size} MB.",
    "update.availableSummary": "Current {current}, available {available}, firmware {size} MB.",
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
  clockStyleLabel: document.querySelector("#clock-style-label"),
  marketLabel: document.querySelector("#market-label"),
  batteryLabel: document.querySelector("#battery-label"),
  syncState: document.querySelector("#sync-state"),
  rtcLabel: document.querySelector("#rtc-label"),
  deviceMessage: document.querySelector("#device-message"),
  savedSsid: document.querySelector("#saved-ssid"),
  ssidInput: document.querySelector("#ssid-input"),
  passwordInput: document.querySelector("#password-input"),
  timezoneSelect: document.querySelector("#timezone-select"),
  clockStyleSelect: document.querySelector("#clock-style-select"),
  comfortTempMinInput: document.querySelector("#comfort-temp-min-input"),
  comfortTempMaxInput: document.querySelector("#comfort-temp-max-input"),
  comfortHumidityMinInput: document.querySelector("#comfort-humidity-min-input"),
  comfortHumidityMaxInput: document.querySelector("#comfort-humidity-max-input"),
  otaManifestInput: document.querySelector("#ota-manifest-input"),
  webFlashManifestInput: document.querySelector("#web-flash-manifest-input"),
  webFlashButton: document.querySelector("#web-flash-button"),
  otaSummary: document.querySelector("#ota-summary"),
  localOtaSummary: document.querySelector("#local-ota-summary"),
  marketCurrentName: document.querySelector("#market-current-name"),
  marketCurrentCode: document.querySelector("#market-current-code"),
  marketSearchInput: document.querySelector("#market-search-input"),
  marketSearchButton: document.querySelector("#market-search-button"),
  marketHotList: document.querySelector("#market-hot-list"),
  marketResultCount: document.querySelector("#market-result-count"),
  marketResults: document.querySelector("#market-results"),
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
  localOtaFile: null,
  localOtaLoggedWritten: 0,
  marketResultsVisible: false,
  marketResults: [],
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
  renderMarketHotList();
  renderMarketResults();
  renderOtaSummary(state.otaManifest);
  renderLocalOtaSummary();
}

function normalizeMarketQuery(value) {
  return String(value || "")
    .replace(/\s+/g, "")
    .toLowerCase();
}

function currentMarketSymbol() {
  return normalizeMarketQuery(state.lastStatus?.marketSymbol || "");
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

function applyComfortInputs(settings = DEFAULT_COMFORT_SETTINGS) {
  elements.comfortTempMinInput.value = formatConfigNumber(
    settings.comfortTemperatureMin,
    1,
  );
  elements.comfortTempMaxInput.value = formatConfigNumber(
    settings.comfortTemperatureMax,
    1,
  );
  elements.comfortHumidityMinInput.value = formatConfigNumber(
    settings.comfortHumidityMin,
    0,
  );
  elements.comfortHumidityMaxInput.value = formatConfigNumber(
    settings.comfortHumidityMax,
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

function readComfortSettingsFromInputs() {
  const comfortTemperatureMin = readNumberField(
    elements.comfortTempMinInput,
    t("comfort.tempMin"),
    COMFORT_LIMITS.temperatureMin,
    COMFORT_LIMITS.temperatureMax,
  );
  const comfortTemperatureMax = readNumberField(
    elements.comfortTempMaxInput,
    t("comfort.tempMax"),
    COMFORT_LIMITS.temperatureMin,
    COMFORT_LIMITS.temperatureMax,
  );
  const comfortHumidityMin = readNumberField(
    elements.comfortHumidityMinInput,
    t("comfort.humidityMin"),
    COMFORT_LIMITS.humidityMin,
    COMFORT_LIMITS.humidityMax,
  );
  const comfortHumidityMax = readNumberField(
    elements.comfortHumidityMaxInput,
    t("comfort.humidityMax"),
    COMFORT_LIMITS.humidityMin,
    COMFORT_LIMITS.humidityMax,
  );

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
  elements.checkUpdateButton.disabled = state.busyCount > 0;
  elements.otaUpdateButton.disabled =
    !allowDeviceActions ||
    !state.otaManifest?.ota?.url ||
    !state.otaUpdateAvailable;
  elements.localOtaUploadButton.disabled =
    !allowDeviceActions ||
    state.connectionType !== "serial" ||
    !state.localOtaFile;
  elements.marketSearchButton.disabled = state.busyCount > 0;
  renderMarketHotList();
  renderMarketResults();
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

function renderMarketHotList() {
  const markets = withCurrentFlag(DEFAULT_MARKETS);
  elements.marketHotList.innerHTML = "";

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
    elements.marketHotList.appendChild(group);
  }
}

function renderMarketResults() {
  elements.marketResults.innerHTML = "";
  if (!state.marketResultsVisible) {
    elements.marketResultCount.textContent = t("market.pending");
    const empty = document.createElement("p");
    empty.className = "empty-state";
    empty.textContent = t("market.emptyBeforeSearch");
    elements.marketResults.appendChild(empty);
    return;
  }

  const results = withCurrentFlag(state.marketResults);
  elements.marketResultCount.textContent = t("market.resultCount", {
    count: results.length,
  });

  if (!results.length) {
    const empty = document.createElement("p");
    empty.className = "empty-state";
    empty.textContent = state.connected
      ? t("market.emptyConnected")
      : t("market.emptyDisconnected");
    elements.marketResults.appendChild(empty);
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
    elements.marketResults.appendChild(button);
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
  elements.clockStyleLabel.textContent =
    status.clockStyle === "dashboard"
      ? t("clockStyle.dashboard")
      : status.clockStyle === "classic"
        ? t("clockStyle.classic")
        : "-";
  const marketDisplayName = status.marketDisplayName || t("market.defaultName");
  const marketCode = status.marketCode || "000001";
  elements.marketLabel.textContent = `${marketDisplayName} · ${marketCode}`;
  elements.marketCurrentName.textContent = marketDisplayName;
  elements.marketCurrentCode.textContent = marketCode;
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
  if (status.clockStyle) {
    elements.clockStyleSelect.value = status.clockStyle;
  }
  applyComfortInputs({
    comfortTemperatureMin:
      status.comfortTemperatureMin ?? DEFAULT_COMFORT_SETTINGS.comfortTemperatureMin,
    comfortTemperatureMax:
      status.comfortTemperatureMax ?? DEFAULT_COMFORT_SETTINGS.comfortTemperatureMax,
    comfortHumidityMin:
      status.comfortHumidityMin ?? DEFAULT_COMFORT_SETTINGS.comfortHumidityMin,
    comfortHumidityMax:
      status.comfortHumidityMax ?? DEFAULT_COMFORT_SETTINGS.comfortHumidityMax,
  });

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
      written >= size ||
      written - state.localOtaLoggedWritten >= LOCAL_OTA_CHUNK_SIZE * 20
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
    queryOverride ?? elements.marketSearchInput.value,
  );
  elements.marketSearchInput.value = query;
  const localMatches = DEFAULT_MARKETS.filter((item) =>
    localMarketMatches(item, query),
  );

  if (!query) {
    state.marketResults = [];
    state.marketResultsVisible = false;
    renderMarketResults();
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
    renderMarketResults();
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

  renderMarketResults();
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

  setMessage(
    localized(
      `正在切换到 ${displayName || requestSymbol}。`,
      `Switching to ${displayName || requestSymbol}.`,
    ),
  );
  const data = await sendCommand(
    "set_market",
    {
      requestSymbol,
      displayName,
    },
    20000,
  );
  updateStatus({
    ...data,
    marketSymbol: requestSymbol,
    marketCode: String(market.code || data.marketCode || ""),
    marketDisplayName: displayName || String(data.marketDisplayName || ""),
  });
  state.marketResults = withCurrentFlag(state.marketResults);
  renderMarketResults();
  setMessage(
    localized(
      `首页行情已切换为 ${displayName || requestSymbol}。`,
      `Home market symbol switched to ${displayName || requestSymbol}.`,
    ),
  );
}

async function saveSettings({ connectNow, syncTime }) {
  const typedSsid = elements.ssidInput.value.trim();
  const savedSsid = String(state.lastStatus?.savedSsid || "").trim();
  const ssid = typedSsid || savedSsid;
  const password = elements.passwordInput.value;
  const timezone = Number(elements.timezoneSelect.value);
  const clockStyle = elements.clockStyleSelect.value;
  const comfortSettings = readComfortSettingsFromInputs();

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
    clockStyle,
    connectNow,
    syncTime,
    ...comfortSettings,
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

  elements.marketSearchButton.addEventListener("click", () =>
    withBusyWork(() => searchMarkets()).catch(handleActionError),
  );

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

  elements.marketSearchInput.addEventListener("keydown", (event) => {
    if (event.key !== "Enter") {
      return;
    }
    event.preventDefault();
    void withBusyWork(() => searchMarkets()).catch(handleActionError);
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
  applyComfortInputs(DEFAULT_COMFORT_SETTINGS);
  elements.otaManifestInput.value = DEFAULT_OTA_MANIFEST_URL;
  elements.webFlashManifestInput.value = DEFAULT_WEB_FLASH_MANIFEST_URL;
  syncWebFlashManifest();
  renderNetworks();
  state.marketResults = [];
  state.marketResultsVisible = false;
  renderMarketHotList();
  renderMarketResults();
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

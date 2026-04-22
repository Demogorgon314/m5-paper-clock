const CFG_PREFIX = "@cfg:";
const DEFAULT_BAUD_RATE = 115200;
const REQUEST_TIMEOUT_MS = 15000;
const POLL_INTERVAL_MS = 20000;
const MARKET_SEARCH_API_PATH = "/api/market-search";
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

const DEFAULT_MARKETS = [
  {
    requestSymbol: "sh000001",
    code: "000001",
    displayName: "上证指数",
    kind: "index",
  },
  {
    requestSymbol: "sz399001",
    code: "399001",
    displayName: "深证成指",
    kind: "index",
  },
  {
    requestSymbol: "sz399006",
    code: "399006",
    displayName: "创业板指",
    kind: "index",
  },
  {
    requestSymbol: "sh000300",
    code: "000300",
    displayName: "沪深300",
    kind: "index",
  },
  {
    requestSymbol: "sh000905",
    code: "000905",
    displayName: "中证500",
    kind: "index",
  },
  {
    requestSymbol: "sh000688",
    code: "000688",
    displayName: "科创50",
    kind: "index",
  },
  {
    requestSymbol: "sh600519",
    code: "600519",
    displayName: "贵州茅台",
    kind: "stock",
  },
  {
    requestSymbol: "sh601318",
    code: "601318",
    displayName: "中国平安",
    kind: "stock",
  },
  {
    requestSymbol: "sz000858",
    code: "000858",
    displayName: "五粮液",
    kind: "stock",
  },
  {
    requestSymbol: "sz300750",
    code: "300750",
    displayName: "宁德时代",
    kind: "stock",
  },
];

const elements = {
  connectButton: document.querySelector("#connect-button"),
  reconnectButton: document.querySelector("#reconnect-button"),
  disconnectButton: document.querySelector("#disconnect-button"),
  statusButton: document.querySelector("#status-button"),
  scanButton: document.querySelector("#scan-button"),
  saveButton: document.querySelector("#save-button"),
  applyButton: document.querySelector("#apply-button"),
  syncButton: document.querySelector("#sync-button"),
  refreshButton: document.querySelector("#refresh-button"),
  rebootButton: document.querySelector("#reboot-button"),
  togglePasswordButton: document.querySelector("#toggle-password-button"),
  clearLogButton: document.querySelector("#clear-log-button"),
  connectionState: document.querySelector("#connection-state"),
  browserNote: document.querySelector("#browser-note"),
  deviceName: document.querySelector("#device-name"),
  pageName: document.querySelector("#page-name"),
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
  port: null,
  reader: null,
  writer: null,
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
  lastStatus: null,
  marketResults: [],
};

function normalizeMarketQuery(value) {
  return String(value || "")
    .replace(/\s+/g, "")
    .toLowerCase();
}

function currentMarketSymbol() {
  return normalizeMarketQuery(state.lastStatus?.marketSymbol || "");
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
  return kind === "index" ? "指数" : "股票";
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
    throw new Error(`请输入有效的${label}`);
  }
  if (value < min || value > max) {
    throw new Error(`${label}需要在 ${min} 到 ${max} 之间`);
  }
  return value;
}

function readComfortSettingsFromInputs() {
  const comfortTemperatureMin = readNumberField(
    elements.comfortTempMinInput,
    "最低舒适温度",
    COMFORT_LIMITS.temperatureMin,
    COMFORT_LIMITS.temperatureMax,
  );
  const comfortTemperatureMax = readNumberField(
    elements.comfortTempMaxInput,
    "最高舒适温度",
    COMFORT_LIMITS.temperatureMin,
    COMFORT_LIMITS.temperatureMax,
  );
  const comfortHumidityMin = readNumberField(
    elements.comfortHumidityMinInput,
    "最低舒适湿度",
    COMFORT_LIMITS.humidityMin,
    COMFORT_LIMITS.humidityMax,
  );
  const comfortHumidityMax = readNumberField(
    elements.comfortHumidityMaxInput,
    "最高舒适湿度",
    COMFORT_LIMITS.humidityMin,
    COMFORT_LIMITS.humidityMax,
  );

  if (comfortTemperatureMin > comfortTemperatureMax) {
    throw new Error("最低舒适温度不能高于最高舒适温度");
  }
  if (comfortHumidityMin > comfortHumidityMax) {
    throw new Error("最低舒适湿度不能高于最高舒适湿度");
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
    throw new Error(`行情搜索失败（HTTP ${response.status}）`);
  }

  const data = await response.json();
  if (!data?.ok) {
    throw new Error(data?.error || "行情搜索失败");
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
  const timestamp = new Date().toLocaleTimeString("zh-CN", {
    hour12: false,
  });
  const prefix =
    level === "error" ? "[错误]" : level === "device" ? "[设备]" : "[信息]";
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
  elements.connectionState.textContent = connected ? "已连接" : "未连接";
  elements.connectionState.classList.toggle("offline", !connected);

  const allowDeviceActions = connected && state.busyCount === 0;
  elements.disconnectButton.disabled = !connected;
  elements.statusButton.disabled = !allowDeviceActions;
  elements.scanButton.disabled = !allowDeviceActions;
  elements.saveButton.disabled = !allowDeviceActions;
  elements.applyButton.disabled = !allowDeviceActions;
  elements.syncButton.disabled = !allowDeviceActions;
  elements.refreshButton.disabled = !allowDeviceActions;
  elements.rebootButton.disabled = !allowDeviceActions;
  elements.marketSearchButton.disabled = state.busyCount > 0;
  renderMarketHotList();
  renderMarketResults();
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
      ? "还没有扫描结果。"
      : "连接设备后可以开始扫描。";
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

  for (const market of markets) {
    const button = document.createElement("button");
    button.type = "button";
    button.className = "market-chip";
    button.disabled = !state.connected || state.busyCount > 0;
    if (market.current) {
      button.classList.add("current");
    }
    button.innerHTML = `<strong>${escapeHtml(market.displayName)}</strong><span>${escapeHtml(
      market.code,
    )} · ${marketKindLabel(market.kind)}</span>`;
    button.addEventListener("click", () => {
      void selectMarket(market).catch(handleActionError);
    });
    elements.marketHotList.appendChild(button);
  }
}

function renderMarketResults() {
  elements.marketResults.innerHTML = "";
  const results = withCurrentFlag(state.marketResults);
  elements.marketResultCount.textContent = `${results.length} 个结果`;

  if (!results.length) {
    const empty = document.createElement("p");
    empty.className = "empty-state";
    empty.textContent = state.connected
      ? "没有找到匹配的标的。"
      : "连接设备后即可搜索并切换行情标的。";
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
        <span class="market-result-name">${escapeHtml(market.displayName)}</span>
        <span class="market-result-meta">${escapeHtml(market.code)} · ${escapeHtml(
      market.requestSymbol,
    )}</span>
      </span>
      <span class="market-kind">${market.current ? "当前" : marketKindLabel(
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
    return "信号未知";
  }
  if (rssi >= -55) {
    return `${rssi} dBm · 很强`;
  }
  if (rssi >= -67) {
    return `${rssi} dBm · 良好`;
  }
  if (rssi >= -75) {
    return `${rssi} dBm · 一般`;
  }
  return `${rssi} dBm · 较弱`;
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
  elements.pageName.textContent = status.page || "-";
  elements.wifiName.textContent = status.wifiConnected
    ? status.currentSsid || status.savedSsid || "-"
    : status.savedSsid || "未连接";
  elements.ipAddress.textContent = status.ipAddress || "-";
  elements.timezoneLabel.textContent =
    typeof status.timezone === "number"
      ? status.timezone >= 0
        ? `UTC+${status.timezone}`
        : `UTC${status.timezone}`
      : "-";
  elements.clockStyleLabel.textContent =
    status.clockStyle === "dashboard"
      ? "仪表盘"
      : status.clockStyle === "classic"
        ? "经典数字"
        : "-";
  const marketDisplayName = status.marketDisplayName || "上证指数";
  const marketCode = status.marketCode || "000001";
  elements.marketLabel.textContent = `${marketDisplayName} · ${marketCode}`;
  elements.marketCurrentName.textContent = marketDisplayName;
  elements.marketCurrentCode.textContent = marketCode;
  elements.batteryLabel.textContent =
    typeof status.batteryPercent === "number"
      ? `${status.batteryPercent}%`
      : "-";
  elements.syncState.textContent = status.timeSynced ? "已同步" : "未同步";
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

  const message = status.statusMessage || "已连接";
  setMessage(message, Boolean(status.statusError));
  renderMarketHotList();
  renderMarketResults();
}

async function writeJson(payload) {
  if (!state.writer) {
    throw new Error("串口未连接");
  }
  const line = `${JSON.stringify(payload)}\n`;
  await state.writer.write(state.encoder.encode(line));
  appendLog(`发送 ${line.trim()}`);
}

async function sendCommand(cmd, data = undefined, timeoutMs = REQUEST_TIMEOUT_MS) {
  if (!state.connected) {
    throw new Error("设备尚未连接");
  }

  const id = state.requestId++;
  const payload = { id, cmd };
  if (data !== undefined) {
    payload.data = data;
  }

  return new Promise(async (resolve, reject) => {
    const timeoutId = window.setTimeout(() => {
      state.pendingRequests.delete(id);
      reject(new Error(`${cmd} 超时`));
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
  appendLog(`接收 ${JSON.stringify(packet)}`, "device");
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
    const error = new Error(packet.error || "设备返回失败");
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
        state.readBuffer += state.decoder.decode(value, { stream: true });
        let newlineIndex = state.readBuffer.indexOf("\n");
        while (newlineIndex >= 0) {
          const rawLine = state.readBuffer.slice(0, newlineIndex);
          state.readBuffer = state.readBuffer.slice(newlineIndex + 1);
          handleLine(rawLine.replace(/\r$/, "").trim());
          newlineIndex = state.readBuffer.indexOf("\n");
        }
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
    appendLog(`无法解析设备返回: ${error.message}`, "error");
  }
}

async function closePort() {
  try {
    if (state.reader) {
      await state.reader.cancel();
    }
  } catch (error) {
    appendLog(`关闭读取器时出现提示: ${error.message}`, "error");
  }

  try {
    if (state.writer) {
      state.writer.releaseLock();
      state.writer = null;
    }
  } catch (error) {
    appendLog(`释放写入器时出现提示: ${error.message}`, "error");
  }

  try {
    if (state.port) {
      await state.port.close();
    }
  } catch (error) {
    appendLog(`关闭串口时出现提示: ${error.message}`, "error");
  }

  state.port = null;
  state.readBuffer = "";
  setConnected(false);
  stopPolling();
  rejectPending(new Error("串口已断开"));
  renderMarketHotList();
  renderMarketResults();
}

async function openPort(port) {
  if (!port) {
    throw new Error("没有可用的串口");
  }

  await closePort();
  state.port = port;
  await state.port.open({
    baudRate: DEFAULT_BAUD_RATE,
    dataBits: 8,
    stopBits: 1,
    parity: "none",
    flowControl: "none",
  });
  state.writer = state.port.writable.getWriter();
  state.decoder = new TextDecoder();
  state.readBuffer = "";
  setConnected(true);
  appendLog("串口已连接");
  setMessage("设备已连接，正在读取状态。");
  void readLoop().catch((error) => {
    appendLog(`读取串口失败: ${error.message}`, "error");
    setMessage(`读取串口失败：${error.message}`, true);
    void closePort();
  });
  startPolling();
  await refreshStatus();
  await searchMarkets("", { silent: true });
}

async function requestPortAndConnect() {
  const port = await navigator.serial.requestPort();
  await openPort(port);
}

async function reconnectAuthorizedPort() {
  const ports = await navigator.serial.getPorts();
  if (!ports.length) {
    throw new Error("还没有已授权设备");
  }
  await openPort(ports[0]);
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
    setMessage("正在读取设备状态。");
  }
  const data = await sendCommand("get_status");
  updateStatus(data);
}

async function scanWifi() {
  setMessage("正在扫描 Wi-Fi，请稍候。");
  const data = await sendCommand("scan_wifi", undefined, 25000);
  const networks = Array.isArray(data.networks) ? data.networks.length : 0;
  setMessage(networks ? `找到 ${networks} 个网络。` : "没有找到可用网络。");
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
    state.marketResults = dedupeMarkets(localMatches);
    renderMarketResults();
    if (!silent) {
      setMessage(
        state.connected
          ? "已加载热门标的。"
          : "已加载热门标的。连接设备后即可直接切换首页行情。",
      );
    }
    return;
  }

  if (!silent) {
    setMessage("正在搜索行情标的。");
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
        setMessage("搜索服务暂时不可用，先显示热门匹配标的。");
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
          ? `找到 ${state.marketResults.length} 个匹配标的。`
          : `找到 ${state.marketResults.length} 个匹配标的，连接设备后即可切换。`
        : "没有找到匹配的标的。",
    );
  }
}

async function selectMarket(market) {
  if (!state.connected) {
    throw new Error("请先连接设备，再切换行情标的");
  }

  const requestSymbol = String(market.requestSymbol || "");
  const displayName = String(market.displayName || "");
  if (!requestSymbol) {
    throw new Error("缺少标的代码");
  }

  setMessage(`正在切换到 ${displayName || requestSymbol}。`);
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
  setMessage(`首页行情已切换为 ${displayName || requestSymbol}。`);
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
    throw new Error("请先输入或选择 Wi-Fi 名称");
  }

  setMessage(
    connectNow ? "正在保存并连接设备。" : "正在保存设置。",
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
  elements.togglePasswordButton.textContent = "显示";
  setMessage(
    connectNow
      ? "设置已保存，设备已经尝试连接并同步时间。"
      : "设置已保存。",
  );
}

async function syncTime() {
  setMessage("正在同步时间。");
  const data = await sendCommand("sync_time", undefined, 20000);
  updateStatus(data);
  setMessage("时间同步已完成。");
}

async function refreshScreen() {
  setMessage("正在触发全量刷新。");
  const data = await sendCommand("refresh_screen", undefined, 12000);
  updateStatus(data);
  setMessage("设备已执行全量刷新。");
}

async function rebootDevice() {
  setMessage("正在重启设备。");
  await sendCommand("reboot", undefined, 5000);
  appendLog("设备已收到重启请求");
  setMessage("设备正在重启，稍后点“恢复已授权设备”即可。");
  await closePort();
}

function togglePasswordVisibility() {
  const nextType =
    elements.passwordInput.type === "password" ? "text" : "password";
  elements.passwordInput.type = nextType;
  elements.togglePasswordButton.textContent =
    nextType === "password" ? "显示" : "隐藏";
}

function installEventHandlers() {
  elements.connectButton.addEventListener("click", () =>
    withBusyWork(async () => {
      await requestPortAndConnect();
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
      setMessage("串口已断开。");
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
      appendLog("串口已被系统断开", "error");
      setMessage("设备串口已断开。", true);
      await closePort();
    }
  });
}

function handleActionError(error) {
  appendLog(error.message, "error");
  setMessage(error.message, true);
}

async function initialize() {
  renderTimezoneOptions();
  applyComfortInputs(DEFAULT_COMFORT_SETTINGS);
  renderNetworks();
  state.marketResults = withCurrentFlag(DEFAULT_MARKETS);
  renderMarketHotList();
  renderMarketResults();
  installEventHandlers();
  setConnected(false);

  if (!("serial" in navigator)) {
    elements.browserNote.textContent =
      "当前浏览器不支持 Web Serial，请使用 Chrome 或 Edge。";
    setMessage("当前浏览器不支持 Web Serial。", true);
    elements.connectButton.disabled = true;
    elements.reconnectButton.disabled = true;
    return;
  }

  if (!window.isSecureContext) {
    elements.browserNote.textContent =
      "请通过 localhost 或 HTTPS 打开这个页面。";
    setMessage("当前不是安全上下文，Web Serial 无法使用。", true);
    elements.connectButton.disabled = true;
    elements.reconnectButton.disabled = true;
    return;
  }

  appendLog("页面已就绪");

  try {
    const ports = await navigator.serial.getPorts();
    if (ports.length) {
      appendLog(`发现 ${ports.length} 个已授权设备`);
      setMessage("已发现授权过的设备，点“恢复已授权设备”即可连接。");
    } else {
      setMessage("请先点“连接设备”授权串口。");
    }
  } catch (error) {
    handleActionError(error);
  }
}

void initialize();

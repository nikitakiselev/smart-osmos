/**
 * @file web_server.cpp
 * @brief Реализация HTTP-сервера: главная страница и JSON API для автообновления.
 */

#include "web_server.h"
#include "tds_sensor.h"
#include "flow_meter.h"
#include <WebServer.h>
#include <WiFi.h>

static WebServer server(WEB_SERVER_PORT);

WebServerHandler webServerHandler;

// -----------------------------------------------------------------------------
// HTML-страница (простой интерфейс без сложного дизайна)
// -----------------------------------------------------------------------------

static const char PAGE_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Умный Осмос</title>
  <style>
    * { box-sizing: border-box; }
    body { font-family: 'Segoe UI', system-ui, sans-serif; margin: 0; padding: 20px; background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%); min-height: 100vh; color: #eee; }
    h1 { margin: 0 0 20px; font-size: 1.6rem; display: flex; align-items: center; gap: 10px; }
    .header-ico { font-size: 1.8rem; filter: drop-shadow(0 0 6px rgba(100,200,255,0.5)); }
    .card { background: rgba(255,255,255,0.08); padding: 20px; margin: 14px 0; border-radius: 14px; border: 1px solid rgba(255,255,255,0.1); box-shadow: 0 4px 20px rgba(0,0,0,0.2); transition: transform 0.2s, box-shadow 0.2s; }
    .card:hover { transform: translateY(-2px); box-shadow: 0 6px 24px rgba(0,0,0,0.3); }
    .card-tds { position: relative; overflow: hidden; }
    .card-tds::before { content: ''; position: absolute; top: 0; left: 0; right: 0; height: 4px; background: var(--tds-color, #4a9); transition: background 0.5s ease; }
    .tds-wrap { display: flex; align-items: center; gap: 20px; flex-wrap: wrap; }
    .tds-ico { font-size: 3.5rem; opacity: 0.9; animation: drip 2s ease-in-out infinite; }
    @keyframes drip { 0%, 100% { transform: translateY(0) scale(1); } 50% { transform: translateY(4px) scale(1.02); } }
    .tds-main { flex: 1; min-width: 160px; }
    .tds-value { font-size: 2.4rem; font-weight: 700; letter-spacing: 1px; color: var(--tds-color, #7f7); transition: color 0.5s ease; animation: pulse 0.5s ease-out; }
    @keyframes pulse { 0% { opacity: 0.7; transform: scale(1.05); } 100% { opacity: 1; transform: scale(1); } }
    .tds-unit { font-size: 1rem; color: rgba(255,255,255,0.6); font-weight: 400; margin-left: 4px; }
    .tds-desc { font-size: 1.1rem; margin-top: 8px; font-weight: 600; color: var(--tds-color); transition: color 0.5s ease; }
    .tds-hint { font-size: 0.8rem; color: rgba(255,255,255,0.45); margin-top: 4px; }
    .row { display: flex; flex-wrap: wrap; gap: 20px; }
    .item { flex: 1; min-width: 120px; padding: 12px; background: rgba(0,0,0,0.15); border-radius: 10px; border-left: 4px solid #3a7bd5; }
    .item.out { border-left-color: #6c5ce7; }
    .item .ico { font-size: 1.5rem; margin-bottom: 6px; opacity: 0.9; }
    .label { color: rgba(255,255,255,0.7); font-size: 0.85em; }
    .value { font-size: 1.5em; font-weight: 700; color: #fff; }
    h3 { margin: 0 0 14px; font-size: 1rem; display: flex; align-items: center; gap: 8px; color: rgba(255,255,255,0.9); }
    h3 .ico { font-size: 1.2rem; }
    .footer-bar { margin-top: 24px; display: flex; align-items: center; justify-content: space-between; flex-wrap: wrap; gap: 12px; }
    .footer-bar span { color: rgba(255,255,255,0.45); font-size: 0.85em; }
    .btn-refresh { display: inline-flex; align-items: center; gap: 8px; padding: 10px 18px; background: linear-gradient(135deg, #3a7bd5, #2d5a9e); color: #fff; border: none; border-radius: 10px; font-size: 0.95rem; font-weight: 600; cursor: pointer; box-shadow: 0 4px 14px rgba(58,123,213,0.4); transition: transform 0.15s, box-shadow 0.15s; }
    .btn-refresh:hover { transform: scale(1.03); box-shadow: 0 6px 18px rgba(58,123,213,0.5); }
    .btn-refresh:active { transform: scale(0.98); }
    .btn-refresh.spin .btn-ico { animation: spin 0.6s linear; }
    @keyframes spin { to { transform: rotate(360deg); } }
    .btn-ico { display: inline-block; font-size: 1.1rem; }
  </style>
</head>
<body>
  <h1><span class="header-ico">💧</span> Умный Осмос</h1>

  <div class="card card-tds">
    <div class="tds-wrap">
      <div class="tds-ico">💧</div>
      <div class="tds-main">
        <div class="tds-value"><span id="tds">--</span><span class="tds-unit">ppm</span></div>
        <div class="tds-desc" id="tdsDesc">—</div>
        <div class="tds-hint" id="tdsHint">Загрузка...</div>
      </div>
    </div>
  </div>

  <div class="card">
    <h3><span class="ico">🔄</span> Расход</h3>
    <div class="row">
      <div class="item">
        <div class="ico">📥</div>
        <div class="label">Вход (л/мин)</div>
        <div class="value"><span id="flowInRate">--</span></div>
      </div>
      <div class="item out">
        <div class="ico">📤</div>
        <div class="label">Выход (л/мин)</div>
        <div class="value"><span id="flowOutRate">--</span></div>
      </div>
    </div>
  </div>

  <div class="card">
    <h3><span class="ico">📊</span> Объём</h3>
    <div class="row">
      <div class="item">
        <div class="ico">📥</div>
        <div class="label">Вход (л)</div>
        <div class="value"><span id="volumeIn">--</span></div>
      </div>
      <div class="item out">
        <div class="ico">📤</div>
        <div class="label">Выход (л)</div>
        <div class="value"><span id="volumeOut">--</span></div>
      </div>
    </div>
  </div>

  <div class="footer-bar">
    <span id="lastUpdate">Обновление каждые 2 с</span>
    <button type="button" class="btn-refresh" id="btnRefresh" title="Обновить данные"><span class="btn-ico">🔄</span> Обновить</button>
  </div>

  <script>
    function getTdsState(ppm) {
      if (ppm == null || isNaN(ppm)) return { desc: '—', hint: 'Нет данных', color: '#888' };
      if (ppm < 20) return { desc: 'Ненадёжные данные', hint: 'Не используйте для оценки воды. Проверьте датчик и калибровку.', color: '#e74c3c' };
      if (ppm <= 50) return { desc: 'Отличная вода', hint: 'Идеально для питья', color: '#2ecc71' };
      if (ppm <= 150) return { desc: 'Хорошая вода', hint: 'Нормальное качество', color: '#27ae60' };
      if (ppm <= 300) return { desc: 'Приемлемо', hint: 'Умеренная жёсткость', color: '#f1c40f' };
      if (ppm <= 500) return { desc: 'Плохая вода', hint: 'Рекомендуется фильтрация', color: '#e67e22' };
      return { desc: 'Очень плохо', hint: 'Не рекомендуется пить', color: '#e74c3c' };
    }
    function applyTdsStyle(color) {
      document.documentElement.style.setProperty('--tds-color', color);
    }
    function updateData() {
      var btn = document.getElementById('btnRefresh');
      if (btn) { btn.classList.add('spin'); setTimeout(function() { btn.classList.remove('spin'); }, 600); }
      fetch('/api/data')
        .then(r => r.json())
        .then(function(data) {
          var tds = data.tds_ppm != null ? data.tds_ppm : null;
          document.getElementById('tds').textContent = tds != null ? tds.toFixed(1) : '--';
          var st = getTdsState(tds);
          document.getElementById('tdsDesc').textContent = st.desc;
          document.getElementById('tdsHint').textContent = st.hint;
          applyTdsStyle(st.color);
          document.getElementById('flowInRate').textContent = data.flow_in_lpm != null ? data.flow_in_lpm.toFixed(2) : '--';
          document.getElementById('flowOutRate').textContent = data.flow_out_lpm != null ? data.flow_out_lpm.toFixed(2) : '--';
          document.getElementById('volumeIn').textContent = data.volume_in_l != null ? data.volume_in_l.toFixed(2) : '--';
          document.getElementById('volumeOut').textContent = data.volume_out_l != null ? data.volume_out_l.toFixed(2) : '--';
          document.getElementById('lastUpdate').textContent = 'Обновлено: ' + new Date().toLocaleTimeString('ru');
        })
        .catch(function() { document.getElementById('lastUpdate').textContent = 'Ошибка загрузки'; });
    }
    document.getElementById('btnRefresh').onclick = updateData;
    updateData();
    setInterval(updateData, 2000);
  </script>
</body>
</html>
)rawliteral";

// -----------------------------------------------------------------------------
// Обработчики
// -----------------------------------------------------------------------------

void WebServerHandler::handleRoot()
{
    server.send_P(200, "text/html; charset=utf-8", PAGE_HTML);
}

void WebServerHandler::sendJsonData()
{
    float tds = tdsSensor.getPpm();
    float fi = flowMeters.inlet().getRateLpm();
    float fo = flowMeters.outlet().getRateLpm();
    float vi = flowMeters.inlet().getTotalLiters();
    float vo = flowMeters.outlet().getTotalLiters();

    String buf;
    buf.reserve(180);
    buf += "{\"tds_ppm\":";
    buf += String(tds, 1);
    buf += ",\"flow_in_lpm\":";
    buf += String(fi, 2);
    buf += ",\"flow_out_lpm\":";
    buf += String(fo, 2);
    buf += ",\"volume_in_l\":";
    buf += String(vi, 2);
    buf += ",\"volume_out_l\":";
    buf += String(vo, 2);
    buf += "}";
    server.send(200, "application/json; charset=utf-8", buf);
}

void WebServerHandler::handleApiData()
{
    sendJsonData();
}

WebServerHandler::WebServerHandler()
{
}

void WebServerHandler::begin()
{
    server.on("/", std::bind(&WebServerHandler::handleRoot, this));
    server.on("/api/data", std::bind(&WebServerHandler::handleApiData, this));
    server.begin();
}

void WebServerHandler::update()
{
    server.handleClient();
}

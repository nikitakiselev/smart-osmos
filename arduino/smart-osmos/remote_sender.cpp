/**
 * @file remote_sender.cpp
 * @brief Отправка телеметрии (JSON) на HTTPS-сервер по таймеру.
 */

#include "remote_sender.h"
#include "config.h"
#include "tds_sensor.h"
#include "flow_meter.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#if REMOTE_SEND_INTERVAL_MS > 0
#include "server_secrets.h"
#endif

RemoteSender remoteSender;

RemoteSender::RemoteSender()
    : _enabled(true)
    , _lastSendMs(0)
    , _busy(false)
{
}

void RemoteSender::begin()
{
    _enabled = (REMOTE_SEND_INTERVAL_MS > 0);
    _lastSendMs = 0;
    _busy = false;
}

void RemoteSender::update()
{
    if (!_enabled || _busy)
        return;

    unsigned long now = millis();
    if (now - _lastSendMs < (unsigned long)REMOTE_SEND_INTERVAL_MS)
        return;

    _lastSendMs = now;
    sendPayload();
}

void RemoteSender::sendPayload()
{
#if REMOTE_SEND_INTERVAL_MS <= 0
    return;
#else
    WiFiClientSecure client;
    client.setInsecure();  // для самоподписанного сертификата; в продакшене задайте setCACert()

    HTTPClient http;
    String url;
    url += "https://";
    url += SERVER_HOST;
    url += ":";
    url += String(SERVER_PORT);
    url += SERVER_PATH;

    if (!http.begin(client, url)) {
        Serial.println(F("[Remote] begin failed"));
        return;
    }

    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-API-Key", String(SERVER_API_KEY));

    float tds = tdsSensor.getPpm();
    float fi = flowMeters.inlet().getRateLpm();
    float fo = flowMeters.outlet().getRateLpm();
    float vi = flowMeters.inlet().getTotalLiters();
    float vo = flowMeters.outlet().getTotalLiters();

    String json;
    json.reserve(220);
    json += "{\"tds_ppm\":";
    json += String(tds, 1);
    json += ",\"flow_in_lpm\":";
    json += String(fi, 2);
    json += ",\"flow_out_lpm\":";
    json += String(fo, 2);
    json += ",\"volume_in_l\":";
    json += String(vi, 2);
    json += ",\"volume_out_l\":";
    json += String(vo, 2);
    json += "}";

    Serial.print(F("[Remote] sending "));
    Serial.print(json);
    Serial.println(F(" ..."));

    int code = http.POST(json);

    if (code == HTTP_CODE_OK || code == HTTP_CODE_CREATED || code == HTTP_CODE_ACCEPTED) {
        Serial.print(F("[Remote] OK code="));
        Serial.println(code);
    } else {
        Serial.print(F("[Remote] POST failed: "));
        Serial.println(code);
    }

    http.end();
#endif
}

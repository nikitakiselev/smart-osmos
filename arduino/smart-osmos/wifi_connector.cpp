/**
 * @file wifi_connector.cpp
 * @brief Реализация неблокирующего подключения к Wi-Fi с реконнектом.
 */

#include "wifi_connector.h"
#include <WiFi.h>
#include <Arduino.h>

WifiConnector wifiConnector;

WifiConnector::WifiConnector()
    : _connected(false)
    , _lastAttemptMs(0)
    , _attemptInProgress(false)
{
}

void WifiConnector::begin()
{
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    _connected = false;
    _attemptInProgress = false;
    _lastAttemptMs = 0;
    tryConnect();  // первая попытка сразу при старте
}

void WifiConnector::update()
{
    bool nowConnected = (WiFi.status() == WL_CONNECTED);

    if (nowConnected) {
        if (!_connected) {
            _connected = true;
            _attemptInProgress = false;
            Serial.println(F("[WiFi] Подключено"));
            Serial.print(F("[WiFi] IP: "));
            Serial.println(WiFi.localIP());
            Serial.print(F("[WiFi] RSSI: "));
            Serial.print(WiFi.RSSI());
            Serial.println(F(" dBm"));
        }
        return;
    }

    _connected = false;

    if (_attemptInProgress) {
        // Ждём результата текущей попытки (с таймаутом)
        if (millis() - _lastAttemptMs > (unsigned long)WIFI_CONNECT_TIMEOUT_MS) {
            _attemptInProgress = false;
            WiFi.disconnect();
            Serial.println(F("[WiFi] Таймаут, повтор через 30 с"));
        }
        return;
    }

    // Периодическая повторная попытка
    unsigned long now = millis();
    if (now - _lastAttemptMs < (unsigned long)WIFI_RETRY_INTERVAL_MS)
        return;

    Serial.println(F("[WiFi] Повторная попытка подключения..."));
    tryConnect();
}

void WifiConnector::tryConnect()
{
    _lastAttemptMs = millis();
    _attemptInProgress = true;
    Serial.print(F("[WiFi] Подключение к "));
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

bool WifiConnector::isConnected() const
{
    return _connected && (WiFi.status() == WL_CONNECTED);
}

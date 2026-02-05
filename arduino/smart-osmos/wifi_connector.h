/**
 * @file wifi_connector.h
 * @brief Подключение к Wi-Fi с повторными попытками при недоступности сети.
 */

#ifndef WIFI_CONNECTOR_H
#define WIFI_CONNECTOR_H

#include <Arduino.h>
#include "config.h"

class WifiConnector {
public:
    WifiConnector();

    /** Инициализация (вызвать в setup) */
    void begin();

    /**
     * Неблокирующее поддержание соединения. Вызывать в loop.
     * При отключении — повторная попытка через WIFI_RETRY_INTERVAL_MS.
     */
    void update();

    /** Подключены ли к Wi-Fi */
    bool isConnected() const;

private:
    void tryConnect();
    void onDisconnected();

    bool _connected;
    unsigned long _lastAttemptMs;
    bool _attemptInProgress;
};

extern WifiConnector wifiConnector;

#endif // WIFI_CONNECTOR_H

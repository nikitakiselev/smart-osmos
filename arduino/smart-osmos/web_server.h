/**
 * @file web_server.h
 * @brief Встроенный HTTP-сервер: API (JSON) и HTML-страница с автообновлением.
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include "config.h"

class WebServerHandler {
public:
    WebServerHandler();

    /** Инициализация маршрутов и запуск сервера (вызвать после подключения Wi-Fi) */
    void begin();

    /** Обработка запросов (вызывать в loop, когда Wi-Fi подключён) */
    void update();

private:
    void handleRoot();
    void handleApiData();
    void sendJsonData();
};

extern WebServerHandler webServerHandler;

#endif // WEB_SERVER_H

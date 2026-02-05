/**
 * @file smart-osmos.ino
 * @brief Главный файл прошивки "Умный Осмос" — ESP32, TDS, расходомеры, Wi-Fi, веб-интерфейс.
 *
 * Архитектура: неблокирующий loop, минимум delay.
 * Данные датчиков обновляются по таймеру; веб-сервер обрабатывает запросы при подключённом Wi-Fi.
 * В будущем легко добавить сохранение калибровки/счётчиков в NVS (см. комментарии в config.h и модулях).
 */

#include "config.h"
#include "tds_sensor.h"
#include "flow_meter.h"
#include "wifi_connector.h"
#include "web_server.h"
#include "remote_sender.h"
#include <WiFi.h>

static bool webServerStarted = false;

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println();
    Serial.println(F("=== Smart Osmos ==="));
    Serial.println(F("Прошивка загружена, инициализация..."));

    tdsSensor.begin();
    Serial.println(F("[OK] TDS-метр"));
    flowMeters.begin();
    Serial.println(F("[OK] Расходомеры"));
    wifiConnector.begin();
    remoteSender.begin();
    Serial.println(F("[OK] WiFi запущен, ожидание подключения..."));
    Serial.println();
}

void loop()
{
    // Поддержание Wi-Fi и реконнект при недоступности
    wifiConnector.update();

    // Обновление показаний TDS (внутри учёт интервала опроса)
    tdsSensor.update();

    // Обновление окна расчёта текущего расхода по расходомерам
    flowMeters.update();

    // Обработка HTTP только при подключённом Wi-Fi
    if (wifiConnector.isConnected()) {
        if (!webServerStarted) {
            webServerHandler.begin();
            webServerStarted = true;
            Serial.println(F("[Web] Сервер запущен"));
            Serial.print(F("      http://"));
            Serial.println(WiFi.localIP());
        }
        webServerHandler.update();
        if (remoteSender.isEnabled())
            remoteSender.update();
    }

    // Без delay — полностью неблокирующий цикл
}

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
#include "led_io.h"
#include <WiFi.h>

static bool webServerStarted = false;
static unsigned long lastRedBlinkMs = 0;
const unsigned long RED_BLINK_INTERVAL_MS = 500;

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
    Led::begin();
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

    // Красный: мигает при подключении к Wi-Fi, гаснет при успешном подключении
    bool wifiOk = wifiConnector.isConnected();
    if (wifiOk) {
        Led::redOff();
    } else {
        unsigned long now = millis();
        if (now - lastRedBlinkMs >= RED_BLINK_INTERVAL_MS) {
            Led::redToggle();
            lastRedBlinkMs = now;
        }
    }

    Led::update();

    // Обработка HTTP только при подключённом Wi-Fi
    if (wifiOk) {
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

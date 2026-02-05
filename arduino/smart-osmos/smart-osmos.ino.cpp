# 1 "C:\\Users\\user\\AppData\\Local\\Temp\\tmp3lrn80h4"
#include <Arduino.h>
# 1 "C:/Users/user/Desktop/smart-osmos/arduino/smart-osmos/smart-osmos.ino"
# 10 "C:/Users/user/Desktop/smart-osmos/arduino/smart-osmos/smart-osmos.ino"
#include "config.h"
#include "tds_sensor.h"
#include "flow_meter.h"
#include "wifi_connector.h"
#include "web_server.h"
#include <WiFi.h>

static bool webServerStarted = false;
void setup();
void loop();
#line 19 "C:/Users/user/Desktop/smart-osmos/arduino/smart-osmos/smart-osmos.ino"
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
    Serial.println(F("[OK] WiFi запущен, ожидание подключения..."));
    Serial.println();
}

void loop()
{

    wifiConnector.update();


    tdsSensor.update();


    flowMeters.update();


    if (wifiConnector.isConnected()) {
        if (!webServerStarted) {
            webServerHandler.begin();
            webServerStarted = true;
            Serial.println(F("[Web] Сервер запущен"));
            Serial.print(F("      http://"));
            Serial.println(WiFi.localIP());
        }
        webServerHandler.update();
    }


}
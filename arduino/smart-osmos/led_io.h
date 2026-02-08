/**
 * @file led_io.h
 * @brief Управление красным и зелёным светодиодами (Wi-Fi и отправка данных).
 */

#ifndef LED_IO_H
#define LED_IO_H

#include <Arduino.h>

namespace Led {

/** Инициализация (вызвать в setup). */
void begin();

/** Красный: вкл / выкл / переключить. Мигает при подключении к Wi-Fi, гаснет при успешном подключении. */
void redOn();
void redOff();
void redToggle();

/** Зелёный: вкл / выкл. Загорается после успешной отправки данных. */
void greenOn();
void greenOff();

/** Зелёный включить на заданное время (мс); погаснет автоматически в update(). */
void greenOnForMs(unsigned long ms);

/** Вызывать в loop() — гасит зелёный по таймеру. */
void update();

} // namespace Led

#endif // LED_IO_H

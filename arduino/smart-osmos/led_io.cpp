/**
 * @file led_io.cpp
 * @brief Реализация: красный — Wi-Fi, зелёный — успешная отправка.
 */

#include "led_io.h"
#include "config.h"

#if PIN_LED_RED > 0 || PIN_LED_GREEN > 0
static inline void writePin(uint8_t pin, bool level) {
    if (pin == 0) return;
    digitalWrite(pin, (LED_ACTIVE_LOW ? !level : level) ? HIGH : LOW);
}
#endif

static unsigned long greenOffAt = 0;

void Led::begin() {
#if PIN_LED_RED > 0
    pinMode(PIN_LED_RED, OUTPUT);
    writePin(PIN_LED_RED, false);
#endif
#if PIN_LED_GREEN > 0
    pinMode(PIN_LED_GREEN, OUTPUT);
    writePin(PIN_LED_GREEN, false);
#endif
    greenOffAt = 0;
}

void Led::redOn() {
#if PIN_LED_RED > 0
    writePin(PIN_LED_RED, true);
#endif
}

void Led::redOff() {
#if PIN_LED_RED > 0
    writePin(PIN_LED_RED, false);
#endif
}

void Led::redToggle() {
#if PIN_LED_RED > 0
    bool level = (digitalRead(PIN_LED_RED) == HIGH);
    if (LED_ACTIVE_LOW) level = !level;
    writePin(PIN_LED_RED, !level);
#endif
}

void Led::greenOn() {
#if PIN_LED_GREEN > 0
    writePin(PIN_LED_GREEN, true);
#endif
    greenOffAt = 0;
}

void Led::greenOff() {
#if PIN_LED_GREEN > 0
    writePin(PIN_LED_GREEN, false);
#endif
    greenOffAt = 0;
}

void Led::greenOnForMs(unsigned long ms) {
#if PIN_LED_GREEN > 0
    writePin(PIN_LED_GREEN, true);
    greenOffAt = millis() + ms;
#endif
}

void Led::update() {
    if (greenOffAt != 0 && (long)(millis() - greenOffAt) >= 0) {
        greenOff();
    }
}

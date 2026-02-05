/**
 * @file flow_meter.cpp
 * @brief Реализация подсчёта импульсов расходомеров с дебаунсом и расчётом л/мин.
 */

#include "flow_meter.h"

FlowMeters flowMeters;

// -----------------------------------------------------------------------------
// FlowMeter
// -----------------------------------------------------------------------------

FlowMeter::FlowMeter(uint8_t pin, const char* name)
    : _pin(pin)
    , _name(name)
    , _pulseCount(0)
    , _lastPulseUs(0)
    , _rateWindowStartMs(0)
    , _pulsesInWindow(0)
{
}

void FlowMeter::begin()
{
    pinMode(_pin, INPUT_PULLUP);
    _pulseCount = 0;
    _lastPulseUs = 0;
    _rateWindowStartMs = millis();
    _pulsesInWindow = 0;
}

void FlowMeter::onPulse()
{
    unsigned long now = micros();
    // Защита от дребезга: игнорируем импульсы чаще чем FLOW_DEBOUNCE_US
    if (now - _lastPulseUs < (unsigned long)FLOW_DEBOUNCE_US)
        return;
    _lastPulseUs = now;

    _pulseCount++;
    _pulsesInWindow++;
}

float FlowMeter::getTotalLiters() const
{
    return (float)_pulseCount / (float)FLOW_PULSES_PER_LITER;
}

void FlowMeter::update()
{
    unsigned long now = millis();
    // Сдвигаем окно: если прошло больше FLOW_RATE_WINDOW_MS — сбрасываем окно
    if (now - _rateWindowStartMs >= FLOW_RATE_WINDOW_MS) {
        _rateWindowStartMs = now;
        _pulsesInWindow = 0;
    }
}

float FlowMeter::getRateLpm() const
{
    unsigned long now = millis();
    unsigned long elapsed = now - _rateWindowStartMs;
    if (elapsed < 500)  // минимум 0.5 с для устойчивой оценки
        return 0.0f;

    float minutes = (float)elapsed / 60000.0f;
    float liters = (float)_pulsesInWindow / (float)FLOW_PULSES_PER_LITER;
    return liters / minutes;
}

void FlowMeter::reset()
{
    _pulseCount = 0;
    _rateWindowStartMs = millis();
    _pulsesInWindow = 0;
}

// -----------------------------------------------------------------------------
// FlowMeters
// -----------------------------------------------------------------------------

FlowMeters::FlowMeters()
    : _in(PIN_FLOW_IN, "in")
    , _out(PIN_FLOW_OUT, "out")
{
}

void FlowMeters::begin()
{
    _in.begin();
    _out.begin();

    attachInterrupt(digitalPinToInterrupt(PIN_FLOW_IN),  flowInIsr,  FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_FLOW_OUT), flowOutIsr, FALLING);
}

void FlowMeters::update()
{
    _in.update();
    _out.update();
}

// -----------------------------------------------------------------------------
// ISR (должны быть быстрыми, без блокировок)
// -----------------------------------------------------------------------------

void IRAM_ATTR flowInIsr()
{
    flowMeters.inlet().onPulse();
}

void IRAM_ATTR flowOutIsr()
{
    flowMeters.outlet().onPulse();
}

/**
 * @file flow_meter.h
 * @brief Драйвер расходомеров с импульсным выходом: прерывания, дебаунс, л/мин и объём.
 */

#ifndef FLOW_METER_H
#define FLOW_METER_H

#include <Arduino.h>
#include "config.h"

/** Один расходомер (вход или выход) */
class FlowMeter {
public:
    FlowMeter(uint8_t pin, const char* name);

    void begin();
    /** Вызывать из ISR при каждом импульсе (с учётом дебаунса — внутри) */
    void onPulse();

    /** Суммарный объём в литрах (накопленный с момента старта) */
    float getTotalLiters() const;

    /** Текущий расход в л/мин (по скользящему окну FLOW_RATE_WINDOW_MS) */
    float getRateLpm() const;

    /** Количество зафиксированных импульсов (для отладки) */
    uint32_t getPulseCount() const { return _pulseCount; }

    /** Сброс счётчика импульсов и истории (например, при калибровке) */
    void reset();

    /** Обновление расчёта текущего расхода по времени (вызывать в loop) */
    void update();

private:
    uint8_t _pin;
    const char* _name;
    volatile uint32_t _pulseCount;
    volatile unsigned long _lastPulseUs;

    // Окно для расчёта л/мин: время начала окна и счётчик импульсов в окне
    unsigned long _rateWindowStartMs;
    volatile uint32_t _pulsesInWindow;
};

/** Агрегатор двух расходомеров: вход и выход */
class FlowMeters {
public:
    FlowMeters();

    void begin();
    void update();

    FlowMeter& inlet()  { return _in; }
    FlowMeter& outlet() { return _out; }

    const FlowMeter& inlet()  const { return _in; }
    const FlowMeter& outlet() const { return _out; }

private:
    FlowMeter _in;
    FlowMeter _out;
};

extern FlowMeters flowMeters;

// Обработчики прерываний (должны быть в .cpp или .ino, но объявление здесь)
void IRAM_ATTR flowInIsr();
void IRAM_ATTR flowOutIsr();

#endif // FLOW_METER_H

/**
 * @file tds_sensor.h
 * @brief Драйвер TDS-метра: чтение ADC, усреднение, перевод в ppm.
 */

#ifndef TDS_SENSOR_H
#define TDS_SENSOR_H

#include <Arduino.h>
#include "config.h"

class TdsSensor {
public:
    TdsSensor();

    /** Инициализация (вызвать в setup) */
    void begin();

    /**
     * Неблокирующее обновление показаний. Вызывать в loop как можно чаще.
     * Внутри учитывается TDS_READ_INTERVAL_MS.
     */
    void update();

    /** Текущее значение TDS в ppm (после усреднения и калибровки) */
    float getPpm() const { return _ppm; }

    /** Сырое усреднённое напряжение в мВ (для отладки) */
    float getVoltageMv() const { return _voltageMv; }

    /** Задать калибровочные коэффициенты (для будущего сохранения в NVS) */
    void setCalibration(float k, float offset);
    void getCalibration(float& k, float& offset) const;

private:
    void readAndAverage();

    float _ppm;
    float _voltageMv;
    uint16_t _samples[TDS_SAMPLES_COUNT];
    uint8_t _sampleIndex;
    uint8_t _samplesFilled;
    unsigned long _lastReadMs;

    float _calibK;
    float _calibOffset;
};

extern TdsSensor tdsSensor;

#endif // TDS_SENSOR_H

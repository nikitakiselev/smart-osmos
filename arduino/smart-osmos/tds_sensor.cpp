/**
 * @file tds_sensor.cpp
 * @brief Реализация чтения TDS-метра с усреднением и калибровкой.
 */

#include "tds_sensor.h"

TdsSensor tdsSensor;

TdsSensor::TdsSensor()
    : _ppm(0.0f)
    , _voltageMv(0.0f)
    , _sampleIndex(0)
    , _samplesFilled(0)
    , _lastReadMs(0)
    , _calibK(TDS_CALIBRATION_K)
    , _calibOffset(TDS_CALIBRATION_OFFSET)
{
    memset(_samples, 0, sizeof(_samples));
}

void TdsSensor::begin()
{
    pinMode(PIN_TDS, INPUT);
    analogReadResolution(TDS_ADC_BITS);
    analogSetAttenuation(ADC_11db);  // полный диапазон 0..3.3V на ESP32
}

void TdsSensor::update()
{
    unsigned long now = millis();
    if (now - _lastReadMs < TDS_READ_INTERVAL_MS)
        return;
    _lastReadMs = now;

    readAndAverage();
}

void TdsSensor::readAndAverage()
{
    _samples[_sampleIndex] = analogRead(PIN_TDS);
    _sampleIndex = (_sampleIndex + 1) % TDS_SAMPLES_COUNT;
    if (_samplesFilled < TDS_SAMPLES_COUNT)
        _samplesFilled++;

    uint32_t sum = 0;
    for (uint8_t i = 0; i < _samplesFilled; i++)
        sum += _samples[i];
    uint32_t avgAdc = sum / _samplesFilled;

    // ADC → напряжение (В)
    const uint32_t maxAdc = (1u << TDS_ADC_BITS) - 1;
    _voltageMv = (float)avgAdc * (float)TDS_VREF_MV / (float)maxAdc;
    float v = _voltageMv / 1000.0f;

    // Стандартная формула TDS meter v1.0 (0..2.3V → 0..1000 ppm):
    // ppm = (133.42*V³ - 255.86*V² + 857.39*V) * 0.5
    float rawPpm = (133.42f * v * v * v - 255.86f * v * v + 857.39f * v) * 0.5f;
    if (rawPpm < 0.0f) rawPpm = 0.0f;

    // Калибровка: итог = raw * K + offset (K=1, offset=0 по умолчанию)
    _ppm = rawPpm * _calibK + _calibOffset;
    if (_ppm < 0.0f)
        _ppm = 0.0f;
}

void TdsSensor::setCalibration(float k, float offset)
{
    _calibK = k;
    _calibOffset = offset;
}

void TdsSensor::getCalibration(float& k, float& offset) const
{
    k = _calibK;
    offset = _calibOffset;
}

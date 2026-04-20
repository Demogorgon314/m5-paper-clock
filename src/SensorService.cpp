#include "SensorService.h"

void SensorService::begin() {
    M5.SHT30.Begin();
}

EnvironmentReading SensorService::read() const {
    EnvironmentReading reading;
    M5.SHT30.UpdateData();
    if (M5.SHT30.GetError() != 0) {
        return reading;
    }

    reading.temperature = M5.SHT30.GetTemperature();
    reading.humidity = M5.SHT30.GetRelHumidity();
    reading.valid = true;
    return reading;
}

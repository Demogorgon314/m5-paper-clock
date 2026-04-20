#pragma once

#include <Arduino.h>
#include <M5EPD.h>

struct EnvironmentReading {
    float temperature = 0.0f;
    float humidity = 0.0f;
    bool valid = false;
};

class SensorService {
public:
    void begin();
    EnvironmentReading read() const;
};

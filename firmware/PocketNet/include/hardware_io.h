#pragma once

#include <Arduino.h>

namespace HardwareIO
{
    void begin();

    void setRGB(bool red, bool green, bool blue);
    void clearRGB();

    void playTone(uint16_t frequency, uint16_t durationMs);
}
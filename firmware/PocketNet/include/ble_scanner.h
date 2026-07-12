#pragma once

#include <Arduino.h>
#include <vector>

namespace BLEScanner
{
    struct Device
    {
        String name;
        String address;
        int rssi;
        bool connectable;
    };

    void begin();
    bool scan(uint32_t durationMs = 5000);

    int count();
    const Device* getDevice(int index);

    void clear();
}
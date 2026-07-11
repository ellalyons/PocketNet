#pragma once

#include <Arduino.h>

namespace Input
{
    enum class Button
    {
        Left,
        Right,
        Select,
        Back
    };

    void begin();
    void update();

    bool wasPressed(Button button);
}
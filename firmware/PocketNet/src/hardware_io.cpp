#include "hardware_io.h"
#include "hardware_config.h"

namespace
{
    constexpr int BUZZER_CHANNEL = 0;
    constexpr int BUZZER_RESOLUTION = 8;
}

namespace HardwareIO
{
    void begin()
    {
        pinMode(PIN_RGB_R, OUTPUT);
        pinMode(PIN_RGB_G, OUTPUT);
        pinMode(PIN_RGB_B, OUTPUT);

        clearRGB();

        ledcSetup(
            BUZZER_CHANNEL,
            2500,
            BUZZER_RESOLUTION
        );

        ledcAttachPin(
            PIN_BUZZER,
            BUZZER_CHANNEL
        );
    }

    void setRGB(bool red, bool green, bool blue)
    {
        digitalWrite(PIN_RGB_R, red);
        digitalWrite(PIN_RGB_G, green);
        digitalWrite(PIN_RGB_B, blue);
    }

    void clearRGB()
    {
        setRGB(LOW, LOW, LOW);
    }

    void playTone(uint16_t frequency, uint16_t durationMs)
    {
        ledcWriteTone(BUZZER_CHANNEL, frequency);
        delay(durationMs);
        ledcWriteTone(BUZZER_CHANNEL, 0);
    }
}
#include <Arduino.h>
#include "hardware_config.h"

void setRGB(bool r, bool g, bool b)
{
    digitalWrite(PIN_RGB_R, r);
    digitalWrite(PIN_RGB_G, g);
    digitalWrite(PIN_RGB_B, b);
}

void setup()
{
    Serial.begin(115200);

    pinMode(PIN_RGB_R, OUTPUT);
    pinMode(PIN_RGB_G, OUTPUT);
    pinMode(PIN_RGB_B, OUTPUT);

    Serial.println("PocketNet Boot");
}

void loop()
{
    Serial.println("Red");
    setRGB(HIGH, LOW, LOW);
    delay(1000);

    Serial.println("Green");
    setRGB(LOW, HIGH, LOW);
    delay(1000);

    Serial.println("Blue");
    setRGB(LOW, LOW, HIGH);
    delay(1000);
}
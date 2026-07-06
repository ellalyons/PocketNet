#include <Arduino.h>
#include "hardware_config.h"

void setRGB(bool r, bool g, bool b)
{
    digitalWrite(PIN_RGB_R, r);
    digitalWrite(PIN_RGB_G, g);
    digitalWrite(PIN_RGB_B, b);
}

void beep(int durationMs)
{
    digitalWrite(PIN_BUZZER, HIGH);
    delay(durationMs);
    digitalWrite(PIN_BUZZER, LOW);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("=================================");
    Serial.println(" PocketNet Hardware Bring-up");
    Serial.println("=================================");

    // RGB LED
    pinMode(PIN_RGB_R, OUTPUT);
    pinMode(PIN_RGB_G, OUTPUT);
    pinMode(PIN_RGB_B, OUTPUT);

    // Buzzer
    pinMode(PIN_BUZZER, OUTPUT);

    // Turn everything off
    setRGB(LOW, LOW, LOW);
    digitalWrite(PIN_BUZZER, LOW);
}

void loop()
{
    Serial.println("Testing RED");
    setRGB(HIGH, LOW, LOW);
    delay(1000);

    Serial.println("Testing GREEN");
    setRGB(LOW, HIGH, LOW);
    delay(1000);

    Serial.println("Testing BLUE");
    setRGB(LOW, LOW, HIGH);
    delay(1000);

    Serial.println("Testing OFF");
    setRGB(LOW, LOW, LOW);
    delay(500);

    Serial.println("Testing BUZZER");
    beep(100);
    delay(900);
}
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
    Serial.println(" PocketNet Input Bring-up");
    Serial.println("=================================");

    pinMode(PIN_RGB_R, OUTPUT);
    pinMode(PIN_RGB_G, OUTPUT);
    pinMode(PIN_RGB_B, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);

    pinMode(PIN_LEFT, INPUT);
    pinMode(PIN_RIGHT, INPUT);
    pinMode(PIN_SELECT, INPUT);
    pinMode(PIN_ALT, INPUT);

    setRGB(LOW, LOW, LOW);
    digitalWrite(PIN_BUZZER, LOW);

    Serial.println("Press buttons / move joystick...");
}

void loop()
{
    bool leftPressed   = digitalRead(PIN_LEFT);
    bool rightPressed  = digitalRead(PIN_RIGHT);
    bool selectPressed = digitalRead(PIN_SELECT);
    bool altPressed    = digitalRead(PIN_ALT);

    if (leftPressed) {
        Serial.println("LEFT pressed");
        setRGB(HIGH, LOW, LOW);
        beep(50);
        delay(250);
    }

    if (rightPressed) {
        Serial.println("RIGHT pressed");
        setRGB(LOW, HIGH, LOW);
        beep(50);
        delay(250);
    }

    if (selectPressed) {
        Serial.println("SELECT pressed");
        setRGB(LOW, LOW, HIGH);
        beep(50);
        delay(250);
    }

    if (altPressed) {
        Serial.println("BUTTON pressed");
        setRGB(HIGH, HIGH, HIGH);
        beep(50);
        delay(250);
    }

    setRGB(LOW, LOW, LOW);
    delay(20);
}
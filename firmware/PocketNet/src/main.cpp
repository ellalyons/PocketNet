#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("PocketNet booting...");
  Serial.println("ESP32 is alive.");
}

void loop() {
  Serial.println("PocketNet heartbeat");
  delay(1000);
}
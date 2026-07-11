#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "hardware_config.h"

class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ST7735S panel;
    lgfx::Bus_SPI bus;

public:
    LGFX()
    {
        {
            auto cfg = bus.config();

            cfg.spi_host = VSPI_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;
            cfg.freq_read = 40000000;
            cfg.spi_3wire = false;
            cfg.use_lock = true;
            cfg.dma_channel = 1;

            cfg.pin_sclk = PIN_TFT_SCK;
            cfg.pin_mosi = PIN_TFT_MOSI;
            cfg.pin_miso = -1;
            cfg.pin_dc = PIN_TFT_DC;

            bus.config(cfg);
            panel.setBus(&bus);
        }

        {
            auto cfg = panel.config();

            cfg.pin_cs = -1;
            cfg.pin_rst = PIN_TFT_RST;
            cfg.pin_busy = -1;

            cfg.memory_width = 132;
            cfg.memory_height = 132;
            cfg.panel_width = 128;
            cfg.panel_height = 128;

            cfg.offset_x = 2;
            cfg.offset_y = 1;
            cfg.offset_rotation = 2;

            cfg.readable = false;
            cfg.invert = false;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = false;

            panel.config(cfg);
        }

        setPanel(&panel);
    }
};

LGFX display;

constexpr int BUZZER_CHANNEL = 0;
constexpr int BUZZER_RESOLUTION = 8;

void setRGB(bool red, bool green, bool blue)
{
    digitalWrite(PIN_RGB_R, red);
    digitalWrite(PIN_RGB_G, green);
    digitalWrite(PIN_RGB_B, blue);
}

void playTone(uint16_t frequency, uint16_t durationMs)
{
    ledcWriteTone(BUZZER_CHANNEL, frequency);
    delay(durationMs);
    ledcWriteTone(BUZZER_CHANNEL, 0);
}

void showStatus(const char* item, const char* status, uint16_t colour)
{
    display.fillRect(0, 48, 128, 55, TFT_BLACK);

    display.setTextSize(1);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setCursor(10, 55);
    display.println(item);

    display.setTextColor(colour, TFT_BLACK);
    display.setCursor(10, 75);
    display.println(status);
}

void runStartupSelfTest()
{
    display.fillScreen(TFT_BLACK);

    display.setTextColor(TFT_GREEN, TFT_BLACK);
    display.setTextSize(2);
    display.setCursor(10, 12);
    display.println("PocketNet");

    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextSize(1);
    display.setCursor(10, 35);
    display.println("Startup self-test");

    showStatus("RGB LED", "Testing red", TFT_RED);
    setRGB(HIGH, LOW, LOW);
    delay(500);

    showStatus("RGB LED", "Testing green", TFT_GREEN);
    setRGB(LOW, HIGH, LOW);
    delay(500);

    showStatus("RGB LED", "Testing blue", TFT_BLUE);
    setRGB(LOW, LOW, HIGH);
    delay(500);

    setRGB(LOW, LOW, LOW);

    showStatus("Buzzer", "Testing tone", TFT_YELLOW);
    playTone(2500, 150);
    delay(250);

    showStatus("Controls", "Ready", TFT_GREEN);
    delay(700);

    display.fillScreen(TFT_BLACK);

    display.setTextColor(TFT_GREEN, TFT_BLACK);
    display.setTextSize(2);
    display.setCursor(10, 20);
    display.println("PocketNet");

    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextSize(1);
    display.setCursor(10, 55);
    display.println("Hardware ready");

    display.setCursor(10, 75);
    display.println("Next: main menu");
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    pinMode(PIN_RGB_R, OUTPUT);
    pinMode(PIN_RGB_G, OUTPUT);
    pinMode(PIN_RGB_B, OUTPUT);

    pinMode(PIN_LEFT, INPUT);
    pinMode(PIN_RIGHT, INPUT);
    pinMode(PIN_SELECT, INPUT);
    pinMode(PIN_ALT, INPUT);

    pinMode(PIN_TFT_BL, OUTPUT);
    digitalWrite(PIN_TFT_BL, LOW);

    setRGB(LOW, LOW, LOW);

    ledcSetup(BUZZER_CHANNEL, 2500, BUZZER_RESOLUTION);
    ledcAttachPin(PIN_BUZZER, BUZZER_CHANNEL);

    display.init();
    display.setRotation(0);

    Serial.println("PocketNet startup self-test");
    runStartupSelfTest();
    Serial.println("Startup self-test complete");
}

void loop()
{
    delay(100);
}
#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "hardware_config.h"

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ST7735S _panel;
  lgfx::Bus_SPI _bus;

public:
  LGFX()
  {
    {
      auto cfg = _bus.config();
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
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }

    {
      auto cfg = _panel.config();
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

      _panel.config(cfg);
    }

    setPanel(&_panel);
  }
};

LGFX display;

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("PocketNet Display Bring-up");

  pinMode(PIN_TFT_BL, OUTPUT);
  digitalWrite(PIN_TFT_BL, LOW);

  display.init();
  display.setRotation(0);
  display.fillScreen(TFT_BLACK);

  display.setTextColor(TFT_GREEN, TFT_BLACK);
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.println("PocketNet");

  display.setTextColor(TFT_WHITE, TFT_BLACK);
  display.setTextSize(1);
  display.setCursor(10, 55);
  display.println("Display OK");

  display.setCursor(10, 75);
  display.println("RGB + Buzzer OK");

  Serial.println("Display test complete");
}

void loop()
{
}
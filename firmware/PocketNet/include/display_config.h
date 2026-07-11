#pragma once

#include <LovyanGFX.hpp>
#include "hardware_config.h"

class PocketNetDisplay : public lgfx::LGFX_Device
{
private:
    lgfx::Panel_ST7735S panel;
    lgfx::Bus_SPI bus;

public:
    PocketNetDisplay()
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
#include <Arduino.h>
#include <LovyanGFX.hpp>
#include "hardware_config.h"

// ============================================================
// Display configuration
// ============================================================

class LGFX : public lgfx::LGFX_Device
{
private:
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

// ============================================================
// Buzzer and RGB LED
// ============================================================

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

// ============================================================
// Button handling
// ============================================================

struct Button
{
    uint8_t pin;
    bool previousState;
    unsigned long lastChangeTime;
};

Button leftButton   = {PIN_LEFT, false, 0};
Button rightButton  = {PIN_RIGHT, false, 0};
Button selectButton = {PIN_SELECT, false, 0};
Button backButton   = {PIN_ALT, false, 0};

constexpr unsigned long DEBOUNCE_MS = 40;

bool wasPressed(Button& button)
{
    const bool currentState = digitalRead(button.pin);
    const unsigned long now = millis();

    bool pressed = false;

    if (currentState != button.previousState &&
        now - button.lastChangeTime >= DEBOUNCE_MS)
    {
        button.lastChangeTime = now;

        if (currentState)
        {
            pressed = true;
        }

        button.previousState = currentState;
    }

    return pressed;
}

// ============================================================
// Menu
// ============================================================

const char* menuItems[] =
{
    "WiFi Scan",
    "Signal Meter",
    "BLE Scan",
    "Network Tools",
    "Settings"
};

constexpr int MENU_ITEM_COUNT =
    sizeof(menuItems) / sizeof(menuItems[0]);

int selectedItem = 0;
bool showingFeaturePage = false;

void drawHeader()
{
    display.fillScreen(TFT_BLACK);

    display.setTextColor(TFT_GREEN, TFT_BLACK);
    display.setTextSize(2);
    display.setCursor(8, 6);
    display.println("PocketNet");

    display.drawFastHLine(6, 29, 116, TFT_DARKGREY);
}

void drawMenu()
{
    showingFeaturePage = false;
    drawHeader();

    display.setTextSize(1);

    for (int i = 0; i < MENU_ITEM_COUNT; i++)
    {
        const int y = 38 + (i * 17);

        if (i == selectedItem)
        {
            display.fillRoundRect(5, y - 3, 118, 15, 3, TFT_GREEN);
            display.setTextColor(TFT_BLACK, TFT_GREEN);
            display.setCursor(10, y);
            display.print("> ");
            display.println(menuItems[i]);
        }
        else
        {
            display.setTextColor(TFT_WHITE, TFT_BLACK);
            display.setCursor(10, y);
            display.print("  ");
            display.println(menuItems[i]);
        }
    }

    Serial.print("Selected menu item: ");
    Serial.println(menuItems[selectedItem]);
}

void showFeaturePage()
{
    showingFeaturePage = true;
    drawHeader();

    display.setTextColor(TFT_CYAN, TFT_BLACK);
    display.setTextSize(1);
    display.setCursor(8, 43);
    display.println(menuItems[selectedItem]);

    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setCursor(8, 65);
    display.println("Feature coming next.");

    display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    display.setCursor(8, 100);
    display.println("Press BUTTON");
    display.setCursor(8, 112);
    display.println("to go back.");

    Serial.print("Opened: ");
    Serial.println(menuItems[selectedItem]);

    setRGB(LOW, HIGH, LOW);
    playTone(2400, 70);
    setRGB(LOW, LOW, LOW);
}

void moveSelection(int direction)
{
    selectedItem += direction;

    if (selectedItem < 0)
    {
        selectedItem = MENU_ITEM_COUNT - 1;
    }

    if (selectedItem >= MENU_ITEM_COUNT)
    {
        selectedItem = 0;
    }

    playTone(1800, 25);
    drawMenu();
}

// ============================================================
// Startup
// ============================================================

void showStartupScreen()
{
    display.fillScreen(TFT_BLACK);

    display.setTextColor(TFT_GREEN, TFT_BLACK);
    display.setTextSize(2);
    display.setCursor(10, 28);
    display.println("PocketNet");

    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setTextSize(1);
    display.setCursor(22, 60);
    display.println("Network Toolkit");

    display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    display.setCursor(36, 82);
    display.println("v0.1.0");

    setRGB(LOW, HIGH, LOW);
    playTone(2200, 80);
    delay(80);
    playTone(2800, 100);

    delay(1000);
    setRGB(LOW, LOW, LOW);
}

// ============================================================
// Arduino lifecycle
// ============================================================

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

    Serial.println();
    Serial.println("PocketNet interactive menu starting");

    showStartupScreen();
    drawMenu();
}

void loop()
{
    if (!showingFeaturePage)
    {
        if (wasPressed(leftButton))
        {
            moveSelection(-1);
        }

        if (wasPressed(rightButton))
        {
            moveSelection(1);
        }

        if (wasPressed(selectButton))
        {
            showFeaturePage();
        }
    }
    else
    {
        if (wasPressed(backButton))
        {
            playTone(1400, 50);
            drawMenu();
        }
    }

    delay(5);
}
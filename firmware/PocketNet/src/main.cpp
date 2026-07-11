#include <Arduino.h>
#include <WiFi.h>
#include <cstring>

#include "hardware_config.h"
#include "display_config.h"
#include "hardware_io.h"
#include "pocketnet_input.h"

using namespace HardwareIO;

// ============================================================
// Display configuration
// ============================================================

PocketNetDisplay display;

// ============================================================
// Application state
// ============================================================

enum class Screen
{
    MainMenu,
    Placeholder,
    WiFiResults,
    SignalMeter,
    NoSignalTarget
};

Screen currentScreen = Screen::MainMenu;

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

int selectedMenuItem = 0;

int wifiNetworkCount = 0;
int selectedNetwork = 0;

// Saved access-point target
bool signalTargetValid = false;
uint8_t signalTargetBSSID[6] = {0};
String signalTargetSSID;
int32_t signalTargetChannel = 0;
int32_t signalTargetRSSI = -127;
wifi_auth_mode_t signalTargetSecurity = WIFI_AUTH_OPEN;

unsigned long lastSignalRefresh = 0;
constexpr unsigned long SIGNAL_REFRESH_INTERVAL_MS = 2000;

// ============================================================
// Shared display functions
// ============================================================

void drawHeader()
{
    display.fillScreen(TFT_BLACK);

    display.setTextColor(TFT_GREEN, TFT_BLACK);
    display.setTextSize(2);
    display.setCursor(8, 6);
    display.println("PocketNet");

    display.drawFastHLine(6, 29, 116, TFT_DARKGREY);
}

String shortenedSSID(String ssid, size_t maximumLength = 18)
{
    if (ssid.length() == 0)
    {
        return "<hidden>";
    }

    if (ssid.length() > maximumLength)
    {
        return ssid.substring(0, maximumLength);
    }

    return ssid;
}

const char* securityLabel(wifi_auth_mode_t security)
{
    switch (security)
    {
        case WIFI_AUTH_OPEN:
            return "Open";

        case WIFI_AUTH_WEP:
            return "WEP";

        case WIFI_AUTH_WPA_PSK:
            return "WPA";

        case WIFI_AUTH_WPA2_PSK:
            return "WPA2";

        case WIFI_AUTH_WPA_WPA2_PSK:
            return "WPA/WPA2";

        case WIFI_AUTH_WPA2_ENTERPRISE:
            return "WPA2-Ent";

        case WIFI_AUTH_WPA3_PSK:
            return "WPA3";

        case WIFI_AUTH_WPA2_WPA3_PSK:
            return "WPA2/WPA3";

        default:
            return "Unknown";
    }
}

const char* signalQuality(int32_t rssi)
{
    if (rssi >= -50)
    {
        return "Excellent";
    }

    if (rssi >= -60)
    {
        return "Good";
    }

    if (rssi >= -70)
    {
        return "Fair";
    }

    return "Weak";
}

uint16_t signalColour(int32_t rssi)
{
    if (rssi >= -60)
    {
        return TFT_GREEN;
    }

    if (rssi >= -70)
    {
        return TFT_YELLOW;
    }

    return TFT_RED;
}

void setSignalLED(int32_t rssi)
{
    if (rssi >= -60)
    {
        setRGB(LOW, HIGH, LOW);
    }
    else if (rssi >= -70)
    {
        setRGB(HIGH, HIGH, LOW);
    }
    else
    {
        setRGB(HIGH, LOW, LOW);
    }
}

void drawSignalBar(int32_t rssi, int y)
{
    int fillWidth;

    if (rssi >= -50)
    {
        fillWidth = 100;
    }
    else if (rssi <= -100)
    {
        fillWidth = 2;
    }
    else
    {
        fillWidth = map(rssi, -100, -50, 2, 100);
    }

    display.drawRect(12, y, 104, 10, TFT_WHITE);
    display.fillRect(
        14,
        y + 2,
        fillWidth,
        6,
        signalColour(rssi)
    );
}

String formatBSSID(const uint8_t* bssid)
{
    char text[18];

    snprintf(
        text,
        sizeof(text),
        "%02X:%02X:%02X:%02X:%02X:%02X",
        bssid[0],
        bssid[1],
        bssid[2],
        bssid[3],
        bssid[4],
        bssid[5]
    );

    return String(text);
}

// ============================================================
// Main menu
// ============================================================

void drawMainMenu()
{
    currentScreen = Screen::MainMenu;
    setRGB(LOW, LOW, LOW);

    drawHeader();
    display.setTextSize(1);

    for (int i = 0; i < MENU_ITEM_COUNT; i++)
    {
        const int y = 38 + (i * 17);

        if (i == selectedMenuItem)
        {
            display.fillRoundRect(
                5,
                y - 3,
                118,
                15,
                3,
                TFT_GREEN
            );

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
}

void drawPlaceholder()
{
    currentScreen = Screen::Placeholder;
    setRGB(LOW, LOW, LOW);

    drawHeader();

    display.setTextSize(1);
    display.setTextColor(TFT_CYAN, TFT_BLACK);
    display.setCursor(8, 43);
    display.println(menuItems[selectedMenuItem]);

    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setCursor(8, 65);
    display.println("Coming soon.");

    display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    display.setCursor(8, 101);
    display.println("BUTTON: Back");
}

void drawNoSignalTarget()
{
    currentScreen = Screen::NoSignalTarget;
    setRGB(HIGH, HIGH, LOW);

    drawHeader();

    display.setTextSize(1);
    display.setTextColor(TFT_YELLOW, TFT_BLACK);
    display.setCursor(8, 43);
    display.println("Signal Meter");

    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setCursor(8, 63);
    display.println("No AP selected.");

    display.setCursor(8, 78);
    display.println("Use WiFi Scan");

    display.setCursor(8, 90);
    display.println("and select one.");

    display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    display.setCursor(8, 112);
    display.println("BUTTON: Back");
}

// ============================================================
// Wi-Fi scanner
// ============================================================

void drawWiFiResult()
{
    currentScreen = Screen::WiFiResults;
    setRGB(LOW, LOW, LOW);

    drawHeader();
    display.setTextSize(1);

    if (wifiNetworkCount <= 0)
    {
        display.setTextColor(TFT_RED, TFT_BLACK);
        display.setCursor(8, 48);
        display.println("No networks found");

        display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        display.setCursor(8, 88);
        display.println("SELECT: Rescan");
        display.setCursor(8, 104);
        display.println("BUTTON: Back");

        return;
    }

    const String ssid =
        shortenedSSID(WiFi.SSID(selectedNetwork));

    const int32_t rssi =
        WiFi.RSSI(selectedNetwork);

    const int32_t channel =
        WiFi.channel(selectedNetwork);

    const wifi_auth_mode_t security =
        WiFi.encryptionType(selectedNetwork);

    display.setTextColor(TFT_CYAN, TFT_BLACK);
    display.setCursor(8, 37);
    display.printf(
        "%d/%d  CH %ld",
        selectedNetwork + 1,
        wifiNetworkCount,
        channel
    );

    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setCursor(8, 53);
    display.println(ssid);

    display.setCursor(8, 68);
    display.printf(
        "%ld dBm  %s",
        rssi,
        signalQuality(rssi)
    );

    display.setCursor(8, 81);
    display.print("Security: ");
    display.println(securityLabel(security));

    drawSignalBar(rssi, 93);

    display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    display.setCursor(8, 111);
    display.println("< >  SELECT: Meter");

    Serial.printf(
        "Network %d/%d: SSID=%s RSSI=%ld CH=%ld Security=%s\n",
        selectedNetwork + 1,
        wifiNetworkCount,
        ssid.c_str(),
        rssi,
        channel,
        securityLabel(security)
    );
}

void runWiFiScan()
{
    currentScreen = Screen::WiFiResults;

    drawHeader();

    display.setTextSize(1);
    display.setTextColor(TFT_YELLOW, TFT_BLACK);
    display.setCursor(8, 50);
    display.println("Scanning WiFi...");

    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setCursor(8, 70);
    display.println("Please wait");

    setRGB(HIGH, HIGH, LOW);

    Serial.println("Starting WiFi scan");

    WiFi.scanDelete();

    wifiNetworkCount =
        WiFi.scanNetworks(false, true);

    selectedNetwork = 0;

    setRGB(LOW, LOW, LOW);

    Serial.printf(
        "WiFi scan complete: %d networks found\n",
        wifiNetworkCount
    );

    if (wifiNetworkCount > 0)
    {
        playTone(2600, 80);
    }
    else
    {
        playTone(900, 150);
    }

    drawWiFiResult();
}

void saveSelectedSignalTarget()
{
    if (wifiNetworkCount <= 0)
    {
        return;
    }

    const uint8_t* bssid =
        WiFi.BSSID(selectedNetwork);

    if (bssid == nullptr)
    {
        signalTargetValid = false;
        return;
    }

    memcpy(
        signalTargetBSSID,
        bssid,
        sizeof(signalTargetBSSID)
    );

    signalTargetSSID =
        WiFi.SSID(selectedNetwork);

    signalTargetChannel =
        WiFi.channel(selectedNetwork);

    signalTargetRSSI =
        WiFi.RSSI(selectedNetwork);

    signalTargetSecurity =
        WiFi.encryptionType(selectedNetwork);

    signalTargetValid = true;

    Serial.print("Signal target selected: ");
    Serial.print(signalTargetSSID);
    Serial.print(" ");
    Serial.println(formatBSSID(signalTargetBSSID));
}

// ============================================================
// Signal Meter
// ============================================================

int findSignalTargetInScan()
{
    if (!signalTargetValid)
    {
        return -1;
    }

    for (int i = 0; i < wifiNetworkCount; i++)
    {
        const uint8_t* candidateBSSID =
            WiFi.BSSID(i);

        if (candidateBSSID == nullptr)
        {
            continue;
        }

        if (memcmp(
                candidateBSSID,
                signalTargetBSSID,
                sizeof(signalTargetBSSID)
            ) == 0)
        {
            return i;
        }
    }

    return -1;
}

void drawSignalMeter(bool found)
{
    currentScreen = Screen::SignalMeter;
    drawHeader();

    display.setTextSize(1);

    display.setTextColor(TFT_CYAN, TFT_BLACK);
    display.setCursor(8, 36);
    display.println(
        shortenedSSID(signalTargetSSID, 17)
    );

    display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    display.setCursor(8, 49);
    display.println(
        formatBSSID(signalTargetBSSID)
    );

    if (!found)
    {
        setRGB(HIGH, LOW, LOW);

        display.setTextColor(TFT_RED, TFT_BLACK);
        display.setCursor(8, 70);
        display.println("AP not detected");

        display.setTextColor(TFT_WHITE, TFT_BLACK);
        display.setCursor(8, 85);
        display.println("Refreshing...");

        display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        display.setCursor(8, 112);
        display.println("BUTTON: Back");

        return;
    }

    setSignalLED(signalTargetRSSI);

    display.setTextColor(
        signalColour(signalTargetRSSI),
        TFT_BLACK
    );

    display.setTextSize(2);
    display.setCursor(8, 64);
    display.printf("%ld", signalTargetRSSI);

    display.setTextSize(1);
    display.setTextColor(TFT_WHITE, TFT_BLACK);
    display.setCursor(72, 71);
    display.println("dBm");

    display.setCursor(8, 87);
    display.print(signalQuality(signalTargetRSSI));

    display.setCursor(70, 87);
    display.printf("CH %ld", signalTargetChannel);

    drawSignalBar(signalTargetRSSI, 99);

    display.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    display.setCursor(8, 114);
    display.println("Auto refresh  BTN Back");
}

void refreshSignalMeter()
{
    if (!signalTargetValid)
    {
        drawNoSignalTarget();
        return;
    }

    currentScreen = Screen::SignalMeter;

    display.fillRect(0, 60, 128, 68, TFT_BLACK);
    display.setTextSize(1);
    display.setTextColor(TFT_YELLOW, TFT_BLACK);
    display.setCursor(8, 73);
    display.println("Refreshing...");

    Serial.print("Refreshing signal meter for ");
    Serial.println(signalTargetSSID);

    WiFi.scanDelete();

    wifiNetworkCount =
        WiFi.scanNetworks(false, true);

    const int targetIndex =
        findSignalTargetInScan();

    if (targetIndex >= 0)
    {
        selectedNetwork = targetIndex;

        signalTargetRSSI =
            WiFi.RSSI(targetIndex);

        signalTargetChannel =
            WiFi.channel(targetIndex);

        signalTargetSecurity =
            WiFi.encryptionType(targetIndex);

        Serial.printf(
            "Target found: RSSI=%ld dBm CH=%ld\n",
            signalTargetRSSI,
            signalTargetChannel
        );

        drawSignalMeter(true);
    }
    else
    {
        Serial.println("Target was not detected");
        drawSignalMeter(false);
    }

    lastSignalRefresh = millis();
}

void openSignalMeter()
{
    if (!signalTargetValid)
    {
        drawNoSignalTarget();
        return;
    }

    lastSignalRefresh = 0;
    drawSignalMeter(true);
}

// ============================================================
// Navigation
// ============================================================

void moveMenuSelection(int direction)
{
    selectedMenuItem += direction;

    if (selectedMenuItem < 0)
    {
        selectedMenuItem =
            MENU_ITEM_COUNT - 1;
    }

    if (selectedMenuItem >= MENU_ITEM_COUNT)
    {
        selectedMenuItem = 0;
    }

    playTone(1800, 20);
    drawMainMenu();
}

void moveWiFiSelection(int direction)
{
    if (wifiNetworkCount <= 0)
    {
        return;
    }

    selectedNetwork += direction;

    if (selectedNetwork < 0)
    {
        selectedNetwork =
            wifiNetworkCount - 1;
    }

    if (selectedNetwork >= wifiNetworkCount)
    {
        selectedNetwork = 0;
    }

    playTone(1800, 20);
    drawWiFiResult();
}

void openSelectedMenuItem()
{
    playTone(2400, 60);

    switch (selectedMenuItem)
    {
        case 0:
            runWiFiScan();
            break;

        case 1:
            openSignalMeter();
            break;

        default:
            drawPlaceholder();
            break;
    }
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
    display.println("v0.3.0");

    setRGB(LOW, HIGH, LOW);

    playTone(2200, 80);
    delay(80);
    playTone(2800, 100);

    delay(800);

    setRGB(LOW, LOW, LOW);
}

// ============================================================
// Arduino lifecycle
// ============================================================

void setup()
{
    Serial.begin(115200);
    delay(500);

    Input::begin();

    pinMode(PIN_TFT_BL, OUTPUT);
    digitalWrite(PIN_TFT_BL, LOW);


    HardwareIO::begin();

    display.init();
    display.setRotation(0);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    Serial.println();
    Serial.println(
        "PocketNet signal meter starting"
    );

    showStartupScreen();
    drawMainMenu();
}

void loop()
{
    Input::update();

    switch (currentScreen)
    {
        case Screen::MainMenu:
            if (Input::wasPressed(Input::Button::Left))
            {
                moveMenuSelection(-1);
            }

            if (Input::wasPressed(Input::Button::Right))
            {
                moveMenuSelection(1);
            }

            if (Input::wasPressed(Input::Button::Select))
            {
                openSelectedMenuItem();
            }

            break;

        case Screen::WiFiResults:
            if (Input::wasPressed(Input::Button::Left))
            {
                moveWiFiSelection(-1);
            }

            if (Input::wasPressed(Input::Button::Right))
            {
                moveWiFiSelection(1);
            }

            if (Input::wasPressed(Input::Button::Select))
            {
                saveSelectedSignalTarget();

                if (signalTargetValid)
                {
                    playTone(2400, 60);
                    openSignalMeter();
                }
            }

            if (Input::wasPressed(Input::Button::Back))
            {
                playTone(1400, 40);
                drawMainMenu();
            }

            break;

        case Screen::SignalMeter:
            if (
                Input::wasPressed(Input::Button::Left) ||
                Input::wasPressed(Input::Button::Right)
            )
            {
                playTone(1600, 30);
                drawWiFiResult();
                break;
            }

            if (Input::wasPressed(Input::Button::Select))
            {
                refreshSignalMeter();
                break;
            }

            if (Input::wasPressed(Input::Button::Back))
            {
                playTone(1400, 40);
                drawWiFiResult();
                break;
            }

            if (
                millis() - lastSignalRefresh >=
                SIGNAL_REFRESH_INTERVAL_MS
            )
            {
                refreshSignalMeter();
            }

            break;

        case Screen::NoSignalTarget:
            if (Input::wasPressed(Input::Button::Back))
            {
                setRGB(LOW, LOW, LOW);
                playTone(1400, 40);
                drawMainMenu();
            }

            break;

        case Screen::Placeholder:
            if (Input::wasPressed(Input::Button::Back))
            {
                playTone(1400, 40);
                drawMainMenu();
            }

            break;
    }

    delay(5);
}
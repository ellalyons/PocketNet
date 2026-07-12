#include "ble_scanner.h"

#include <NimBLEDevice.h>

namespace
{
    bool initialised = false;
    std::vector<BLEScanner::Device> devices;
}

namespace BLEScanner
{
    void begin()
    {
        if (initialised)
        {
            return;
        }

        NimBLEDevice::init("PocketNet");

        NimBLEScan* scanner = NimBLEDevice::getScan();

        // Active scanning requests additional scan-response data,
        // which may reveal device names not present in the first packet.
        scanner->setActiveScan(true);

        scanner->setInterval(100);
        scanner->setWindow(80);

        // Store up to 50 results to protect available RAM.
        scanner->setMaxResults(50);

        initialised = true;
    }

    bool scan(uint32_t durationMs)
    {
        begin();

        devices.clear();

        NimBLEScan* scanner = NimBLEDevice::getScan();

        scanner->stop();
        scanner->clearResults();

        Serial.println("Starting BLE scan");

        NimBLEScanResults results =
            scanner->getResults(durationMs, false);

        const int resultCount = results.getCount();

        Serial.printf(
            "BLE scan complete: %d devices found\n",
            resultCount
        );

        devices.reserve(resultCount);

        for (int i = 0; i < resultCount; i++)
        {
            const NimBLEAdvertisedDevice* advertisedDevice =
                results.getDevice(i);

            if (advertisedDevice == nullptr)
            {
                continue;
            }

            Device device;

            const std::string advertisedName =
                advertisedDevice->getName();

            if (advertisedName.empty())
            {
                device.name = "<unnamed>";
            }
            else
            {
                device.name = advertisedName.c_str();
            }

            device.address =
                advertisedDevice->getAddress().toString().c_str();

            device.rssi =
                advertisedDevice->getRSSI();

            device.connectable =
                advertisedDevice->isConnectable();

            devices.push_back(device);

            Serial.printf(
                "BLE: %s | %s | %d dBm | %s\n",
                device.name.c_str(),
                device.address.c_str(),
                device.rssi,
                device.connectable
                    ? "Connectable"
                    : "Beacon"
            );
        }

        scanner->clearResults();

        return !devices.empty();
    }

    int count()
    {
        return static_cast<int>(devices.size());
    }

    const Device* getDevice(int index)
    {
        if (index < 0 || index >= count())
        {
            return nullptr;
        }

        return &devices[index];
    }

    void clear()
    {
        devices.clear();

        if (initialised)
        {
            NimBLEDevice::getScan()->clearResults();
        }
    }
}
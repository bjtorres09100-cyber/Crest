#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "hal.h"
#include "menu.h"

extern SystemState currentState;
extern void drawTextLine(int row, const char* text, bool inverted);
extern void clearCanvas();
extern void renderCanvas();
extern void initMenu();

// Fixed: Changed type to match the modern framework class name
BLEScan* pBLEScan; 
bool bleInitialized = false;

// Custom callback class that executes every time a BLE packet is captured in the air
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        char buffer[32];
        String name = advertisedDevice.getName().c_str();
        String address = advertisedDevice.getAddress().toString().c_str();
        int rssi = advertisedDevice.getRSSI();

        clearCanvas();
        drawTextLine(0, "[ PASSIVE BLE SCAN ]", false);
        
        // Display detected device metadata
        if (name.length() > 0) {
            snprintf(buffer, sizeof(buffer), "Dev: %s", name.substring(0, 14).c_str());
        } else {
            snprintf(buffer, sizeof(buffer), "Dev: [Unknown]");
        }
        drawTextLine(1, buffer, false);

        snprintf(buffer, sizeof(buffer), "MAC: %s", address.substring(0, 17).c_str());
        drawTextLine(2, buffer, false);

        snprintf(buffer, sizeof(buffer), "RSSI: %d dBm", rssi);
        drawTextLine(3, buffer, false);
        
        drawTextLine(5, "Hold SEL to Exit", false);
        renderCanvas();
    }
};

void initBLEScanner() {
    clearCanvas();
    drawTextLine(0, "[ PASSIVE BLE SCAN ]", false);
    drawTextLine(2, "Initializing Radio...", false);
    renderCanvas();

    BLEDevice::init("");
    // Fixed: Using the modern framework method getScan() instead of getBLEScan()
    pBLEScan = BLEDevice::getScan(); 
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(false); // Passive scanning is completely silent
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    bleInitialized = true;
}

void loopBleApp() {
    if (!bleInitialized) {
        initBLEScanner();
    }

    // Start a short synchronous scan window
    pBLEScan->start(1, false);
    pBLEScan->clearResults(); // Clear buffer memory to prevent leaks

    if (digitalRead(BTN_SEL) == LOW) {
        delay(200);
        // Deconstruct BLE parameters to return RAM back to the main OS
        BLEDevice::deinit(false);
        bleInitialized = false;
        currentState = STATE_MENU;
        initMenu();
    }
}

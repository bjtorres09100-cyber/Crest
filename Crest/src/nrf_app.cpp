#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "nrf_app.h"
#include "hal.h"
#include "menu.h"

extern SystemState currentState;
extern Adafruit_SSD1306 display;
extern void initMenu();
extern void clearCanvas();
extern void renderCanvas();

// Use the core system SPI instance declared in main.cpp
extern RF24 radio;

enum NrfSubState { NRF_SUBMENU, RUN_SPECTRUM, RUN_JAMMER, RUN_MOUSEJACK };
static NrfSubState nrfState = NRF_SUBMENU;

static const char* nrfMenuItems[] = {
    "1. Spectrum Analyzer",
    "2. RF/BLE Jammer",
    "3. MouseJack Sniffer"
};
static int nrfMenuIndex = 0;
static const int nrfTotalItems = 3;

// ==================================================================
// SYSTEM SUB-MENU NAVIGATION
// ==================================================================
void drawNrfSubMenu() {
    clearCanvas();
    display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(4, 2);
    display.print("NRF24L01+ SUITE");

    display.setTextColor(SSD1306_WHITE);
    for (int i = 0; i < nrfTotalItems; i++) {
        int yPos = 16 + (i * 14);
        if (i == nrfMenuIndex) {
            display.fillRect(2, yPos, 124, 12, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
        } else {
            display.setTextColor(SSD1306_WHITE);
        }
        display.setCursor(6, yPos + 2);
        display.print(nrfMenuItems[i]);
    }
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(2, 56);
    display.print("[Hold LEFT to Main Menu]");
    renderCanvas();
}

void loopNrfApp() {
    switch (nrfState) {
        case NRF_SUBMENU:
            if (digitalRead(BTN_UP) == LOW) {
                nrfMenuIndex = (nrfMenuIndex - 1 + nrfTotalItems) % nrfTotalItems;
                drawNrfSubMenu();
                delay(200);
            }
            if (digitalRead(BTN_DOWN) == LOW) {
                nrfMenuIndex = (nrfMenuIndex + 1) % nrfTotalItems;
                drawNrfSubMenu();
                delay(200);
            }
            if (digitalRead(BTN_SEL) == LOW) {
                delay(200);
                if (nrfMenuIndex == 0) nrfState = RUN_SPECTRUM;
                if (nrfMenuIndex == 1) nrfState = RUN_JAMMER;
                if (nrfMenuIndex == 2) nrfState = RUN_MOUSEJACK;
                clearCanvas();
            }
            if (digitalRead(BTN_LEFT) == LOW) {
                delay(200);
                currentState = STATE_MENU;
                initMenu();
            }
            break;

        case RUN_SPECTRUM:
            runNrfSpectrum();
            break;

        case RUN_JAMMER:
            runNrfJammer();
            break;

        case RUN_MOUSEJACK:
            runMouseJack();
            break;
    }
}

// ==================================================================
// MODULE 1: 2.4GHz SPECTRUM ANALYZER ENGINE
// ==================================================================
static uint8_t specEnergy[64];
static uint8_t specPeaks[64];
static bool specSetup = false;

void runNrfSpectrum() {
    if (!specSetup) {
        radio.begin();
        radio.setAutoAck(false);
        radio.disableCRC();
        radio.setAddressWidth(2);
        radio.setDataRate(RF24_1MBPS);
        memset(specEnergy, 0, sizeof(specEnergy));
        memset(specPeaks, 0, sizeof(specPeaks));
        specSetup = false; // toggle active scanner loop flag
        specSetup = true;
    }

    digitalWrite(NRF24_CE, LOW);
    for (int i = 0; i < 64; i++) {
        int ch = (i * 80) / 64;
        radio.setChannel(ch);
        radio.startListening();
        delayMicroseconds(130);
        radio.stopListening();

        int rpd = radio.testRPD() ? 1 : 0;
        if (rpd) specEnergy[i] = (specEnergy[i] + 40) / 2;
        else specEnergy[i] = (specEnergy[i] * 3) / 4;

        if (specEnergy[i] > specPeaks[i]) specPeaks[i] = specEnergy[i];
        else if (specPeaks[i] > 0) specPeaks[i]--;
    }
    digitalWrite(NRF24_CE, HIGH);

    clearCanvas();
    display.fillRect(0, 0, 128, 11, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(4, 2);
    display.print("2.4GHZ REALTIME RF SCAN");

    for (int i = 0; i < 64; i++) {
        int x = i * 2;
        int barH = map(specEnergy[i], 0, 40, 0, 42);
        int pkH = map(specPeaks[i], 0, 40, 0, 42);

        if (barH > 0) display.fillRect(x, 54 - barH, 2, barH, SSD1306_WHITE);
        if (pkH > 0) display.drawPixel(x, 54 - pkH, SSD1306_WHITE);
    }
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 56); display.print("2.40G");
    display.setCursor(98, 56); display.print("2.48G");
    renderCanvas();

    if (digitalRead(BTN_SEL) == LOW) {
        delay(200);
        radio.stopListening();
        radio.powerDown();
        specSetup = false;
        nrfState = NRF_SUBMENU;
        drawNrfSubMenu();
    }
}

// ==================================================================
// MODULE 2: MULTI-BAND HOPPING RF JAMMER ENGINE
// ==================================================================
static bool jamSetup = false;
static int jamModeIdx = 0;
// Channels mapped out across Bluetooth, BLE advertising lanes, and WiFi cores
static uint8_t jamChannels[] = {2, 26, 80, 7, 12, 37, 62, 11, 22, 33, 44, 55, 66, 77};
static const int totalJamChannels = 14;

void runNrfJammer() {
    if (!jamSetup) {
        radio.begin();
        radio.setPALevel(RF24_PA_MAX);
        radio.setDataRate(RF24_2MBPS);
        radio.setAddressWidth(5);
        radio.setPayloadSize(2);
        radio.startConstCarrier(RF24_PA_MAX, jamChannels[0]);
        jamSetup = true;
    }

    // High speed channel hopping deployment sequence loop
    for (int i = 0; i < totalJamChannels; i++) {
        radio.stopConstCarrier();
        delayMicroseconds(100);
        radio.setChannel(jamChannels[i]);
        radio.startConstCarrier(RF24_PA_MAX, jamChannels[i]);
        delayMicroseconds(500); // Continuous wave burst interval window
    }

    clearCanvas();
    display.drawRoundRect(0, 0, 128, 64, 4, SSD1306_WHITE);
    display.setCursor(14, 12);
    display.setTextColor(SSD1306_WHITE);
    display.print(">> NRF24 JAMMER <<");
    display.setCursor(10, 32);
    display.print("FHSS-Spectrum Active");
    display.setCursor(12, 50);
    display.print("Press SEL to Terminate");
    renderCanvas();

    if (digitalRead(BTN_SEL) == LOW) {
        delay(200);
        radio.stopConstCarrier();
        radio.powerDown();
        jamSetup = false;
        nrfState = NRF_SUBMENU;
        drawNrfSubMenu();
    }
}

// ==================================================================
// MODULE 3: MOUSEJACK PASSIVE PACKET SNIFFER
// ==================================================================
static bool mjSetup = false;
static uint8_t currentMjChannel = 2;
static int detectedMiceCount = 0;

void runMouseJack() {
    if (!mjSetup) {
        radio.begin();
        radio.setAutoAck(false);
        radio.disableCRC();
        radio.setAddressWidth(2);
        radio.setDataRate(RF24_2MBPS);
        radio.setPayloadSize(32);
        
        // Open reading pipes on target alignment preambles used by wireless peripherals
        const uint8_t noiseAddresses[][2] = {
            {0x55, 0x55}, {0xAA, 0xAA}, {0xA0, 0xAA}, {0xAB, 0xAA}
        };
        for (uint8_t i = 0; i < 4; i++) {
            radio.openReadingPipe(i, noiseAddresses[i]);
        }
        mjSetup = true;
    }

    // Rotate across processing channels sequentially to sniff packets
    radio.setChannel(currentMjChannel);
    radio.startListening();
    
    for (int i = 0; i < 10; i++) {
        delayMicroseconds(200);
        if (radio.available()) {
            uint8_t rxBuffer[32];
            radio.read(rxBuffer, sizeof(rxBuffer));
            
            // Analyze Enhanced ShockBurst payloads for structural device signatures
            // Check for unencrypted Microsoft protocol payload sizes
            if (rxBuffer[5] >> 2 == 19 && rxBuffer[6] == 0x08) {
                detectedMiceCount++;
            }
            // Check for Logitech native device layout preambles
            if (rxBuffer[0] == 0x00 && rxBuffer[1] == 0xC2) {
                detectedMiceCount++;
            }
        }
    }
    radio.stopListening();

    currentMjChannel++;
    if (currentMjChannel > 84) currentMjChannel = 2; // Keep loop restricted within safe ESB bands

    clearCanvas();
    display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(4, 2);
    display.print("MOUSEJACK DISCOVERY");

    display.setTextColor(SSD1306_WHITE);
    display.setCursor(6, 20);
    display.printf("Sweeping CH: %d", currentMjChannel);
    display.setCursor(6, 36);
    display.printf("Vulnerable Devs: %d", detectedMiceCount);
    display.setCursor(6, 52);
    display.print("Press SEL to Abort");
    renderCanvas();

    if (digitalRead(BTN_SEL) == LOW) {
        delay(200);
        radio.powerDown();
        mjSetup = false;
        nrfState = NRF_SUBMENU;
        drawNrfSubMenu();
    }
}

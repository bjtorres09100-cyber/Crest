#include <Arduino.h>
#include <RadioLib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "hal.h"
#include "menu.h"
#include "subghz_app.h"

// Connect CC1101 GDO0 to GPIO 17 for the packet-ready interrupt.
static CC1101 radio = new Module(CC1101_CS, CC1101_GDO0, RADIOLIB_NC, RADIOLIB_NC);
static bool radioReady = false;
static bool receiving = false;
static volatile bool packetReceived = false;
static char lastMessage[61] = "Waiting for packet...";
static uint32_t lastDraw = 0;
extern SystemState currentState;
extern void initMenu();
extern void clearCanvas();
extern void renderCanvas();

static void onPacketReceived() {
    packetReceived = true;
}

static void drawSubGhzScreen(const char* status) {
    clearCanvas();
    extern Adafruit_SSD1306 display;
    display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(4, 2);
    display.print("CC1101 PACKET LINK");
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(2, 17);
    display.print(status);
    display.setCursor(2, 30);
    display.print("RX: ");
    display.setCursor(2, 40);
    display.print(lastMessage);
    display.setCursor(2, 56);
    display.print("Serial: type text + Enter");
    renderCanvas();
}

void initSubGhzApp() {
    // SPI is initialized by main.cpp before this call.
    int16_t state = radio.begin(433.92);
    radioReady = (state == RADIOLIB_ERR_NONE);
    if (radioReady) {
        radio.setPacketReceivedAction(onPacketReceived);
        radio.setOutputPower(10);  // dBm; conservative default for a short-range demo
        radio.setCrcFiltering(true);
    }
}

void loopSubGhzApp() {
    if (!radioReady) {
        drawSubGhzScreen("Radio init failed; check wiring");
        if (digitalRead(BTN_LEFT) == LOW) {
            delay(200);
            currentState = STATE_MENU;
            initMenu();
        }
        return;
    }

    if (!receiving) {
        radio.startReceive();
        receiving = true;
    }

    // Send any line entered in the PlatformIO/USB serial monitor.
    if (Serial.available()) {
        String message = Serial.readStringUntil('\n');
        message.trim();
        if (message.length() > 0) {
            radio.standby();
            receiving = false;
            int16_t result = radio.transmit(message);
            Serial.printf("CC1101 TX %s (%d)\r\n", result == RADIOLIB_ERR_NONE ? "OK" : "FAILED", result);
            strncpy(lastMessage, message.c_str(), sizeof(lastMessage) - 1);
            lastMessage[sizeof(lastMessage) - 1] = '\0';
            radio.startReceive();
            receiving = true;
        }
    }

    // RadioLib returns a nonzero packet length once a packet has arrived.
    if (packetReceived) {
        packetReceived = false;
        size_t packetLength = radio.getPacketLength();
        uint8_t packet[61] = {};
        size_t bytesToRead = packetLength > sizeof(packet) - 1 ? sizeof(packet) - 1 : packetLength;
        int16_t result = bytesToRead ? radio.readData(packet, bytesToRead) : RADIOLIB_ERR_UNKNOWN;
        if (result == RADIOLIB_ERR_NONE) {
            packet[bytesToRead] = '\0';
            strncpy(lastMessage, reinterpret_cast<char*>(packet), sizeof(lastMessage) - 1);
            lastMessage[sizeof(lastMessage) - 1] = '\0';
            Serial.printf("CC1101 RX: %s\r\n", lastMessage);
        }
        receiving = false;
    }

    if (millis() - lastDraw > 250) {
        drawSubGhzScreen("433.92 MHz | 10 dBm");
        lastDraw = millis();
    }

    if (digitalRead(BTN_LEFT) == LOW) {
        delay(200);
        radio.standby();
        receiving = false;
        currentState = STATE_MENU;
        initMenu();
    }
}

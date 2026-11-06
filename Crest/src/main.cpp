#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <RF24.h>
#include "hal.h"
#include "menu.h"
#include "wifi_app.h"
#include "nrf_app.h"
#include "ir_app.h"
#include "subghz_app.h"

// Forward declare display management handlers
extern void initDisplay();
extern void drawSplashScreen();
extern void loopBleApp();

// Instantiate the global radio object that src/nrf_app.cpp is searching for
RF24 radio(NRF24_CE, NRF24_CSN);

SystemState currentState = STATE_MENU;

void setup() {
    Serial.begin(115200);
    
    // Initialize All 5 Directional Buttons
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    pinMode(BTN_SEL, INPUT_PULLUP);
    
    initDisplay();
    
    // Run the Moon Crest Boot Screen immediately
    drawSplashScreen();
    
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    Wire.begin(I2C_SDA, I2C_SCL);
    
    pinMode(CC1101_CS, OUTPUT);
    pinMode(NRF24_CSN, OUTPUT);
    pinMode(LARGE_DISP_CS, OUTPUT);
    digitalWrite(CC1101_CS, HIGH);
    digitalWrite(NRF24_CSN, HIGH);
    digitalWrite(LARGE_DISP_CS, HIGH);
    
    initMenu();
    initSubGhzApp();
}

void loop() {
    switch (currentState) {
        case STATE_MENU:
            loopMenu();
            break;
        case STATE_WIFI:
            loopWifiApp();
            break;
        case STATE_NRF: 
            loopNrfApp();
            break;
        case STATE_SUBGHZ:
            loopSubGhzApp();
            break;
        case STATE_IR:
            loopIrApp();
            break;
        case STATE_BLE: 
            loopBleApp();
            break;
        default:
            break;
    }
    delay(10);
}

#pragma once

// --- Display Driver Configuration Toggle ---
#define USE_OLED_DISPLAY true

// --- 5-Directional Navigation Buttons ---
#define BTN_UP    34
#define BTN_DOWN  35
#define BTN_LEFT  32  // New Pin
#define BTN_RIGHT 33  // New Pin
#define BTN_SEL   0   // Center/Select Button

// --- Shared SPI Bus Layout ---
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK  18

// --- Shared I2C Bus Layout ---
#define I2C_SDA  21
#define I2C_SCL  22

// --- CC1101 Sub-GHz Radio Module ---
#define CC1101_CS 5

// --- NRF24L01 2.4GHz Radio Module ---
#define NRF24_CSN 4
#define NRF24_CE  16

// --- Infrared Subsystem Layout ---
#define IR_RX_PIN 15
#define IR_TX_PIN 13

// --- Large Display Dedicated Pins (Used if USE_OLED_DISPLAY is false) ---
#define LARGE_DISP_CS  14
#define LARGE_DISP_DC  27
#define LARGE_DISP_RST 26

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TFT_eSPI.h>
#include "hal.h"

// Define the display hardware instance globally so the compiler can link it
#if USE_OLED_DISPLAY
  Adafruit_SSD1306 display(128, 64, &Wire, -1);
#else
  TFT_eSPI display = TFT_eSPI();
#endif

void initDisplay() {
    #if USE_OLED_DISPLAY
        if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            for(;;); // Halt if OLED fails to allocate memory
        }
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.display();
    #else
        display.init();
        display.setRotation(1);
        display.fillScreen(TFT_BLACK);
    #endif
}

void clearCanvas() {
    #if USE_OLED_DISPLAY
        display.clearDisplay();
    #else
        display.fillScreen(TFT_BLACK);
    #endif
}

void renderCanvas() {
    #if USE_OLED_DISPLAY
        display.display();
    #endif
}

void drawTextLine(int row, const char* text, bool inverted) {
    #if USE_OLED_DISPLAY
        if (inverted) {
            display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        } else {
            display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
        }
        display.setCursor(0, row * 10);
        display.print(text);
    #else
        if (inverted) {
            display.setTextColor(TFT_BLACK, TFT_WHITE);
        } else {
            display.setTextColor(TFT_WHITE, TFT_BLACK);
        }
        display.drawString(text, 10, row * 20, 2);
    #endif
}

void drawSplashScreen() {
    clearCanvas();
    #if USE_OLED_DISPLAY
        display.fillCircle(64, 25, 20, SSD1306_WHITE);
        display.fillCircle(72, 21, 18, SSD1306_BLACK);
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(24, 52);
        display.print("LUNAR OS v1.0");
    #else
        display.fillCircle(120, 80, 50, TFT_WHITE);
        display.fillCircle(140, 70, 46, TFT_BLACK);
        display.setTextColor(TFT_WHITE);
        display.drawString("LUNAR OS v1.0", 75, 150, 4);
    #endif
    renderCanvas();
    delay(2000); 
}

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "menu.h"
#include "hal.h"

extern SystemState currentState;
extern void clearCanvas();
extern void renderCanvas();

#if USE_OLED_DISPLAY
  extern Adafruit_SSD1306 display;
#endif

const char* menuItems[] = {
    "Wi-Fi Suite",
    "NRF24 Suite",
    "CC1101 Link",
    "BLE Scanner",
    "IR TV-B-Gone"
};

const char* menuIcons[] = {
    "[WIFI]",
    "[NRF!]",         
    "[433M]",
    "[BT-S]",
    "[IR-X]"
};

const int totalItems = 5;
int currentIndex = 0;

void initMenu() {
    drawMenu();
}

void drawMenu() {
    clearCanvas();
    
    #if USE_OLED_DISPLAY
        // Flipper Frame Header Bar
        display.fillRect(0, 0, 128, 13, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(4, 3);
        display.setTextSize(1);
        display.print("LUNAR // MODULES");
        
        display.fillCircle(112, 6, 2, SSD1306_BLACK);
        display.fillCircle(120, 6, 2, SSD1306_BLACK);

        // Main Content Frame Renderer
        int startingItem = (currentIndex >= 3) ? 3 : (currentIndex >= 2 ? 2 : 0);
        
        for (int i = startingItem; i < startingItem + 2; i++) {
            if (i >= totalItems) break;
            
            int yPos = 18 + ((i - startingItem) * 22);
            
            if (i == currentIndex) {
                display.fillRoundRect(2, yPos, 124, 18, 3, SSD1306_WHITE);
                display.setTextColor(SSD1306_BLACK);
            } else {
                display.drawRoundRect(2, yPos, 124, 18, 3, SSD1306_WHITE);
                display.setTextColor(SSD1306_WHITE);
            }
            
            display.setCursor(8, yPos + 5);
            display.print(menuIcons[i]);
            display.setCursor(50, yPos + 5);
            display.print(menuItems[i]);
        }
        
        // Flipper Style Scrollbar Indicator
        display.drawRect(2, 61, 124, 3, SSD1306_WHITE);
        int indicatorWidth = 124 / totalItems;
        display.fillRect(2 + (currentIndex * indicatorWidth), 61, indicatorWidth, 3, SSD1306_WHITE);
    #endif
    
    renderCanvas();
}

void loopMenu() {
    if (digitalRead(BTN_UP) == LOW) {
        currentIndex = (currentIndex - 1 + totalItems) % totalItems;
        drawMenu();
        delay(200);
    }
    if (digitalRead(BTN_DOWN) == LOW) {
        currentIndex = (currentIndex + 1) % totalItems;
        drawMenu();
        delay(200);
    }
    if (digitalRead(BTN_SEL) == LOW) {
        delay(200);
        
        if (currentIndex == 0) currentState = STATE_WIFI;
        if (currentIndex == 1) { 
            currentState = STATE_NRF; 
            extern void drawNrfSubMenu();
            drawNrfSubMenu(); 
        }
        if (currentIndex == 2) currentState = STATE_SUBGHZ;
        if (currentIndex == 3) currentState = STATE_BLE;
        if (currentIndex == 4) currentState = STATE_IR;
        clearCanvas();
    }
}
